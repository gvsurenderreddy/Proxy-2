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

#include <unistd.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>

#include "Debug.h"
#include "Lock.h"
#include "Singleton.h"
#include "Deque.h"
#include "ExecUnit.h"
#include "Decorator.h"
#include "Memory.h"
#include "ThreadsBase.h"
#include "Threads.h"
#include "SockHandler.h"
#include "Headers.h"

#include "ProxyConf.h"
#include "Check.h"
#include "Proto.h"

namespace Proxy {

/*
 * Lenght I/O
 * Lenght of IPv6 datagram (or IPv4 datagram)
 */
int ProtoBase::Lenght(char *_buff) const
{
	int lenght=0;
	if (reinterpret_cast<Craft::ip*>(_buff)->ip_v == 6) {
		auto ip6_h=reinterpret_cast<Craft::ip6*>(_buff);
		lenght = ntohs(ip6_h->ip6_plen)+40;
		logh->Log("[ProtoBase::Lenght]: IPv6 of", lenght);
#ifdef DONOTDISABLEV4
	} else {
		auto ip_h=reinterpret_cast<Craft::ip*>(_buff);
		lenght = ntohs(ip_h->ip_len);
		logh->Log("[ProtoBase::Lenght]: IPv4 of", lenght);
#endif
	}
	return lenght;
}

void ProtoBase::KeepAlive()
{
	/* ! SO_KEEPALIVE experiments*/
	sd->SetOptlevel(SOL_SOCKET);
	sd->SetOptname(SO_KEEPALIVE);
	int opt=1;
	sd->SetOptval(reinterpret_cast<char*>(&opt));
	sd->SetOptlen(sizeof(opt));
	int err = exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Setsockopt(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		return 0;
	});
#ifndef __FreeBSD__
	sd->SetOptlevel(SOL_TCP);
	sd->SetOptname(TCP_KEEPIDLE);
	int opt=10;
	sd->SetOptval(reinterpret_cast<char*>(&opt));
	sd->SetOptlen(sizeof(opt));
	err = exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Setsockopt(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		return 0;
	});
	sd->SetOptlevel(SOL_TCP);
	sd->SetOptname(TCP_KEEPINTVL);
	int opt=5;
	sd->SetOptval(reinterpret_cast<char*>(&opt));
	sd->SetOptlen(sizeof(opt));
	err = exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Setsockopt(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		return 0;
	});
	sd->SetOptlevel(SOL_TCP);
	sd->SetOptname(TCP_KEEPCNT);
	int opt=10;
	sd->SetOptval(reinterpret_cast<char*>(&opt));
	sd->SetOptlen(sizeof(opt));
	err = exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Setsockopt(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		return 0;
	});
#else
	pid_t pid=-1;
	int status=0;
	if ((pid=fork()) < 0)
		logh->Exc("[Interface::Addr6]: error forking");
	else if (pid > 0)
		waitpid(pid, &status, 0);
	else 
		execl("/sbin/sysctl", "/sbin/sysctl", "-w",
		    "net.inet.tcp.keepidle=10000",
		    "net.inet.tcp.keepintvl=5000",
		    "net.inet.tcp.keepcnt=10", nullptr);
	logh->Log("[ProtoBase::KeepAlive]: keep alive with status", status);
#endif
	if (err == -1)
		logh->Exc("[ProtoBase::KeepAlive]: error initializing");
}

/*
 * Recv I/O
 * receiving in the outer socket writes in the reverse deque
 */
void ProtoBase::Recv(std::shared_ptr<Layer::BaseSock> _fd, int *_flag)
{
	char *buff=nullptr;
	int n=mp->Alloc(buff);
	if (!n) {
		logh->Exc("[ProtoBase::Recv]: no memory blocks available");
		return;
	}
	_fd->SetBuff(buff);
	_fd->SetN(n);
	int nread;
	if ((nread=sh->Recv(_fd))>0 && !_fd->GetError()->Get()) {
		logh->Log("[ProtoBase::Recv]:", nread);
		logh->Hex(buff, nread);
		int plen=Lenght(buff);
		if (plen != nread)
			logh->Exc("[ProtoBase::Lenght]:", nread, "!=", plen);
		else
			logh->Log("[ProtoBase::Lenght]: bce xopowo");
		dh->PushReverse(nread, buff);
	} else {
		logh->Log("[ProtoBase::Recv]: EOF", *_flag);
		mp->Dealloc(buff);
		*_flag = 0;
	}
}

