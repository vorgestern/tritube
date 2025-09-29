
#include <windows.h>
#include <string>
#include <format>
#include <filesystem>
#include "winhelper.h"

using namespace std;
using fspath=filesystem::path;

int startpiped(prochelper&ph, const string&exec, string_view arguments)
{
    string commandline=format("\"{}\" {}", exec, arguments);

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
