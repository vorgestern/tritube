
#include <tritube/tritube.h>
#include <windows.h>
#include <cstdio>
#include <string>
// #include <string_view>
// #include <format>
#include <functional>
#include <iostream>
#include "winhelper.h"

using namespace std;
using namespace tritube;
using fspath=filesystem::path;

string piper4(fspath&fullpath, string_view args)
{
    vector<string>merk;

    prochelper ph;
    const int rc0=startpiped(ph, fullpath.string(), args);
    if (rc0==0)
    {
//      printf("piper4: running\n");
        if (false)
        {
            DWORD nw;
            const char info[]="So kommen wir der Sache schon naeher.\n";
            BOOL f=WriteFile(ph.write_to_stdin, info, sizeof info, &nw, nullptr);
            if (!f) fprintf(stderr, "Failed to write to stdin\n");
        }
        CloseHandle(ph.write_to_stdin);

        bool err_from_stderr=false, err_from_stdout=false;
        char ccout[1024]="", ccerr[1024]="";
        DWORD nrout=0, nrerr=0;
        if (!ReadFile(ph.read_from_stdout, ccout, sizeof ccout, &nrout, &ph.olout))
        {
            err_from_stdout=true;
//          const int err=GetLastError();
//          printf("Kann Lesevorgang von stdout nicht starten (%d).\n", err);
        }
        if (!ReadFile(ph.read_from_stderr, ccerr, sizeof ccerr, &nrerr, &ph.olerr))
        {
            err_from_stderr=true;
//          const int err=GetLastError();
//          printf("Kann Lesevorgang von stderr nicht starten (%d).\n", err);
        }
        bool fterm=false;

        auto handle_stderr=[&nrerr,&ccerr,&err_from_stderr,&ph]()
        {
            if (nrerr>0)
            {
//              printf("<");
                fwrite(ccerr, 1, nrerr, stdout);
//              printf(">");
                if (!ReadFile(ph.read_from_stderr, ccerr, sizeof ccerr, &nrerr, &ph.olerr))
                {
                    auto err=GetLastError();
                    switch (err)
                    {
                        case ERROR_IO_PENDING: break;
                        case ERROR_BROKEN_PIPE: err_from_stderr=true; break;
                        default:
                        {
                            printf("error input: err=%d\n", err);
                            break;
                        }
                    }
                }
            }
        };

        auto handle_stdout=[&nrout,&ccout,&err_from_stdout,&ph,&merk]()
        {
            if (nrout>0)
            {
                merk.emplace_back(ccout);
                if (!ReadFile(ph.read_from_stdout, ccout, sizeof ccout, &nrout, &ph.olout))
                {
                    auto err=GetLastError();
                    switch (err)
                    {
                        case ERROR_IO_PENDING: break;
                        case ERROR_BROKEN_PIPE: err_from_stdout=true; break;
                        default:
                        {
                            printf("std input: err=%d\n", err);
                            break;
                        }
                    }
                }
            }
        };

        auto handle_process=[&fterm, &err_from_stderr, &err_from_stdout, &ph]()
        {
            unsigned long exitcode=STILL_ACTIVE;
            BOOL f=GetExitCodeProcess(ph.process, &exitcode);
            fterm=!f || exitcode!=STILL_ACTIVE;
//          if (fterm) printf("Terminate (pipes: out %s, err %s)\n", err_from_stdout?"broken":"open", err_from_stderr?"broken":"open");
        };

        const int numobj=3;
        HANDLE objects[numobj]={ph.olout.hEvent, ph.olerr.hEvent, ph.process};
        function<void()>handlers[numobj]={handle_stdout, handle_stderr, handle_process};
        unsigned nc=0;
        while (!fterm)
        {
            ++nc;
            if (nc&1)
            {
                objects[0]=ph.olout.hEvent; handlers[0]=handle_stdout;
                objects[1]=ph.olerr.hEvent; handlers[1]=handle_stderr;
            }
            else
            {
                objects[0]=ph.olerr.hEvent; handlers[0]=handle_stderr;
                objects[1]=ph.olout.hEvent; handlers[1]=handle_stdout;
            }
            auto rc=WaitForMultipleObjects(numobj, objects, FALSE, 1000);
            if (rc>=WAIT_OBJECT_0 && rc<WAIT_OBJECT_0+numobj)
            {
                const int obj=rc-WAIT_OBJECT_0;
                switch (obj)
                {
                    case 0: handlers[0](); break;
                    case 1: handlers[1](); break;
                    case 2: handlers[2](); break;
                    default:
                    {
                        printf("WFMO: unexpected object %d\n", obj);
                        break;
                    }
                }
            }
            else if (rc>=WAIT_ABANDONED_0 && rc<WAIT_ABANDONED_0+numobj)
            {
                // TSNH
                printf("WAIT_ABANDONED %d\n", rc-WAIT_ABANDONED_0);
                break;
            }
            else if (rc==WAIT_TIMEOUT)
            {
                if (err_from_stdout && err_from_stderr)
                {
                    printf("Beide Pipes sind unterbrochen.\n");
                    fterm=true;
                }
                break;
            }
            else if (rc==WAIT_FAILED)
            {
                auto err=GetLastError();
                printf("WAIT_FAILED rc=%d\n", err);
                break;
            }
            else
            {
                printf("WFMO: unexpected rc=0x%08x %d\n", rc, rc);
                break;
            }
        }
    }
    else printf("piper4: startpiped fail: rc0=%d\n", rc0);
    string s;
    for (auto&m: merk) s.append(m);
    return s;
}

