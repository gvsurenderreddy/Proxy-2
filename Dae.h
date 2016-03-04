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

#ifndef DAE_H
#define DAE_H

#include <memory>
#include <string>

#include "SockHandler.h"

namespace Debug {
	class Debug;
}
namespace Deque {
	template<typename I, typename T>
	class Deque;
}
namespace TunTap {
	class Interface;
}
namespace Craft {
	class PacketBase;
}
namespace Layer {
	template<class IAX, class SAIX>
	class ActiveSock;
	class SockHandler;
}
namespace Check {
	class IConf;
}

namespace Proxy {

class BaseSocket {
private:
	BaseSocket(const BaseSocket&);
	BaseSocket& operator=(const BaseSocket&);
protected:
	std::shared_ptr<Check::PConf> confh;
	std::shared_ptr<Debug::Debug> logh;
public:
	virtual void Init()=0;
	virtual void Run()=0;
	BaseSocket();
	virtual ~BaseSocket();
};

class Iface;
class InnerSocket : public BaseSocket {
private:
	InnerSocket(const InnerSocket&);
	InnerSocket& operator=(const InnerSocket&);
protected:
	std::shared_ptr<TunTap::Interface> iface;
	std::shared_ptr<Iface> app;
public:
	void Init();
	void Run();
	InnerSocket();
	virtual ~InnerSocket();
};

class ProtoBase;
class OuterSocket : public BaseSocket {
private:
	OuterSocket(const OuterSocket&);
	OuterSocket& operator=(const OuterSocket&);
protected:
	std::shared_ptr<Layer::ActiveSock<Layer::IA6, Layer::SAI6>> sd;
	std::shared_ptr<ProtoBase> app;
	std::shared_ptr<Layer::SockHandler> tls;
	std::shared_ptr<Layer::SockHandler> tcp;
	std::shared_ptr<Layer::SockHandler> ip;
	void InitClient(std::string, uint16_t);
	void InitServer(std::string, uint16_t);
	void InitSsl();
public:
	virtual void Init();
	virtual void Run();
	OuterSocket();
	virtual ~OuterSocket();
};

class ControlSocket : public OuterSocket {
private:
	ControlSocket(const ControlSocket&);
	ControlSocket& operator=(const ControlSocket&);
public:
	void Init();
	void Run();
	ControlSocket();
	virtual ~ControlSocket();
};

} /* namespace Proxy */
#endif
/* tabstop=8 */
