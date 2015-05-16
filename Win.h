#ifdef __WIN32
#ifndef SVCWIN_H
#define SVCWIN_H

#include "Svc.h"

namespace Proxy {

    class SvcWin : public Svc {
    private:
    public:
        VOID SvcInit(DWORD, LPTSTR *);
        DWORD WINAPI SvcMain(LPVOID);
        SvcWin();
        virtual ~SvcWin();
    };
}
#endif
#endif
