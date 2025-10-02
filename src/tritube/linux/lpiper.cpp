
#include <tritube/tritube.h>
#include <functional>
#include <numeric>
#include <iostream>

#include "../../forkpipes.h"

#include <cstring>
#include <unistd.h>

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

static int process_inputs(forkpipes3&PP, string&X)
{
    string OutLine;
    vector<string> Items;
    while (!isclosed(PP))
    {
        const auto [c,fd]=PP.readchar();
        switch (fd)
        {
            case 1: // stdin
            {
                if (c!='\r') OutLine.push_back(c);
                if (c=='\n')
                {
                    Items.push_back(OutLine);
                    OutLine.clear();
                }
                break;
            }
            case 2: break;
        }
    }
    auto total=accumulate(Items.begin(), Items.end(), 0, [](size_t acc, string&X){ return acc+X.size(); });
    X.clear();
    X.reserve(total);
    for (auto&j: Items) X.append(j);
    return 0;
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
        string Out;
        [[maybe_unused]] const int rc=process_inputs(PP, Out);
        return Out;
    }
    else return {};
}

rc_out_err tritube::piper_roe(fspath&fullpath, const vector<string>&args)
{
    return {-1, {}, {}};
}

rc_Out_Err tritube::piper_roev(fspath&fullpath, const vector<string>&args)
{
    return {-1, {}, {}};
}

int tritube::piper_linewise(fspath&fullpath, const vector<string>&args, function<void(const string&)>process_stdout, function<void(const string&)>process_stderr)
{
    return -1;
}
