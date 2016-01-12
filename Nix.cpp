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

#ifndef _WIN32

#include <memory>

#include "Debug.h"
#include "Singleton.h"
#include "Deque.h"
#include "Threads.h"

#include "ProxyConf.h"
#include "Check.h"
#include "Nix.h"
#include "Dae.h"

namespace Proxy {

/*
 * Inner socket thread:
 * manages I/O to and from the TUN interface
 */
void Nix::Cinner()
{
	logh->Log("[Nix::Cinner]");
	it = std::make_shared<Threads::SmartThread>([](void *_t)->void* {
		auto _x=static_cast<Nix*>(_t);
		_x->is->Run();
		_x->iflag = true;
		return nullptr;
	}, (void*)this);
	iflag = false;

	/* add thread to pool and start its execution */
	Threads::STI sti;
	tp->Add(it, &sti);
}

/*
 * Outer socket thread:
 * manages I/O between client and server (TLS)
 */
void Nix::Couter()
{
	logh->Log("[Nix::Couter]");
	ot = std::make_shared<Threads::SmartThread>([](void *_t)->void* {
		auto _x=static_cast<Nix*>(_t);
		_x->os->Run();
		_x->oflag = true;
		return nullptr;
	}, (void*)this);
	oflag = false;

	/* add thread to pool and start its execution */
	Threads::STI sti;
	tp->Add(ot, &sti);
}

/*
 * Health status:
 * status of deque and reference counter of shared pointers
 */
void Nix::Summary() const
{
	logh->Log("[Nix::Summary]: main thread status");
	logh->Log("[Nix::Summary]:\tthreads", tp->N());
	logh->Log("[Nix::Summary]:\tdebug", _rc(Debug::Debug));
	logh->Log("[Nix::Summary]:\tdeque",
	    Singleton::rc<Deque::Deque<int,std::string>>());
	logh->Log("[Nix::Summary]:\tIConf", _rc(Check::IConf));
	logh->Log("[Nix::Summary]:\tPConf", _rc(Check::PConf));
	logh->Log("[Nix::Summary]:\tDConf", _rc(Check::DConf));
	logh->Log("[Nix::Summary]:\tCConf", _rc(Check::CConf));
}

/*
 * Cycle threads: release threads in halted state
 */
void Nix::Cycle()
{
	logh->Log("[Nix::Cycle]");
	if (!Check::DConf::GetConfig())
		logh->Log("[Nix::Cycle]: threads #", tp->Cycle());
	else
		logh->Log("[Nix::Cycle]: threads ok");
}

/*
 * Clean Up:
 * join all threads in thread pool
 * resume if there has been a dynamic hot change of the configuration params
 */
void Nix::Clean()
{
	logh->Log("[Nix::Clean]");
	if (!Check::DConf::GetReset()) {
		Check::DConf::SetConfig(false);
		logh->Log("[Nix::Clean]: recovering ...");
		Tloop();
	} else {
		logh->Log("[Nix::Clean]: tearing down gently");
		tp->Join();
	}
}

/*
 * Initialize inner and outer threads
 */
void Nix::Tinit()
{
	logh->Log("[Nix::Tinit]: AKT6S46IS46T6KA");
	os->Init();
	is->Init();
}

/*
 * Main thread infinite loop
 */
void Nix::Tloop()
{
	logh->Log("[Nix::Tloop]: preparing threads");
	timeval tp={confh->Guard()*TF/F, U};
	while (!Check::DConf::GetConfig() && !Check::DConf::GetReset()) {
		timeval tv=tp;
		if (iflag)
			Cinner();
		if (oflag)
			Couter();
		select(0, 0, 0, 0, &tv);
		Cycle();
		Summary();
	}
	Clean();
}

Nix::Nix()
	: tp(__(Threads::ThreadPool)),
	  it(nullptr),
	  ot(nullptr),
	  is(std::make_shared<InnerSocket>()),
	  os(std::make_shared<OuterSocket>()),
	  iflag(true),
	  oflag(true),
	  confh(__(Check::PConf)),
	  logh(__(Debug::Debug))
{
	logh->Log("[Nix::Nix]");
}

Nix::~Nix()
{
	logh->Log("[Nix::~Nix]");
	tp->Clean();
}

} /* namespace Proxy */
#endif
/* tabstop=8 */
