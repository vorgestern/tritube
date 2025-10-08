
#include <windows.h>
#include <vector>
#include <string>
#include <format>
#include <filesystem>
#include <functional>

using namespace std;
using fspath=filesystem::path;

#include "winhelper.h"

pair<int,int> startpiped(prochelper&ph, const string&exec, const vector<string>&arguments)
{
    string commandline=format("\"{}\"", exec);
    for (auto&k: arguments) commandline.append(" "+k);

    HANDLE newstdin, write_stdin, newstdout, read_stdout, newstderr, read_stderr;

    // Erzeuge einen geeigneten SECURITY-Descriptor
    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, true, NULL, false);

    SECURITY_ATTRIBUTES sa;
    sa.lpSecurityDescriptor=&sd;
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle=true;                          // allow inheritable handles

    if (!CreatePipe(&newstdin, &write_stdin, &sa, 0)) return {1,GetLastError()};
    if (!SetHandleInformation(write_stdin, HANDLE_FLAG_INHERIT, 0)) return {11,GetLastError()};

    if (!CreatePipe(&read_stdout, &newstdout, &sa, 0))
    {
        auto e=GetLastError();
        CloseHandle(newstdin);
        CloseHandle(write_stdin);
        return {2,e};
    }
    if (!SetHandleInformation(read_stdout, HANDLE_FLAG_INHERIT, 0)) return {12,GetLastError()};

    if (!CreatePipe(&read_stderr, &newstderr, &sa, 0))
    {
        auto e=GetLastError();
        CloseHandle(newstdin);
        CloseHandle(write_stdin);
        CloseHandle(newstdout);
        CloseHandle(write_stdin);
        return {3,e};
    }
    if (!SetHandleInformation(read_stderr, HANDLE_FLAG_INHERIT, 0)) return {13,GetLastError()};

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
    auto e=f?0:GetLastError();

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
        return {0,e};
    }
    else
    {
        CloseHandle(read_stdout);
        CloseHandle(read_stderr);
        CloseHandle(write_stdin);
        // printf("CreateProcess failed; LastError=%d\n", GetLastError());
        // printf("    exec '%s'\n", exec.c_str());
        // printf("    args '%s'\n", commandline.data());
        return {4,e};
    }
}

void inputchannel_async::read()
{
    nr=0;
    if (!ReadFile(read_from_child, buffer, cap, &nr, &ol))
    {
        switch (GetLastError())
        {
            case ERROR_IO_PENDING: break;
            default:
            case ERROR_BROKEN_PIPE: closed=true; break;
        }
    }
}

void inputchannel_async::closehandles()
{
    CloseHandle(ol.hEvent);
    CloseHandle(read_from_child);
    closed=true;
}

int entertain(HANDLE hprocess, inputchannel_async&out, function<void()>handle_out, inputchannel_async&err, function<void()>handle_err)
{
    int rcx=-1;
    bool fterm=false;

    out.read();
    err.read();

    auto handle_process=[&fterm, &rcx, hprocess]()
    {
        unsigned long exitcode=STILL_ACTIVE;
        if (GetExitCodeProcess(hprocess, &exitcode))
        {
            if (exitcode!=STILL_ACTIVE){ rcx=exitcode; fterm=true; }
        }
        else fterm=true;
    };

    const int numobj=3;
    HANDLE objects[numobj]={out.ol.hEvent, err.ol.hEvent, hprocess};
    function<void()>handlers[numobj]={handle_out, handle_err, handle_process};
    unsigned nc=0;
    while (!fterm)
    {
        ++nc;
        if (nc&1)
        {
            objects[0]=out.ol.hEvent; handlers[0]=handle_out;
            objects[1]=err.ol.hEvent; handlers[1]=handle_err;
        }
        else
        {
            objects[0]=err.ol.hEvent; handlers[0]=handle_err;
            objects[1]=out.ol.hEvent; handlers[1]=handle_out;
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
    out.closehandles();
    err.closehandles();
    return rcx;
}

#include <algorithm>
#include <iostream>
#include <tritube/tritube.h>

using namespace tritube;

static vector<fspath> pathdirectories()
{
    vector<fspath>result;
    const string P=getenv("PATH");
    size_t pos=0;
    while (true)
    {
        auto s=P.find_first_of(';', pos);
        if (s!=P.npos){ result.push_back(P.substr(pos, s-pos)); pos=s+1; }
        else{ result.push_back(P.substr(pos)); break; }
    }
    return result;
}

static vector<fspath> acceptableextensions()
{
    vector<fspath>result;
    const string P=getenv("PathExt");
    size_t pos=0;
    while (true)
    {
        auto s=P.find_first_of(';', pos);
        if (s!=P.npos){ result.push_back(P.substr(pos, s-pos)); pos=s+1; }
        else{ result.push_back(P.substr(pos)); break; }
    }
    return result;
}

[[maybe_unused]] static ostream&operator<<(ostream&out, const vector<fspath>&X)
{
    for_each(X.begin(), X.end(), [n=0](const fspath&p) mutable { cout<<(n++>0?" ":"")<<p.string(); });
    return out;
}

// ============================================================================

static optional<fspath> exists_as(const fspath&X)
{
    const auto ext=X.extension();
    const auto acceptable=acceptableextensions();
    const bool extension_accepted=!ext.empty() && any_of(acceptable.begin(), acceptable.end(), [ext](const fspath&X)->bool { return X==ext; });
    if (extension_accepted && filesystem::exists(X)) return X;
    for (auto&a: acceptable)
    {
        const fspath P=X.string()+a.string();
        if (filesystem::exists(P)) return P;
    }
    return {};
}

optional<fspath> tritube::applpath(xfind f, string_view appp)
{
    fspath appl(appp);

    switch (f)
    {
        case xfdirect:
        {
            if (!appl.is_absolute())
            {
                error_code ec;
                const auto here=filesystem::current_path(ec);
                if (ec) return {};
                appl=here/appl;
            }
            return exists_as(appl);
        }
        case xfpath:
        {
            if (appl.is_absolute()) return exists_as(appl)?appl:optional<fspath>();
            const auto Paths=pathdirectories();
            for (auto&k: Paths) if (const auto k1=exists_as(k/appl); k1.has_value()) return k1;
            return {};
        }
        default: return {};
    }
}