// ============================================================================

struct inputchannel_async
{
    HANDLE read_from_child;
    OVERLAPPED ol;
    bool closed {false};
    char buffer[1024];
    DWORD nr {0};
    void read()
    {
        nr=0;
        if (!ReadFile(read_from_child, buffer, sizeof buffer, &nr, &ol))
        {
            switch (GetLastError())
            {
                case ERROR_IO_PENDING: break;
                default:
                case ERROR_BROKEN_PIPE: closed=true; break;
            }
        }
    }
};

rc_out_err tritube::piper4_roe(fspath&fullpath, string_view args)
{
    prochelper ph;
    const int rcstart=startpiped(ph, fullpath.string(), args);
    if (rcstart!=0) return {-1,{},{}};

    CloseHandle(ph.write_to_stdin);

    struct channel: public inputchannel_async
    {
        vector<string>merk {};
    } out={ph.read_from_stdout, ph.olout},
      err={ph.read_from_stderr, ph.olerr};

    int rcx=-1;
    bool fterm=false;

    out.read();
    err.read();

    auto handle_stderr=[&err]()
    {
        if (err.nr<=0) return;
        err.merk.emplace_back(err.buffer, err.nr);
        err.read();
    };

    auto handle_stdout=[&out]()
    {
        if (out.nr<=0) return;
        out.merk.emplace_back(out.buffer, out.nr);
        out.read();
    };

    auto handle_process=[&fterm, &rcx, ph]()
    {
        unsigned long exitcode=STILL_ACTIVE;
        if (GetExitCodeProcess(ph.process, &exitcode))
        {
            if (exitcode!=STILL_ACTIVE){ rcx=exitcode; fterm=true; }
        }
        else fterm=true;
    };

    const int numobj=3;
    HANDLE objects[numobj]={ph.olout.hEvent, ph.olerr.hEvent, ph.process};
    function<void()>handlers[numobj]={handle_stdout, handle_stderr, handle_process};
    unsigned nc=0;
    while (!fterm)
    {
        ++nc;
        if (nc&1)
        {
            objects[0]=out.ol.hEvent; handlers[0]=handle_stdout;
            objects[1]=err.ol.hEvent; handlers[1]=handle_stderr;
        }
        else
        {
            objects[0]=err.ol.hEvent; handlers[0]=handle_stderr;
            objects[1]=out.ol.hEvent; handlers[1]=handle_stdout;
        }
        const auto r=WaitForMultipleObjects(numobj, objects, FALSE, 1000);
        if (r>=WAIT_OBJECT_0 && r<WAIT_OBJECT_0+numobj) handlers[r-WAIT_OBJECT_0]();
        else if (r>=WAIT_ABANDONED_0 && r<WAIT_ABANDONED_0+numobj) break; // TSNH
        else if (r==WAIT_TIMEOUT)
        {
            if (out.closed && err.closed) fterm=true;
            break;
        }
        else if (r==WAIT_FAILED){ fterm=true; break; } // TSNH
        else                    { fterm=true; break; } // TSNH
    }
    string so, se;
    for (auto&m: out.merk) so.append(m);
    for (auto&m: err.merk) se.append(m);
    return {rcx, so, se};
}
