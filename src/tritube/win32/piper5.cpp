
#include <tritube/tritube.h>
#include <windows.h>
#include <cstdio>
#include <string>
#include <string_view>
#include <format>
#include <functional>
#include <iostream>

using namespace std;
using namespace tritube;
using fspath=filesystem::path;

struct prochelper
{
    HANDLE process, write_to_stdin, read_from_stdout, read_from_stderr;
    OVERLAPPED olout, olerr;
};

static int startpiped(prochelper&ph, const string&exec, string_view arguments)
{
    string commandline=format("{} {}", "git", arguments);

cout<<format("startpiped: {} ... ({})\n", exec, commandline);

    HANDLE newstdin, write_stdin, newstdout, read_stdout, newstderr, read_stderr;

    // Erzeuge einen geeigneten SECURITY-Descriptor
    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, true, NULL, false);

    SECURITY_ATTRIBUTES sa;
    sa.lpSecurityDescriptor=&sd;
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle=true;                          // allow inheritable handles

    if (!CreatePipe(&newstdin, &write_stdin, &sa, 0)) return 1;
    if (!SetHandleInformation(write_stdin, HANDLE_FLAG_INHERIT, 0)) return 11;

    if (!CreatePipe(&read_stdout, &newstdout, &sa, 0))
    {
        CloseHandle(newstdin);
        CloseHandle(write_stdin);
        return 2;
    }
    if (!SetHandleInformation(read_stdout, HANDLE_FLAG_INHERIT, 0)) return 12;

    if (!CreatePipe(&read_stderr, &newstderr, &sa, 0))
    {
        CloseHandle(newstdin);
        CloseHandle(write_stdin);
        CloseHandle(newstdout);
        CloseHandle(write_stdin);
        return 3;
    }
    if (!SetHandleInformation(read_stderr, HANDLE_FLAG_INHERIT, 0)) return 13;

    // Erzeuge ein startup-info aus dem fuer diesen Prozess gueltigen.
    // Ueberschreibe in der Kopie die Angaben zu den Standard-IO-Kanaelen
    //   The dwFlags member tells CreateProcess how to make the process.
    //   STARTF_USESTDHANDLES validates the hStd* members.
    //   STARTF_USESHOWWINDOW validates the wShowWindow member.
    STARTUPINFO si;
    GetStartupInfo(&si);
    si.dwFlags=STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.wShowWindow=SW_HIDE;
    si.hStdOutput=newstdout;
    si.hStdError=newstderr;
    si.hStdInput=newstdin;

    PROCESS_INFORMATION pi;
    const BOOL f=CreateProcess(exec.c_str(), commandline.data(),0L,0L,TRUE,0,0L,0L,&si,&pi);
    CloseHandle(newstdin);
    CloseHandle(newstdout);
    CloseHandle(newstderr);
    CloseHandle(pi.hThread);

    if (f)
    {
        ph.process=pi.hProcess;
        ph.write_to_stdin=write_stdin;
        ph.read_from_stdout=read_stdout;
        ph.read_from_stderr=read_stderr;
        ZeroMemory(&ph.olout, sizeof(OVERLAPPED));
        ZeroMemory(&ph.olerr, sizeof(OVERLAPPED));
        ph.olout.hEvent=CreateEvent(&sa, FALSE, FALSE, "event_stdout");
        ph.olerr.hEvent=CreateEvent(&sa, FALSE, FALSE, "event_stderr");
        return 0;
    }
    else
    {
        CloseHandle(read_stdout);
        CloseHandle(read_stderr);
        CloseHandle(write_stdin);
        return 3;
    }
}

string piper5(fspath&fullpath, string_view args)
{
    vector<string>merk;

    prochelper ph;
    const int rc0=startpiped(ph, fullpath.string(), args);
    if (rc0==0)
    {
        printf("piper5: running\n");
        if (false)
        {
            DWORD nw;
            const char info[]="So kommen wir der Sache schon naeher.\n";
            BOOL f=WriteFile(ph.write_to_stdin, info, sizeof info, &nw, nullptr);
            if (!f) fprintf(stderr, "Failed to write to stdin\n");
        }
        CloseHandle(ph.write_to_stdin);

        bool err_from_stdout=false;
        char ccout[1024]="";
        DWORD nrout=0;
        if (!ReadFile(ph.read_from_stdout, ccout, sizeof ccout, &nrout, &ph.olout))
        {
            const int err=GetLastError();
            printf("Kann Lesevorgang von stdout nicht starten (%d).\n", err);
        }
        bool fterm=false;

        auto handle_stdout=[&nrout,&ccout,&err_from_stdout,&ph,&merk]()
        {
            if (nrout>0)
            {
                if (nrout>0)
                {
                    // fwrite(ccout, 1, nrout, stdout);
                    merk.emplace_back(ccout);
                    if (!ReadFile(ph.read_from_stdout, ccout, sizeof ccout, &nrout, &ph.olout))
                    {
                        auto err=GetLastError();
                        switch (err)
                        {
                            case ERROR_IO_PENDING: break;
                            case ERROR_BROKEN_PIPE: err_from_stdout=true; printf("=========== err from stdout ============\n"); break;
                            default:
                            {
                                printf("std input: err=%d\n", err);
                                break;
                            }
                        }
                    }
                }
            }
        };

        auto handle_process=[&fterm, &err_from_stdout, &ph]()
        {
            unsigned long exitcode=STILL_ACTIVE;
            BOOL f=GetExitCodeProcess(ph.process, &exitcode);
            fterm=!f || exitcode!=STILL_ACTIVE;
            if (fterm) printf("Terminate (outpipe %s)\n", err_from_stdout?"broken":"open");
        };

        const int numobj=2;
        HANDLE objects[numobj]={ph.olout.hEvent, ph.process};
        function<void()>handlers[numobj]={handle_stdout, handle_process};
        while (!fterm)
        {
            auto rc=WaitForMultipleObjects(numobj, objects, FALSE, 1000);
            if (rc>=WAIT_OBJECT_0 && rc<WAIT_OBJECT_0+numobj)
            {
                const int obj=rc-WAIT_OBJECT_0;
                switch (obj)
                {
                    case 0: handlers[0](); break;
                    case 1: handlers[1](); break;
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
                // printf("Beide Pipes sind unterbrochen.\n");
                // fterm=true;
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
    else printf("piper5: startpiped fail: rc0=%d\n", rc0);
    string s;
    for (auto&m: merk) s.append(m);
    return s;
}