/*
 * Send I/O
 * receiving in the direct deque writes in the outer socket
 */
void ProtoBase::Send(std::shared_ptr<Layer::BaseSock> _fd, int *_flag)
{
	logh->Log("[ProtoBase::Send]: send", *_flag);
	auto de=dh->FrontDirect();
	if (de) { 
		_fd->SetBuff(de->second);
		_fd->SetN(Lenght(de->second));
		if (sh->Send(_fd)<=0 || _fd->GetError()->Get()) {
			logh->Exc("[ProtoBase::Send]: EOF", *_flag);
			*_flag = 0;
		}
		mp->Dealloc(de->second);
		dh->PopDirect();
	}
	dh->CheckDirect();
}

/*
 * Set Handler:
 * socket handler is used by means of object composition
 */
void ProtoBase::SetHandler(std::shared_ptr<Layer::SockHandler> _h)
{
	logh->Log("[ProtoBase::SetHandler]: manually setting sh");
	sh = _h;
}

/*
 * Get Handler:
 * returns the socket layer handler (the COR for the TCP/IP stack)
 */
std::shared_ptr<Layer::SockHandler> ProtoBase::GetHandler() const
{
	logh->Log("[ProtoBase::GetHandler]");
	return sh;
}

/*
 * Set Socket:
 * set the socket, that is, the message passed in the COR
 */
void ProtoBase::SetSocket(std::shared_ptr<Layer::BaseSock> _s)
{
	logh->Log("[ProtoBase::SetSocket]: manually setting sd");
	sd = _s;
}

/*
 * Get Socket:
 * return the socket, that is, the message passed in the COR
 */
std::shared_ptr<Layer::BaseSock> ProtoBase::GetSock() const
{
	logh->Log("[ProtoBase::GetSocket]");
	return sd;
}

ProtoBase::ProtoBase(std::shared_ptr<Layer::BaseSock> _s,
    std::shared_ptr<Layer::SockHandler> _h) 
	: dh(Singleton::_<Deque::Deque<int, char*>>()),
	  mp(__(Memory::MemoryPool)),
	  exc(nullptr),
	  confh(__(Check::PConf)),
	  sh(_h),
	  sd(_s),
	  logh(__(Debug::Debug))
{
	logh->Log("[ProtoBase::ProtoBase]");
	auto ee=std::make_shared<ExecUnit::Eunit<int, ProtoBase*>>
	    ([](ProtoBase *_this)->int {
		if (_this->sd->GetError()->Get())
			return 1;
		return 0;
	}, this);
	ee->SetDrc(-1);
	exc = std::make_shared<Decorator::Decorator<int, int, ProtoBase*>>(ee);
}

ProtoBase::~ProtoBase()
{
	logh->Log("[ProtoBase::~ProtoBase]");
}

/*
 * Run-time status of the client
 */
void StreamClient::Summary() const
{
	logh->Log("[StreamClient::Summary]: outer socket status");
	logh->Log("[StreamClient::Summary]:\tdeque reverse size",
	    dh->SizeReverse());
	logh->Log("[StreamClient::Summary]:\tdeque direct size",
	    dh->SizeDirect());
}

/*
 * Client Selector:
 * monitor status of direct deque and socket file descriptors
 */
