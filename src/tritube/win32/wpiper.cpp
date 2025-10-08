
#include <tritube/tritube.h>
#include <windows.h>
#include <functional>

using namespace std;
using namespace tritube;
using fspath=filesystem::path;

#include "winhelper.h"

static pair<bool, size_t>write_initial_input(HANDLE write_to_stdin, string_view initial_input)
{
    bool receiver_closed=false;
    DWORD numwritten=0;
    while (numwritten<initial_input.size() && !receiver_closed)
    {
        DWORD nw=0;
        const bool f=WriteFile(write_to_stdin, initial_input.data(), (DWORD)initial_input.size(), &nw, nullptr);
        numwritten+=nw;
        if (!f)
        {
            switch (GetLastError())
            {
                case ERROR_BROKEN_PIPE: receiver_closed=true; break;
                case ERROR_INVALID_USER_BUFFER: break;
                case ERROR_NOT_ENOUGH_MEMORY: break;
                case ERROR_NOT_ENOUGH_QUOTA: break;
                case ERROR_OPERATION_ABORTED: receiver_closed=true; break;
            }
        }
    }
    return {receiver_closed, numwritten};
}

string tritube::piper_o(fspath&fullpath, const vector<string>&args, string_view initial_input)
{
    prochelper ph;
    const auto [rcstart,e]=startpiped(ph, fullpath.string(), args);
    if (rcstart!=0 || e!=0) return {};

    [[maybe_unused]] auto [f,nw]=write_initial_input(ph.write_to_stdin, initial_input);
    CloseHandle(ph.write_to_stdin);

    char outbuffer[1024], errbuffer[1024];
    struct channel: public inputchannel_async
    {
        vector<string>merk {};
    } out({ph.read_from_stdout, ph.olout, outbuffer, sizeof outbuffer}),
      err({ph.read_from_stderr, ph.olerr, errbuffer, sizeof errbuffer});

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

    if (const int rcx=entertain(ph.process, out, handle_stdout, err, handle_stderr); rcx==0)
    {
        string so;
        for (auto&m: out.merk) so.append(m);
        return so;
    }
    else return {};
}

rc_out_err tritube::piper_roe(fspath&fullpath, const vector<string>&args, string_view initial_input)
{
    prochelper ph;
    const auto [rcstart,e]=startpiped(ph, fullpath.string(), args);
    if (rcstart!=0) return {e,{},{}};

    [[maybe_unused]] auto [f,nw]=write_initial_input(ph.write_to_stdin, initial_input);
    CloseHandle(ph.write_to_stdin);

    char outbuffer[1024], errbuffer[1024];
    struct channel: public inputchannel_async
    {
        vector<string>merk {};
    } out({ph.read_from_stdout, ph.olout, outbuffer, sizeof outbuffer}),
      err({ph.read_from_stderr, ph.olerr, errbuffer, sizeof errbuffer});

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

    const int rcx=entertain(ph.process, out, handle_stdout, err, handle_stderr);
    string so, se;
    for (auto&m: out.merk) so.append(m);
    for (auto&m: err.merk) se.append(m);
    return {rcx, so, se};
}

rc_Out_Err tritube::piper_roev(fspath&fullpath, const vector<string>&args, string_view initial_input)
{
    prochelper ph;
    const auto [rcstart,e]=startpiped(ph, fullpath.string(), args);
    if (rcstart!=0) return {e,{},{}};

    [[maybe_unused]] auto [f,nw]=write_initial_input(ph.write_to_stdin, initial_input);
    CloseHandle(ph.write_to_stdin);

    char outbuffer[1024], errbuffer[1024];
    struct channel: public inputchannel_async
    {
        vector<string>merk {};
    } out({ph.read_from_stdout, ph.olout, outbuffer, sizeof outbuffer}),
      err({ph.read_from_stderr, ph.olerr, errbuffer, sizeof errbuffer});

    auto ls=[](vector<string>&X, bool current_line_terminated, const char buffer[], size_t len)->bool
    {
        const char*p=buffer;
        const char*pdone=buffer+len;
        bool b=current_line_terminated;
        while (p<pdone)
        {
            auto s=strchr(p, '\n');
            if (s)
            {
                if (X.size()>0&&!b) X.back().append(p, s-p);
                else X.emplace_back(p, s-p);
                p=s+1;
                b=true;
            }
            else
            {
                if (X.size()>0&&!b) X.back().append(p);
                else X.emplace_back(p);
                p=pdone;
                b=false;
            }
        }
        return b;
    };

    vector<string>Out, Err;

    auto handle_stderr=[ls,&err,&Err,lterm=false]() mutable
    {
        if (err.nr<=0) return;
        lterm=ls(Err, lterm, err.buffer, err.nr);
        err.read();
    };

    auto handle_stdout=[ls,&out,&Out,lterm=false]() mutable
    {
        if (out.nr<=0) return;
        lterm=ls(Out, lterm, out.buffer, out.nr);
        out.read();
    };

    const int rcx=entertain(ph.process, out, handle_stdout, err, handle_stderr);
    return {rcx, Out, Err};
}

int tritube::piper_linewise(fspath&fullpath, const vector<string>&args, function<void(const string&)>process_stdout, function<void(const string&)>process_stderr, string_view initial_input)
{
    prochelper ph;
    const auto [rcstart,e]=startpiped(ph, fullpath.string(), args);
    if (rcstart!=0) return e;

    [[maybe_unused]] auto [f,nw]=write_initial_input(ph.write_to_stdin, initial_input);
    CloseHandle(ph.write_to_stdin);

    char outbuffer[1], errbuffer[1];
    struct channel: public inputchannel_async
    {
        vector<string>merk {};
    } out({ph.read_from_stdout, ph.olout, outbuffer, sizeof outbuffer}),
      err({ph.read_from_stderr, ph.olerr, errbuffer, sizeof errbuffer});

    string Out, Err;

    auto handle_stderr=[process_stderr,&err,&Err]()
    {
        if (err.nr==1 && process_stderr) switch (err.buffer[0])
        {
            case '\n': process_stderr(Err); Err.clear(); break;
            case '\r': break;
            default: Err.push_back(err.buffer[0]); break;
        }
        err.read();
    };

    auto handle_stdout=[process_stdout,&out,&Out]()
    {
        if (out.nr==1 && process_stdout) switch (out.buffer[0])
        {
            case '\n': process_stdout(Out); Out.clear(); break;
            case '\r': break;
            default: Out.push_back(out.buffer[0]); break;
        }
        out.read();
    };

    return entertain(ph.process, out, handle_stdout, err, handle_stderr);
}
