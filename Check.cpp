/*-
 * Copyright 2015 - Polite Ping Software Foundation - All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the Polite Ping Software
 *     Foundation and its contributors.
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

#ifndef __WIN32
#include <unistd.h>
#include <cstdlib>
#endif
#include <vector>
#include <sstream>
#include <iostream>
#include <iterator>

#include "Debug.h"
#include "Singleton.h"

#include "ProxyConf.h"
#include "Check.h"

namespace Check {

bool DConf::reset=false;
bool DConf::config=false;
bool CConf::status=false;
#ifdef __WIN32
CRITICAL_SECTION g_csl=*Lock::SingleMutex().get(); /*S g_csl!=csl!*/
#else
pthread_mutex_t g_csl=PTHREAD_MUTEX_INITIALIZER; /*S g_csl!=csl!*/
#endif

void IConf::Config(const int _k, const std::string _v)
{
	std::ostringstream ks;
	ks << _k;
	Config(ks.str(), _v);
}

void *IConf::Config(const int _k)
{
	std::ostringstream ks;
	ks << _k;
	return Config(ks.str());
}

IConf::IConf()
	: logh(__(Debug::Debug))
{
#ifdef __WIN32
	InitializeCriticalSection(&csl);
#else
	csl = PTHREAD_MUTEX_INITIALIZER;
#endif
	logh->Log("[IConf::IConf]");
}

IConf::~IConf()
{
#ifdef __WIN32
	DeleteCriticalSection(&csl);
#endif
	logh->Log("[IConf::~IConf]");
}

void DConf::Config(const std::string _k, const std::string _v)
{
	Lock::Lock l(&csl);
	switch (atoi(_k.c_str())) {
	case DAEMON:
		logh->Log("[DConf::Config]: daemonize:", _v);
		std::istringstream(_v) >> std::boolalpha >> daemonize;
		break;
	case SAFE:
		logh->Log("[DConf::Config]: SafeGuard:", _v);
		guard = atoi(_v.c_str());
		break;
	case MAX:
		logh->Log("[DConf::Config]: MaxBuffer:", _v);
		max = atoi(_v.c_str());
		break;
	case IAUTO:
		logh->Log("[DConf::Config]: Iface auto-conf:", _v);
		std::istringstream(_v) >> std::boolalpha >> iauto;
		break;
	};
}

void *DConf::Config(const std::string _k)
{
	Lock::Lock l(&csl);
	switch (atoi(_k.c_str())) {
	case DAEMON:
		return &daemonize;
	case SAFE:
		return &guard;
	case MAX:
		return &max;
	case IAUTO:
		return &iauto;
	default:
		return nullptr;
	};
}

void DConf::SetConfig(const bool _c)
{
	Lock::Lock l(&g_csl);
	config = _c;
}

bool DConf::GetConfig()
{
	Lock::Lock l(&g_csl);
	return config;
}

void DConf::SetReset()
{
	Lock::Lock l(&g_csl);
	reset = true;
}

bool DConf::GetReset()
{
	Lock::Lock l(&g_csl);
	return reset;
}

DConf::DConf()
	: IConf()
{
#ifdef DAEMONIZE
	daemonize = true;
#else
	daemonize = false;
#endif
#ifdef __WIN32
	guard = 1;
#else
	guard = 1000;
#endif
#ifdef MAX_BUFF
	max = MAX_BUFF;
#else
	max = 131072;
#endif
#ifdef IAUTO
	iauto = true;
#else
	iauto = false;
#endif
	logh->Log("[DConf::DConf]");
}

DConf::~DConf()
{
	logh->Log("[DConf::~DConf]");
}

void CConf::Config(const std::string _k, const std::string _v)
{
	Lock::Lock l(&csl);
	switch (atoi(_k.c_str())) {
	case CLIENT:
		logh->Log("[CConf::Config]: client", _v);
		std::istringstream(_v) >> std::boolalpha >> client;
		break;
	case ADDR:
		logh->Log("[CConf::Config]: addr", _v);
		addr = _v;
		break;
	case PORT:
		logh->Log("[CConf::Config]: port", _v);
		port = atoi(_v.c_str());
		break;
	case KEY:
		logh->Log("[CConf::Config]: key path", _v);
		key = cwd + _v;
		break;
	case X509:
		logh->Log("[CConf::Config]: certificate path", _v);
		cert = cwd + _v;
		break;
	case CAX:
		logh->Log("[CConf::Config]: cacertificate path", _v);
		cax = cwd + _v;
		break;
	case TUNI:
		logh->Log("[CConf::Config]: tun interface", _v);
		tun = _v;
	};
}