void StreamClient::Selector()
{
	timeval ts;
	fd_set rds, wds;
	int dd=dh->FdDirect();
	int fd=sd->GetSd(0);
	int tfd=fd;
	logh->Log("[StreamServer::Selector]:", fd);
	while (fd && !Check::DConf::GetReset() && !sd->GetError()->Get()) {
		ts = {confh->Guard()*AF/F, U};
		auto maxfd=std::max(dd, fd)+1;
		FD_ZERO(&rds);
		FD_ZERO(&wds);
		FD_SET(fd, &rds);
		FD_SET(dd, &rds);
		select(maxfd, &rds, 0, 0, &ts);
		if (FD_ISSET(fd, &rds) && !Check::DConf::GetReset() &&
		    !sd->GetError()->Get())
			Recv(sd, &fd);
		if (FD_ISSET(dd, &rds) && !Check::DConf::GetReset() &&
		    !sd->GetError()->Get())
			Send(sd, &fd);
	}
	Check::CConf::Status(false);
	logh->Log("[StreamServer::Selector]: removing", tfd);
	if (!((confh->Key()).empty() || (confh->Cert()).empty()))
		sh->Close(tfd, sd->GetSsl());
	else
		sh->Close(tfd);
}

/*
 * Client socket initialization
 */
void StreamClient::Init()
{
	logh->Log("[StreamClient::Init]");
	if (!sd || !sh) {
		logh->Exc("[StreamClient::Init]: !sd || !sh");
		Check::DConf::SetReset();
		return;
	}
	int err=exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Socket(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		return 0;
	});
	KeepAlive();
	err = exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Connect(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		Check::CConf::Status(true);
		return 0;
	});
	if (err == -1)
		logh->Exc("[StreamClient::Init]: error initializing");
}

/*
 * Connect client to server and run selector
 */
void StreamClient::Run()
{
	logh->Log("[StreamClient::Run]");
	if (!Check::DConf::GetReset() && !sd->GetError()->Get())
		Selector();
	else if (!Check::DConf::GetReset())
		Check::DConf::SetReset();
}

StreamClient::StreamClient(std::shared_ptr<Layer::BaseSock> _s,
    std::shared_ptr<Layer::SockHandler> _h) 
	: ProtoBase(_s, _h)
{
	logh->Log("[StreamClient::StreamClient]");
}

StreamClient::~StreamClient()
{
	logh->Log("[StreamClient::~StreamClient]");
}

/*
 * Set/Get connected socket descriptor of a specific thread (identified by tid)
 */
int& StreamServer::Csd::operator[](uint64_t _k)
{
	Lock::Lock l(&csl);
	return _csd[_k];
}

/*
 * Remove entry from map
 */
void StreamServer::Csd::Erase(uint64_t _k)
{
	Lock::Lock l(&csl);
	_csd.erase(_k);
}

StreamServer::Csd::Csd()
	: _csd(std::map<uint64_t, int>())
{
#ifdef _WIN32
	InitializeCriticalSection(&csl);
#else
	csl = PTHREAD_MUTEX_INITIALIZER;
#endif
}

/*
 * Run-time status of the server
 */
void StreamServer::Summary() const
{
	logh->Log("[StreamServer::Summary]: outer socket status");
	logh->Log("[StreamServer::Summary]:\tsockpool", sp->N());
	logh->Log("[StreamServer::Summary]:\tdeque reverse size",
	    dh->SizeReverse());
	logh->Log("[StreamServer::Summary]:\tdeque direct size",
	    dh->SizeDirect());
}

/*
 * Handle incoming connection:
 * handle new accepted connections from clients in their own thread
 */
void StreamServer::HandleConn()
{
	logh->Log("[StreamServer::HandleConn]");
	auto nsd=std::make_shared<Layer::BaseSock>(*sd);
	sp->Set(sd->GetSd(1), nsd);
	auto st=std::make_shared<Threads::SmartThread>([](void *_t)->void* {
		auto _x=static_cast<StreamServer*>(_t);
		__(Debug::Debug)->Log("[StreamServer::HandleConn]: spawning");
		_x->Selector();
		return nullptr;
	}, (void*)this);
	logh->Log("[StreamServer::HandleConn]: setting connected socket for",
	    st->Uid());
	csd[st->Uid()] = sd->GetSd(1);
	Threads::STI sti;
	tp->Add(st, &sti);

	/* current version only supports one connection */
	sh->Close(sd->GetSd(0));
	st->Join();
	Init();
}

/*
 * Server Selector:
 * monitor status of direct deque and socket file descriptors
 * as server is multi-thread, each thread is responsible of its own socket (csd)
 */
