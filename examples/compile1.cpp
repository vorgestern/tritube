
#include <cstring>
#include <unistd.h>
#include <forkpipes.h>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

const auto A=R"__(
#include <cstdio>
#include <string>
#pragma message("hoppla")
using namespace std;
int main()
{
    string name="abc";
    for (auto j=0; j<name.size(); ++j){}
    return 0;
}
)__"sv;

static int exec_neu(const vector<string>&args)
{
    vector<char*>argv(args.size()+1);
    for (auto j=0u; j<args.size(); ++j) argv[j]=strdup(args[j].c_str());
    argv[args.size()]=nullptr;
    return execvp(argv[0], &argv[0]);
}

static int process_inputs(forkpipes3&PP, vector<string>&Items)
{
    const string ii {">> "}, ee {"ee "};
    char pad[200], err[200];
    unsigned nump=0, nume=0;
    while (!isclosed(PP))
    {
        const auto [c,rc]=PP.readchar();
        switch (rc)
        {
            case 1: // stdin
            {
                if (c=='\n'||c=='\r')
                {
                    pad[nump]=0;
                    nump=0;
                    Items.push_back(ii+pad);
                }
                else if (nump<sizeof pad-1) pad[nump++]=c;
                break;
            }
            case 2: // errin
            {
                if (c=='\n'||c=='\r')
                {
                    err[nume]=0;
                    nume=0;
                    Items.push_back(ee+err);
                }
                else if (nume<sizeof err-1) err[nume++]=c;
                break;
            }
        }
    }
    return 0;
}

static int task_demo(forkpipes3&PP)
{
    int rc=0;
    if (ischild(PP))
    {
        exit(exec_neu({"g++", "-Werror", "-Wall", "-o", "hoppla.o", "-x", "c++", "-"}));
    }
    else if (isparent(PP))
    {
        write(parent_write(PP), A.data(), A.size());
        close(parent_write(PP));
        vector<string>Items;
        Items.clear();
        Items.push_back("***** Start  *****");
        rc=process_inputs(PP, Items);
        Items.push_back("***** Finish *****");
        for (auto&k: Items) printf("%s\n", k.c_str());
    }
    return rc;
}

static int task_compile(forkpipes3&PP, string&tocompile)
{
    int rc=0;
    if (ischild(PP))
    {
        exit(exec_neu({"g++", "-c", "-Werror", "-Wall", "-I", "src", "-o", "hoppla.o", tocompile}));
    }
    else if (isparent(PP))
    {
        write(parent_write(PP), A.data(), A.size());
        close(parent_write(PP));
        vector<string>Items;
        Items.clear();
        Items.push_back("***** Start  *****");
        rc=process_inputs(PP, Items);
        Items.push_back("***** Finish *****");
        for (auto&k: Items) printf("%s\n", k.c_str());
    }
    return rc;
}

int main(int argc, char*argv[])
{
    enum {ttdemo, ttcompile, ttecho} task {ttdemo};
    string tocompile;
    if (argc>1)
    {
        string_view arg=argv[1];
        if (arg=="compile"sv && argc>2){ task=ttcompile; tocompile=argv[2]; }
    }
    forkpipes3 PP;
    switch (task)
    {
        case ttdemo: return task_demo(PP);
        case ttcompile: return task_compile(PP, tocompile);
        default: break;
    }
    return 0;
}
