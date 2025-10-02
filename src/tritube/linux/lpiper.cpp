
#include <tritube/tritube.h>
#include <functional>
#include <numeric>
#include <iostream>

#include "../../forkpipes.h"

#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;
using namespace tritube;
using fspath=filesystem::path;

static int exec_neu(const fspath&appl, const vector<string>&args)
{
    vector<char*>argv(args.size()+2);
    auto j=0u;
    argv[j++]=strdup(appl.string().c_str());
    for (auto&k: args) argv[j++]=strdup(k.c_str());
    argv[j++]=nullptr;
    return execvp(argv[0], &argv[0]);
}

static int process_inputs(forkpipes3&PP, vector<string>&Out, vector<string>&Err)
{
    Out.clear();
    Err.clear();
    string OutLine, ErrLine;
    while (!isclosed(PP))
    {
        const auto [c,fd]=PP.readchar();
        switch (fd)
        {
            case 1: // stdin
            {
                if (c=='\n')
                {
                    Out.push_back(OutLine);
                    OutLine.clear();
                }
                else if (c!='\r') OutLine.push_back(c);
                break;
            }
            case 2:
            {
                if (c=='\n')
                {
                    Err.push_back(ErrLine);
                    ErrLine.clear();
                }
                else if (c!='\r') ErrLine.push_back(c);
                break;
            }
        }
    }
    if (const int pid=childpid(PP); pid>0)
    {
        int stat=0;
        waitpid(pid, &stat, 0);
        return WEXITSTATUS(stat);
    }
    else return -1;
}

string tritube::piper_o(fspath&fullpath, const vector<string>&args)
{
    forkpipes3 PP;
    if (ischild(PP))
    {
        exit(exec_neu(fullpath, args));
    }
    else if (isparent(PP))
    {
        close(parent_write(PP));
        vector<string> Out, Err;
        [[maybe_unused]] const int rc=process_inputs(PP, Out, Err);
        auto total=accumulate(Out.begin(), Out.end(), 0, [](size_t acc, string&X){ return acc+X.size(); });
        string Result;
        Result.reserve(total);
        for (auto&j: Out) Result.append(j+"\n");
        return Result;
    }
    else return {};
}

rc_out_err tritube::piper_roe(fspath&fullpath, const vector<string>&args)
{
    forkpipes3 PP;
    if (ischild(PP))
    {
        exit(exec_neu(fullpath, args));
    }
    else if (isparent(PP))
    {
        close(parent_write(PP));
        vector<string> OutLines, ErrLines;
        [[maybe_unused]] const int rc=process_inputs(PP, OutLines, ErrLines);
        string Out, Err;
        auto totalout=accumulate(OutLines.begin(), OutLines.end(), 0, [](size_t acc, string&X){ return acc+X.size(); }),
             totalerr=accumulate(ErrLines.begin(), ErrLines.end(), 0, [](size_t acc, string&X){ return acc+X.size(); });
        Out.reserve(totalout); for (auto&k: OutLines) Out.append(k+"\n");
        Err.reserve(totalerr); for (auto&k: ErrLines) Err.append(k+"\n");
        return {rc, Out, Err};
    }
    return {-1, {}, {}};
}

rc_Out_Err tritube::piper_roev(fspath&fullpath, const vector<string>&args)
{
    forkpipes3 PP;
    if (ischild(PP))
    {
        exit(exec_neu(fullpath, args));
    }
    else if (isparent(PP))
    {
        close(parent_write(PP));
        vector<string> Out, Err;
        [[maybe_unused]] const int rc=process_inputs(PP, Out, Err);
        return {rc, Out, Err};
    }
    return {-1, {}, {}};
}

int tritube::piper_linewise(fspath&fullpath, const vector<string>&args, function<void(const string&)>process_stdout, function<void(const string&)>process_stderr)
{
    return -1;
}
