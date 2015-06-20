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

#ifndef PROTO_H
#define PROTO_H

#ifdef _WIN32
#define _WINSOCKAPI_
#include <windows.h>
#else 
#include <pthread.h>
#include <pthread.h>
#endif 

#include <memory>
#include <map>

namespace Debug {
	class Debug;
}
namespace Deque {
	template<typename I, typename T>
	class Deque;
}
namespace Decorator {
	template<typename RD, typename RE, typename... Ts>
	class Decorator;
}
namespace Memory {
	class MemoryPool;
}
namespace Threads {
	class ThreadPool;
}
namespace Layer {
	class SockHandler;
	class BaseSock;
	class SockPool;
}

namespace Proxy {


class ProtoBase {
protected:
	std::shared_ptr<Deque::Deque<int, char*>> dh;
	std::shared_ptr<Memory::MemoryPool> mp;
	std::shared_ptr<Decorator::Decorator<int, int, ProtoBase*>> exc;
	std::shared_ptr<Check::PConf> confh;
	std::shared_ptr<Layer::SockHandler> sh;
	std::shared_ptr<Layer::BaseSock> sd;
	std::shared_ptr<Debug::Debug> logh;
	virtual int Lenght(char*) const;
	virtual void Recv(std::shared_ptr<Layer::BaseSock>, int*);
	virtual void Send(std::shared_ptr<Layer::BaseSock>, int*);
public:
	void SetHandler(std::shared_ptr<Layer::SockHandler>);
	std::shared_ptr<Layer::SockHandler> GetHandler() const;
	void SetSocket(std::shared_ptr<Layer::BaseSock>);
	std::shared_ptr<Layer::BaseSock> GetSock() const;
	virtual void Init()=0;
	virtual void Run()=0;
	ProtoBase(std::shared_ptr<Layer::BaseSock> _s=nullptr,
	    std::shared_ptr<Layer::SockHandler> _h=nullptr);
	virtual ~ProtoBase();
};

class StreamClient : public ProtoBase {
private:
	void Summary() const;
	void Selector();
public:
	void Init();
	void Run();
	StreamClient(std::shared_ptr<Layer::BaseSock> _s=nullptr,
	    std::shared_ptr<Layer::SockHandler> _h=nullptr);
	virtual ~StreamClient();
};

class StreamServer : public ProtoBase {
private:
	class Csd {
	private:
#ifdef _WIN32
		CRITICAL_SECTION csl;
#else
		pthread_mutex_t csl;
#endif 
		std::map<uint64_t, int> _csd;
		Csd(const Csd&);
		Csd& operator=(const Csd&);
	public:
		int& operator[](uint64_t);
		void Erase(uint64_t);
		Csd();
		virtual ~Csd() {};
	};
	Csd csd;
	std::shared_ptr<Layer::SockPool> sp;
	std::shared_ptr<Threads::ThreadPool> tp;
	void Summary() const;
	void HandleConn();
	void Selector();
public:
	void Init();
	void Run();
	StreamServer(std::shared_ptr<Layer::BaseSock> _s=nullptr,
	    std::shared_ptr<Layer::SockHandler> _h=nullptr);
	virtual ~StreamServer();
};
} /* namespace Proxy */
#endif
/* tabstop=8 */
