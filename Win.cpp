/*
 *    Portions taken from Microsoft SDK
 *    http://msdn.microsoft.com/
 */

#ifdef __WIN32
#define _WINSOCKAPI_
#include "NetRatConf.h"
#include "Trace.h"
#include "Configurator.h"
#include "Svc.h"
#include "SvcWin.h"
#include "Tools.h"
#include "Threads.h"
#include "Logger.h"
#include "SockHandler.h"
#include <netfw.h>
#include <Strsafe.h>

namespace Proxy {

    VOID SvcWin::SvcInit(DWORD dwArgc, LPTSTR *lpszArgv)
    {
        logh->Log("[SvcWin::SvcInit]");
#ifdef ICMPENABLE
        logh->Log("[SvcWin::SvcInit]: adding ICMP_ALL (*:*) rule");
        EnableICMP_IN();
        Sleep(FIRE_FACTOR*SAFE_GUARD);
        EnableICMP_OUT();
        Sleep(FIRE_FACTOR*SAFE_GUARD);
#endif
#ifdef DISFIRW
        logh->Log("[SvcWin::SvcInit]: disabling firewall");
        DisableFirewall();
#endif
        logh->Log("[SvcWin::SvcInit]: managing service specific threads");
        _NetTools::STI _tid;
        HANDLE serv, comm;
        _NetTools::Xthread::Wthread_create(&serv, SvcAbstractionLayer::SvcMonitorStop, 0, &_tid);
        logh->Log("[SvcWin::SvcInit]: service thread ", _tid);
        _NetTools::Xthread::Wthread_create(&comm, [](LPVOID _t)->DWORD {
            (static_cast<SvcWin*>(_t))->SvcMain(0);
            return 1;
        }, (LPVOID)this, &_tid);
        logh->Log("[SvcWin::SvcInit]: communication thread ", _tid);
        _NetTools::Xthread::Wthread_join(&serv, INFINITE);
        logh->Log("[SvcWin::SvcInit]: service thread terminated");
        _NetTools::Xthread::Wthread_terminate(&comm, 0);
        logh->Log("[SvcWin::SvcInit]: communication thread terminated");
        CloseHandle(comm);
        CloseHandle(serv);
        logh->Log("[SvcWin::SvcInit]: BYE BYE ...");
    }

    DWORD WINAPI SvcWin::SvcMain(LPVOID arg)
    {
        logh->Log("[SvcWin::SvcMain]");
        SvcDynamicSetUp();
        logh->Log("[SvcWin::SvcMain]: preparing protocol threads");
        DWORD tv;
        while (!_Configurator::ServiceConfigurator::GetConfig() && !_Configurator::ServiceConfigurator::GetReset()) {
            tv = (*static_cast<const int*>(servh->Config(_Configurator::SAFE)))*THREAD_FACTOR;
            if (tcpFlag) {
                logh->Log("[SvcWin::SvcMain]: tcpMode enabled, creating its protocol thread");
                tcpThread = std::make_shared<_NetTools::SmartThread>([](LPVOID _t)->DWORD {
                    auto _x=static_cast<SvcWin*>(_t);
                    _x->SvcTcpMode();
                    _x->SetProtoFlag(_x->tcpFlag, true);
                    return 1;
                }, (LPVOID)this);
                SetProtoFlag(tcpFlag, false);
                status->Add(tcpThread);
            }
            if (udpFlag) {
                logh->Log("[SvcWin::SvcMain]: udpMode enabled, creating its protocol thread");
                udpThread = std::make_shared<_NetTools::SmartThread>([](LPVOID _t)->DWORD {
                    auto _x=static_cast<SvcWin*>(_t);
                    _x->SvcUdpMode();
                    _x->SetProtoFlag(_x->udpFlag, true);
                    return 1;
                }, (LPVOID)this);
                SetProtoFlag(udpFlag, false);
                status->Add(udpThread);
            }
            if (sctpFlag) {
                logh->Log("[SvcWin::SvcMain]: sctpMode enabled, creating its protocol thread");
                sctpThread = std::make_shared<_NetTools::SmartThread>([](LPVOID _t)->DWORD {
                    auto _x=static_cast<SvcWin*>(_t);
                    _x->SvcSctpMode();
                    _x->SetProtoFlag(_x->sctpFlag, true);
                    return 1;
                }, (LPVOID)this);
                SetProtoFlag(sctpFlag, false);
                status->Add(sctpThread);
            }
            logh->Log("[SvcWin::SvcMain]: blocking in Select");
            Sleep(tv);
            logh->Log("[SvcWin::SvcMain]: checking comm threads status ...");
            if (!_Configurator::ServiceConfigurator::GetConfig())
                logh->Log("[SvcWin::SvcMain]: cycling #thread ", status->Cycle());
        }
        status->Join();
        SvcDynamicCleanUp();
        logh->Log("[SvcWin::SvcMain]: service thread ended");
        logh->Log("[SvcWin::SvcMain]: pool size ", pool->Size());
        if (!_Configurator::ServiceConfigurator::GetReset()) {
            _Configurator::ServiceConfigurator::SetConfig(false);
            logh->Log("[SvcWin::SvcMain]: recovering ...");
            SvcMain(0);
        } else
            logh->Log("[SvcWin::SvcMain]: tearing Down gently  ");
        return 0;
    }

    SvcWin::SvcWin()
        : Svc(std::shared_ptr<Svc>(this, [](Svc* _){;}))
    {
        logh->Log("[SvcWin::SvcWin]");
    }

    SvcWin::~SvcWin()
    {
        logh->Log("[SvcWin::~SvcWin]");
    }

} /*namespace*/
#endif
