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

#include <signal.h>

#include "Debug.h"
#include "Singleton.h"
#include "Daemon.h"

#include "ProxyConf.h"
#include "Check.h"
#include "Nix.h"

/*
 * UNIX signal handling
 */
void sig()
{
	/* SIGINT halt */
	signal(SIGINT, [](int par) {
		__(Debug::Debug)->Exc("[Main]: SIGINT caught, plating");
		Check::DConf::SetReset();
	});
	/* SIGTERM halt */
	signal(SIGTERM, [](int par) {
		__(Debug::Debug)->Exc("[Main]: SIGTERM caught, plating");
		Check::DConf::SetReset();
	});
	/* SIGABRT ignored */
	signal(SIGABRT, [](int par) {
		__(Debug::Debug)->Exc("[Main]: SIGABRT caught, ignoring");
	});
#ifndef _WIN32
	/* SIGPIPE ignored */
	signal(SIGPIPE, [](int par) {
		__(Debug::Debug)->Exc("[Main]: SIGPIPE caught, ignoring");
	});
	/* SIGSTOP ignored */
	signal(SIGSTOP, [](int par) {
		__(Debug::Debug)->Exc("[Main]: SIGSTOP caught, ignoring");
	});
	/* SIGCHLD ignored */
	signal(SIGCHLD, SIG_IGN);
#endif
}

/*
 * Main: entry point
 */
int main(int argc, char* argv[])
{
	/* Parse command line arguments and configure as daemon or cmd line */
	__(Debug::Debug)->Log("[Main]: *Nix", _rc(Debug::Debug));
	if (Singleton::_<Check::PConf>(argc, argv)->Daemon() &&
	    !Check::DConf::GetReset())
		__(Daemon::Daemon)->Daemonize();
	else if (!Check::DConf::GetReset())
		sig();
	else
		return -1;

	/* Instantiate platform specific Proxy class, init and run loop */
	Proxy::Nix nix;
	nix.Tinit();
	nix.Tloop();
	__(Debug::Debug)->Log("[Main]: bye bye", _rc(Debug::Debug));
	return 0;
}
/* tabstop=8 */
