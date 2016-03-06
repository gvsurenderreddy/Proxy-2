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

#include <memory>
#include <string>
#include <algorithm>
#ifndef __FreeBSD__
#include <linux/if_tun.h>
#endif

#include "Debug.h"
#include "Singleton.h"
#include "Deque.h"
#include "TunTap.h"
#include "CraftRaw.h"
#include "SockHandler.h"
#include "SockNetwork.h"
#include "SockTransport.h"
#include "SockPresentation.h"

#include "ProxyConf.h"
#include "Check.h"
#include "Dae.h"
#include "Iface.h"
#include "Proto.h"

namespace Proxy {

BaseSocket::BaseSocket()
	: confh(__(Check::PConf)),
	  logh(__(Debug::Debug))
{
	logh->Log("[BaseSocket::BaseSocket]");
}

BaseSocket::~BaseSocket()
{
	logh->Log("[BaseSocket::~BaseSocket]");
}

/*
 * Inner socket initialization:
 * create TUN devices and add IPv6 (IPv4) address and routes (if auto-conf)
 */
void InnerSocket::Init()
{
	logh->Log("[InnerSocket::Init]");
	if (iface->Alloc() < 0) {
		Check::DConf::SetReset();
		return;
	}
	iface->Up();
	app = std::make_shared<Iface>(iface);
	if (confh->Client()) {
		iface->Addr6(confh->Ac6());
#ifdef DONOTDISABLEV4
		iface->Addr4(confh->Ac4());
#endif
	} else {
		iface->Addr6(std::string(TSADDR6));
#ifdef DONOTDISABLEV4
		iface->Addr4(std::string(TSADDR4));
#endif
	}
	if (confh->Client() && confh->Iauto())
	{
		iface->SSRoute6();
		iface->SDRoute6();
#ifdef DONOTDISABLEV4
		iface->SSRoute4();
		iface->SDRoute4();
#endif
	}
	if (Check::DConf::GetReset() && confh->Client() && confh->Iauto()) {
		iface->RRoute6();
#ifdef DONOTDISABLEV4
		iface->RRoute4();
#endif
	}
}

/*
 * Inner socket infinite loop
 * I/O through TUN device
 */
void InnerSocket::Run()
{
	logh->Log("[InnerSocket::Run]");
	app->Run();
	if (confh->Client() && confh->Iauto()) {
		iface->RRoute6();
#ifdef DONOTDISABLEV4
		iface->RRoute4();
#endif
	}
}

/*
 * Inner socket cleanup
 */
void InnerSocket::End()
{
	logh->Log("[InnerSocket::End]: doing nothing");
}

InnerSocket::InnerSocket()
	: BaseSocket(),
#ifdef __FreeBSD__
	  iface(std::make_shared<TunTap::Interface>(0,
	      confh->Tun(), confh->Addr())),
#else
	  iface(std::make_shared<TunTap::Interface>(IFF_TUN|IFF_NO_PI,
	      confh->Tun(), confh->Addr())),
#endif
	  app(nullptr)
{
	logh->Log("[InnerSocket::InnerSocket]");
}

InnerSocket::~InnerSocket()
{
	logh->Log("[InnerSocket::~InnerSocket]");
	iface->Close();
}

/*
 * Outer socket client initialization:
 * configure socket descriptor (message in COR)
 */
void OuterSocket::InitClient(std::string _addr, uint16_t _port)
{
	logh->Log("[OuterSocket::InitClient]");
	sd->SetMode(ModeClient);
	sd->SetMutual(true);
	sd->SetPort(Active, _port);
	char tbuff[MIN_BUFF]={0};
	auto tstr=_addr;
	auto tlen=std::min((uint32_t)tstr.size(), (uint32_t)MIN_BUFF-1);
	sd->SetBuff(tbuff);
	std::copy(tstr.begin(), tstr.begin()+tlen, tbuff);
	tls->Getaddrinfo(sd);
}

/*
 * Outer socket server initialization:
 * configure socket descriptor (message in COR)
 */
void OuterSocket::InitServer(std::string _addr, uint16_t _port)
{
	logh->Log("[OuterSocket::InitServer]");
	sd->SetMode(ModeServer);
	sd->SetMutual(true);
	sd->SetAddr(Passive, _addr);
	sd->SetPort(Passive, _port);
        sd->SetOptlevel(SOL_SOCKET);
        sd->SetOptname(SO_REUSEADDR);
	int opt=1;
        sd->SetOptval(reinterpret_cast<char*>(&opt));
        sd->SetOptlen(sizeof(opt));
}

/*
 * Initialize SSL
 */
void OuterSocket::InitSsl()
{
	logh->Log("[OuterSocket::InitSsl]: key", confh->Key());
	logh->Log("[OuterSocket::InitSsl]: X509", confh->Cert());
	logh->Log("[OuterSocket::InitSsl]: CAX", confh->Cax());
	sd->SetKeypath(confh->Key());
	sd->SetCertpath(confh->Cert());
	sd->SetCacertpath(confh->Cax());
}

/*
 * Outer socket initialization:
 * configure the Chain of Responsibility (TCP/IP stack) and its message (sd)
 * IPv6 server on dual-stack host can service both IPv6 and IPv4
 * IPv6 client on dual-stack host can communicate with IPv6 and IPv4 servers
 * ... hence -> IPv6
 */
void OuterSocket::Init()
{
	logh->Log("[OuterSocket::Init]");
	if (confh->Client())
		app = std::make_shared<StreamClient>();
	else
		app = std::make_shared<StreamServer>();
	tls = std::make_shared<Layer::SockTls>();
	tcp = std::make_shared<Layer::SockTcp>();
	ip = std::make_shared<Layer::SockIp<Layer::IA6,Layer::SAI6>>(IPv6);
	sd = std::make_shared<Layer::ActiveSock<Layer::IA6,Layer::SAI6>>(IPv6);
	tls->SetSucc(tcp);
	tcp->SetSucc(ip);
	if (confh->Client())
		InitClient(confh->Addr(), confh->Port());
	else
		InitServer(confh->Addr(), confh->Port());
	InitSsl();
	app->SetHandler(tls);
	app->SetSocket(sd);
	app->Init();
}

/*
 * Outer socket infinite loop
 * TLS I/O
 */
void OuterSocket::Run()
{
	logh->Log("[OuterSocket::Run]");
	app->Run();
}

/*
 * Outersocket cleanup
 */
void OuterSocket::End()
{
	logh->Log("[OuterSocket::End]");
	app->End();
}

OuterSocket::OuterSocket()
	: BaseSocket(),
	  sd(nullptr),
	  app(nullptr),
	  tls(nullptr),
	  tcp(nullptr),
	  ip(nullptr)
{
	logh->Log("[OuterSocket::OuterSocket]");
}

OuterSocket::~OuterSocket()
{
	logh->Log("[OuterSocket::~OuterSocket]");
}

/*
 * Control socket init:
 * request 1
 */
void ControlSocket::Init()
{
	logh->Log("[ControlSocket::Init]");
	app = std::make_shared<Control>();
	tls = std::make_shared<Layer::SockTls>();
	tcp = std::make_shared<Layer::SockTcp>();
	ip = std::make_shared<Layer::SockIp<Layer::IA6,Layer::SAI6>>(IPv6);
	sd = std::make_shared<Layer::ActiveSock<Layer::IA6,Layer::SAI6>>(IPv6);
	tls->SetSucc(tcp);
	tcp->SetSucc(ip);
	InitClient(confh->Addr(), PORT_C);
	InitSsl();
	app->SetHandler(tls);
	app->SetSocket(sd);
	app->Init();
}

/*
 * Control socket run:
 * Control loop logic, server 1
 */
void ControlSocket::Run()
{
	logh->Log("[ControlSocket::Run]");
	app = std::make_shared<Control>();
	tls = std::make_shared<Layer::SockTls>();
	tcp = std::make_shared<Layer::SockTcp>();
	ip = std::make_shared<Layer::SockIp<Layer::IA6,Layer::SAI6>>(IPv6);
	sd = std::make_shared<Layer::ActiveSock<Layer::IA6,Layer::SAI6>>(IPv6);
	tls->SetSucc(tcp);
	tcp->SetSucc(ip);
	InitServer(confh->Addr(), PORT_C);
	InitSsl();
	app->SetHandler(tls);
	app->SetSocket(sd);
	app->Run();
}

ControlSocket::ControlSocket()
	: OuterSocket()
{
	logh->Log("[ControlSocket::ControlSocket]");
}

ControlSocket::~ControlSocket()
{
	logh->Log("[ControlSocket::~ControlSocket]");
}
} /* namespace Proxy */
/* tabstop=8 */
