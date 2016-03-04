/*-
 * Copyright 2015 - Datagram Garden OÜ - All rights reserved.
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

#ifndef CHECK_H
#define CHECK_H

#include <string>

#ifdef _WIN32
#define _WINSOCKAPI_
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace Debug {
	class Debug;
}

namespace Check {

enum ConfType {PTYPE=3, PPROTO=5};
enum ServType {DAEMON=1, SAFE, MAX, IAUTO};
enum ProtoType {CLIENT=1, ADDR, PORT, KEY, X509, CAX, TUNI, AC6, AC4};

class IConf {
protected:
#ifdef _WIN32
	mutable CRITICAL_SECTION csl;
#else
	mutable pthread_mutex_t csl;
#endif
	std::shared_ptr<Debug::Debug> logh;
public:
	virtual void Config(const std::string, const std::string)=0;
	virtual void *Config(const std::string)=0;
	virtual void Config(const int, const std::string);
	virtual void *Config(const int);
	IConf();
	virtual ~IConf();
};

class DConf : public IConf {
private:
	bool daemonize;
	uint16_t guard;
	uint32_t max;
	bool iauto;
	static bool reset;
public:
	void Config(const std::string, const std::string);
	void *Config(const std::string);
	static void SetReset();
	static bool GetReset();
	DConf();
	virtual ~DConf();
};

class CConf : public IConf {
private:
	bool client;
	std::string addr;
	uint16_t port;
	std::string cwd;
	std::string key;
	std::string cert;
	std::string cax;
	std::string tun;
	static bool status;
	std::string ac6;
	std::string ac4;
public:
	void Config(const std::string, const std::string);
	void *Config(const std::string);
	static void Status(const bool);
	static bool Status();
	CConf();
	virtual ~CConf();
};

class PConf {
private:
#ifdef _WIN32
	mutable CRITICAL_SECTION csl;
#else
	mutable pthread_mutex_t csl;
#endif
	std::shared_ptr<IConf> dconf;
	std::shared_ptr<IConf> cconf;
	std::shared_ptr<Debug::Debug> logh;
	void CheckFile(std::string&);
	void CheckConfig();
	void Summary() const;
	void Help() const;
public:
	std::shared_ptr<IConf> D() const;
	std::shared_ptr<IConf> C() const;
	const bool Client() const;
	const bool Daemon() const;
	const uint16_t Guard() const;
	const uint32_t Max() const;
	const bool Iauto() const;
	const std::string Addr() const;
	const uint16_t Port() const;
	const std::string Key() const;
	const std::string Cert() const;
	const std::string Cax() const;
	const std::string Tun() const;
	const std::string Ac6() const;
	const std::string Ac4() const;
	void Static(const std::string) const;
	void Dynamic(const std::string) const;
	PConf(int argc=0, char** argv=nullptr);
	virtual ~PConf();
};
} /* namespace Check */
#endif
/* tabstop=8 */