void *CConf::Config(const std::string _k)
{
	Lock::Lock l(&csl);
	switch (atoi(_k.c_str())) {
	case CLIENT:
		return &client;
	case ADDR:
		return &addr;
	case PORT:
		return &port;
	case KEY:
		return &key;
	case X509:
		return &cert;
	case CAX:
		return &cax;
	case TUNI:
		return &tun;
	default:
		return nullptr;
	};
}

void CConf::Status(const bool _s)
{
	Lock::Lock l(&g_csl);
	status = _s;
}

bool CConf::Status()
{
	Lock::Lock l(&g_csl);
	return status;
}

CConf::CConf()
	: IConf()
{
	char buff[256]={0};
	if (getcwd(buff, 256))
		cwd = buff;
	else
		DConf::SetReset();
#ifdef CLIENT_L
	client = CLIENT;
#else
	client = 0;
#endif
#ifdef SOCKET_L
	addr = SOCKET_L;
#else
	addr = "::";
#endif
#ifdef SERV_PORT_L
	port = PORT_L;
#else
	port = 12345;
#endif
#ifdef NESTKEY
	key = cwd + NESTKEY;
#else
	key = "";
#endif
#ifdef NESTX
	cert = cwd + NESTX;
#else
	cert = "";
#endif
#ifdef NESTCAX
	cax = cwd + NESTX;
#else
	cax = "";
#endif
#ifdef TUN
	tun = TUN;
#else
	tun = "tun3";
#endif
	logh->Log("[CConf::CConf]");
}

CConf::~CConf()
{
	logh->Log("[CConf::~CConf]");
}

void PConf::CheckFile(std::string &_f)
{
	FILE *f=fopen(_f.c_str(), "r");
	if (!f) {
		logh->Exc("[PConf::CheckFile]:", _f, "doesn't exist");
		DConf::SetReset();
		_f = "";
	} else
		logh->Log("[PConf::CheckFile]:", _f, "exists");
}

void PConf::CheckConfig()
{
	auto addr=*static_cast<std::string*>(cconf->Config(ADDR));
	logh->Log("[PConf::CheckConfig]: addr");
	if (Client() && addr.empty())
		logh->Exc("[PConf::CheckConfif]: no address, error");
	else if (!Client() && addr.empty()) {
		logh->Exc("[PConf::CheckConfig]: no address, guessing ...");
		addr = "::";
	}
	auto port=*static_cast<uint16_t*>(cconf->Config(PORT));
	if (Client() && !port)
		logh->Exc("[PConf::CheckConfif]: no port, error");
	else if (!Client() && !port) {
		logh->Exc("[PConf::CheckConfig]: no address, guessing ...");
		port = 11235;
	}
	logh->Log("[PConf::CheckConfig]: key");
	CheckFile(*static_cast<std::string*>(cconf->Config(KEY)));
	logh->Log("[PConf::CheckConfig]: x509");
	CheckFile(*static_cast<std::string*>(cconf->Config(X509)));
}

void PConf::Summary() const
{
	logh->Log("[PConf::Summary]");
	if (Client())
		logh->Log("[PConf::Summary]: client mode");
	else
		logh->Log("[PConf::Summary]: server mode");
	if (Daemon()) 
		logh->Log("[PConf::Summary]: daemon");
	else
		logh->Log("[PConf::Summary]: console application");
	if (!Key().empty())
		logh->Log("[PConf::Summary]: key", Key());
	if (!Cert().empty())
		logh->Log("[PConf::Summary]: X509", Cert());
	if (!Cax().empty())
		logh->Log("[PConf::Summary]: CAX", Cax());
	if (!Addr().empty())
		logh->Log("[PConf::Summary]: address", Addr());
	else
		logh->Exc("[PConf::Summary]: address not provided, error");
	if (Port())
		logh->Log("[PConf::Summary]: port", Port());
	else
		logh->Exc("[PConf::Summary]: port not provided, error");
	if (!Tun().empty())
		logh->Log("[PConf::Summary]: interface", Tun());
	else
		logh->Exc("[PConf::Summary]: interface not provided, error");
}

void PConf::Help() const
{
	logh->Log("[PConf::Help]");
	std::cout << "Options:" << std::endl;
	std::cout << "\t [-a] set address (client mode)" << std::endl;
	std::cout << "\t [-d] set daemon mode" << std::endl;
	std::cout << "\t [-i] set interface auto-conf mode" << std::endl;
	std::cout << "\t [-p] set port" << std::endl;
	std::cout << "\t [-t] set interface" << std::endl;
	std::cout << std::endl;
}