void StreamServer::Selector()
{
	auto tt=tp->Get(Threads::ThreadBase::Pself());
	logh->Log("[StreamServer::Selector]: getting for", tt->Uid());
	int fd=csd[tt->Uid()];
	int dd=dh->FdDirect();
	int tfd=fd;
	auto tsd=sp->Get(fd);
	logh->Log("[StreamServer::Selector]:", fd);
	timeval ts;
	fd_set rds, wds;
	while (fd && !Check::DConf::GetReset() && !tsd->GetError()->Get()) {
		ts = {confh->Guard()*AF/F, U};
		auto maxfd=std::max(dd, fd)+1;
		FD_ZERO(&rds);
		FD_ZERO(&wds);
		FD_SET(fd, &rds);
		FD_SET(dd, &rds);
		select(maxfd, &rds, 0, 0, &ts);
		if (FD_ISSET(fd, &rds) && !Check::DConf::GetReset() &&
		    !tsd->GetError()->Get())
			Recv(tsd, &fd);
		if (FD_ISSET(dd, &rds) && !Check::DConf::GetReset() &&
		    !sd->GetError()->Get())
			Send(tsd, &fd);
	}
	Check::CConf::Status(false);
	logh->Log("[StreamServer::Selector]: closing connection", tfd);
	csd.Erase(tt->Uid());
	sp->Del(tfd);
	if (!((confh->Key()).empty() || (confh->Cert()).empty()))
		sh->Close(tfd, tsd->GetSsl());
	else
		sh->Close(tfd);
}

/*
 * Server socket initialization
 * mark socket as passive
 */
void StreamServer::Init()
{
	logh->Log("[StreamServer::Init]");
	if (!sd || !sh) {
		logh->Exc("[StreamServer::Init]: !sd || !sh");
		Check::DConf::SetReset();
		return;
	}
	int err=exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Socket(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		return 0;
	});
	err = exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Setsockopt(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		return 0;
	});
	KeepAlive();
	err = exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Bind(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		return 0;
	});
	err = exc->run([this]()->int {
		if (Check::DConf::GetReset())
			return -1;
		this->sh->Listen(this->sd);
		if (this->sd->GetError()->Get()) {
			Check::DConf::SetReset();
			return -1;
		}
		return 0;
	});
	if (err == -1) {
		logh->Exc("[StreamServer::Init]: error initializing");
		return;
	}
	sd->SetPool(sp);
}

/*
 * Accept connections from clients and handle them
 */
void StreamServer::Run()
{
	logh->Log("[StreamServer::Run]");
	sp->Set(sd->GetSd(0), sd);
	while (!Check::DConf::GetReset() && !sd->GetError()->Get()) {
		timeval ts={confh->Guard()*AF/F, U};
		fd_set rds;
		auto maxfd=sd->GetSd(0)+1;
		FD_ZERO(&rds);
		FD_SET(sd->GetSd(0), &rds);
		select(maxfd, &rds, 0, 0, &ts);
		if (!Check::DConf::GetReset() &&
		    FD_ISSET(sd->GetSd(0), &rds)) {
			int err=exc->run([this]()->int {
				if (Check::DConf::GetReset())
					return -1;
				this->sh->Accept(this->sd);
				if (this->sd->GetError()->Get() ||
				    (this->sd->GetSd(1) <= 0))
					return -1;
				Check::CConf::Status(true);
				return 0;
			});
			if (!err)
				HandleConn();
			else
				logh->Exc("[StreamServer::Run]:",
				    "error accepting new connection");
		}
		Summary();
	}
	if (!Check::DConf::GetReset())
		Check::DConf::SetReset();
}

StreamServer::StreamServer(std::shared_ptr<Layer::BaseSock> _s,
    std::shared_ptr<Layer::SockHandler> _h) 
	: ProtoBase(_s, _h),
	  csd(),
	  sp(__(Layer::SockPool)),
	  tp(__(Threads::ThreadPool))
{
	logh->Log("[StreamServer::StreamServer]");
}

StreamServer::~StreamServer()
{
	logh->Log("[StreamServer::~StreamServer]");
	sp->Clear();
}
} /* namespace Proxy */
/* tabstop=8 */
