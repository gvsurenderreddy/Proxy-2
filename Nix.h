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
#ifndef NIX_H
#define NIX_H

#include <memory>

namespace Debug {
	class Debug;
}
namespace Deque {
	template<typename I, typename T>
	class Deque;
}
namespace Threads {
	class SmartThread;
	class ThreadPool;
}
namespace Layer {
	class SockPool;
}
namespace Check {
	class PConf;
}

namespace Proxy {

class BaseSocket;
class Nix {
private:
	std::shared_ptr<Threads::ThreadPool> tp;
	std::shared_ptr<Threads::SmartThread> ct;
	std::shared_ptr<Threads::SmartThread> it;
	std::shared_ptr<Threads::SmartThread> ot;
	std::shared_ptr<Layer::SockPool> sp;
	std::shared_ptr<BaseSocket> cs;
	std::shared_ptr<BaseSocket> is;
	std::shared_ptr<BaseSocket> os;
	bool cflag;
	bool iflag;
	bool oflag;
	std::shared_ptr<Check::PConf> confh;
	std::shared_ptr<Debug::Debug> logh;
	void Control();
	void Cinner();
	void Couter();
	void Summary() const;
	void Cycle();
	void Clean();
public:
	void Tctrl();
	void Tinit();
	void Tloop();
	Nix();
	virtual ~Nix();
};
} /* namespace Proxy */
#endif
#endif
/* tabstop=8 */