std::shared_ptr<IConf> PConf::D() const
{
	return dconf;
}

std::shared_ptr<IConf> PConf::C() const
{
	return cconf;
}

const bool PConf::Client() const
{
	return *(static_cast<const bool*>(cconf->Config(CLIENT)));
}

const bool PConf::Daemon() const
{
	return *(static_cast<const bool*>(dconf->Config(DAEMON)));
}

const uint16_t PConf::Guard() const
{
	return *(static_cast<const uint16_t*>(dconf->Config(SAFE)));
}

const uint32_t PConf::Max() const
{
	return *(static_cast<const uint16_t*>(dconf->Config(MAX)));
}

const bool PConf::Iauto() const
{
	return *(static_cast<const bool*>(dconf->Config(IAUTO)));
}

const std::string PConf::Addr() const
{
	return *(static_cast<const std::string*>(cconf->Config(ADDR)));
}

const uint16_t PConf::Port() const
{
	return *(static_cast<const uint16_t*>(cconf->Config(PORT)));
}

const std::string PConf::Key() const
{
	return *(static_cast<const std::string*>(cconf->Config(KEY)));
}

const std::string PConf::Cert() const
{
	return *(static_cast<const std::string*>(cconf->Config(X509)));
}

const std::string PConf::Cax() const
{
	return *(static_cast<const std::string*>(cconf->Config(CAX)));
}

const std::string PConf::Tun() const
{
	return *(static_cast<const std::string*>(cconf->Config(TUNI)));
}

void PConf::Static(const std::string _c) const
{
	std::ifstream from(_c);
	if (!from.is_open()) {
		logh->Log("[PConf::Static]: no file");
		return;
	}
	logh->Log("[PConf::Static]:", _c);
	std::string line;
	while (from) {
		getline(from, line);
		Dynamic(line);
	}
	from.close();
}

void PConf::Dynamic(const std::string _l) const
{
	Lock::Lock l(&csl);
	if (_l.empty() || (_l.find_first_not_of(' ')==std::string::npos))
		return;
	logh->Log("[PConf::Dynamic]");
	std::vector<std::string> params(NUMPARAMS+1);
	std::istringstream iss(_l);
	copy(std::istream_iterator<std::string>(iss),
	    std::istream_iterator<std::string>(), params.begin());
	try {
		if (!params.empty()) {
			std::string tmp(params.at(0));
			switch (atoi(tmp.substr(1, tmp.length()-2).c_str())) {
			case PTYPE:
				dconf->Config(params.at(1), params.at(2));
				break;
			case PPROTO:
				cconf->Config(params.at(1), params.at(2));
				break;
			}
		}
	} catch (std::out_of_range) {
		logh->Exc("[PConf::Dynamic]: out of range, skipping parameter");
	}
	params.clear();
}

PConf::PConf(int argc, char** argv)
	: dconf(std::make_shared<DConf>()),
	  cconf(std::make_shared<CConf>()),
	  logh(__(Debug::Debug))
{
#ifdef __WIN32
	InitializeCriticalSection(&csl);
#else
	csl = PTHREAD_MUTEX_INITIALIZER;
#endif
	Static(PROXYCONF);
	while (--argc>0 && (*++argv)[0]=='-')
		while (auto c=*++argv[0])
			switch (c) {
			case 'a':
				cconf->Config(CLIENT, "true");
				if (--argc)
					if (argv[1][0] != '-') {
						cconf->Config(ADDR,
						    std::string(*++argv));
						argv[0][1] = 0;
						break;
					}
				Help();
				exit(0);
			case 'd':
				dconf->Config(DAEMON, "true");
				break;
			case 'i':
				dconf->Config(IAUTO, "true");
				break;
			case 'p':
				if (--argc)
					if (argv[1][0] != '-') {
						cconf->Config(PORT,
						    std::string(*++argv));
						argv[0][1] = 0;
						break;
					}
				Help();
				exit(0);
			case 't':
				if (--argc)
					if (argv[1][0] != '-') {
						cconf->Config(TUNI,
						    std::string(*++argv));
						argv[0][1] = 0;
						break;
					}
				Help();
				exit(0);
			case 'h':
			default:
				Help();
				exit(0);
			}
	logh->Log("[PConf::PConf]");
	CheckConfig();
	Summary();
};

PConf::~PConf()
{
	logh->Log("[PConf::~PConf]");
#ifdef __WIN32
	DeleteCriticalSection(&csl);
#endif
}
} /* namespace Check */
/* tabstop=8 */
