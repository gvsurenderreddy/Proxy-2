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

#ifndef IFACE_H
#define IFACE_H

#include <memory>

namespace Debug {
	class Debug;
}
namespace Deque {
	template<typename I, typename T>
	class Deque;
}
namespace Memory {
	class MemoryPool;
}
namespace TunTap {
	class Interface;
}

namespace Proxy {

class Iface {
protected:
	int id;
	std::shared_ptr<TunTap::Interface> iface;
	std::shared_ptr<Deque::Deque<int, char*>> dh;
	std::shared_ptr<Memory::MemoryPool> mp;
	std::shared_ptr<Check::PConf> confh;
	std::shared_ptr<Debug::Debug> logh;
	int MultiAf(char*, bool) const;
	void SetFd(char*, int);
	int GetFd(char*);
	void Read();
	void Write();
public:
	void Run();
	Iface(std::shared_ptr<TunTap::Interface>);
	virtual ~Iface();
};
} /* namespace Proxy */
#endif
/* tabstop=8 */
