/*-
 * Copyright 2015 - Datagram Garden OÜ - All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *	must display the following acknowledgement:
 *	This product includes software developed by the Polite Ping Software
 *	 Foundation and its contributors.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <algorithm>
#ifdef __FreeBSD__
#include <sys/socket.h>
#endif

#include "Debug.h"
#include "Lock.h"
#include "Singleton.h"
#include "Deque.h"
#include "Memory.h"
#include "TunTap.h"
#include "Headers.h"

#include "ProxyConf.h"
#include "Check.h"
#include "Iface.h"

namespace Proxy {

/*
 * Multi Address Family
 * Acommodate to FreeBSD tun device weirdness of 4bytes multi AF
 */
int Iface::MultiAf(char *_buff, bool _read) const
{
	int lenght=0;
	if (_read) {
#ifdef __FreeBSD__
		if (reinterpret_cast<Craft::ip*>(_buff+4)->ip_v == 6) {
			auto ip6_h=reinterpret_cast<Craft::ip6*>(_buff+4);
#else
		if (reinterpret_cast<Craft::ip*>(_buff)->ip_v == 6) {
			auto ip6_h=reinterpret_cast<Craft::ip6*>(_buff);
#endif
			lenght = ntohs(ip6_h->ip6_plen)+40;
			logh->Log("[Iface::MultiAf]: moving", lenght);
#ifdef DONOTDISABLEV4
		} else {
#ifdef __FreeBSD__
			auto ip_h=reinterpret_cast<Craft::ip*>(_buff+4);
#else
			auto ip_h=reinterpret_cast<Craft::ip*>(_buff);
#endif
			lenght = ntohs(ip_h->ip_len);
			logh->Log("[Iface::MultiAf]: moving", lenght);
#endif
		}
#ifdef __FreeBSD__
		memmove(_buff, _buff+4, lenght);
#endif
	} else {
		if (reinterpret_cast<Craft::ip*>(_buff)->ip_v == 6) {
			auto ip6_h=reinterpret_cast<Craft::ip6*>(_buff);
			lenght = ntohs(ip6_h->ip6_plen)+40;
			logh->Log("[Iface::MultiAf]: moving", lenght);
#ifdef __FreeBSD__
			memmove(_buff+4, _buff, lenght);
			uint32_t caf6=htonl(AF_INET6);
			memcpy(_buff, &caf6, sizeof(uint32_t));
			lenght += 4;
#endif
#ifdef DONOTDISABLEV4
		} else {
			auto ip_h=reinterpret_cast<Craft::ip*>(_buff);
			lenght = ntohs(ip_h->ip_len);
			logh->Log("[Iface::MultiAf]: moving", lenght);
#ifdef __FreeBSD__
			memmove(_buff+4, _buff, lenght);
			uint32_t caf4=htonl(AF_INET);
			memcpy(_buff, &caf4, sizeof(uint32_t));
			lenght += 4;
#endif
#endif
		}
	}
	return lenght;
}

/*
 * Read I/O
 * receiving in the TUN interface writes in the direct deque (if connected)
 */
void Iface::Read()
{
	char *buff=nullptr;
	int n=mp->Alloc(buff);
	if (!n || !buff) {
		logh->Exc("[Iface::Read]: no memory blocks available");
		return;
	}
	int nread;
	if ((nread=read(id, buff, n)) > 0) {
		logh->Log("[Iface::Read]:", nread);
		logh->Hex(buff, nread);
		int plen=MultiAf(buff, true);
#ifdef __FreeBSD__
		if (plen != nread-4)
#else
		if (plen != nread)
#endif
			logh->Exc("[Iface::Read]:", nread, "!=", plen);
		else
			logh->Log("[Iface::Read]: bce xopowo");
		if (Check::CConf::Status())
			dh->PushDirect(nread, buff);
		else
			mp->Dealloc(buff);
	} else
		mp->Dealloc(buff);
}

/*
 * Write I/O
 * receiving the reverse deque writes in the TUN interface
 */
void Iface::Write()
{
	logh->Log("[Iface::Write]");
	auto re=dh->FrontReverse();
	if (re) { 
		int nwrite=MultiAf(re->second, false);
		if (write(id, re->second, nwrite) != nwrite)
			logh->Exc("[Iface::Write]: error");
		else
			logh->Log("[Iface::Write]: bce xopowo");
		mp->Dealloc(re->second);
		dh->PopReverse();
	}
	dh->CheckReverse();
}

/*
 * Interface infinite loop
 * monitor tun interface and reverse deque by means of selec syscall
 */
void Iface::Run()
{
	logh->Log("[Iface::Run]");
	timeval ts;
	fd_set rds, wds;
	int rd=dh->FdReverse();
	while (!Check::DConf::GetReset()) {
		ts = {confh->Guard()*AF/F, U};
		auto maxfd=std::max(id, rd)+1;
		FD_ZERO(&rds);
		FD_ZERO(&wds);
		FD_SET(id, &rds);
		FD_SET(rd, &rds);
		select(maxfd, &rds, 0, 0, &ts);
		if (FD_ISSET(id, &rds) && !Check::DConf::GetReset())
			Read(); 
		if (FD_ISSET(rd, &rds) && !Check::DConf::GetReset())
			Write();
	}
}

Iface::Iface(std::shared_ptr<TunTap::Interface> _i)
	: iface(_i),
	  dh(Singleton::_<Deque::Deque<int, char*>>()),
	  mp(__(Memory::MemoryPool)),
	  confh(__(Check::PConf)),
	  logh(__(Debug::Debug))
{
	logh->Log("[Iface::Iface]");
	id = iface->Fd();
}

Iface::~Iface()
{
	logh->Log("[Iface::~Iface]");
}

}
