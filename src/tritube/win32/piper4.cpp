
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

string tritube::piper4_o(fspath&fullpath, string_view args)
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
cout<<"piper4_o ok; "<<so<<"\n";
        return so;
    }
    else return {};
}

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
