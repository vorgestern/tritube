
#include <cstring>
#include <unistd.h>
#include <forkpipes.h>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

const auto A=R"__(

#include <cstdio>
int main()
{
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

int main()
{
    forkpipes3 PP;
    if (ischild(PP))
    {
        exit(exec_neu({"g++", "-o", "hoppla.o", "-x", "c++", "-"}));
    }
    else if (isparent(PP))
    {
        write(parent_write(PP), A.data(), A.size());
        close(parent_write(PP));
        vector<string>Items;
        Items.clear();
        Items.push_back("***** Start  *****");
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
                        Items.push_back(string {">> "}+pad);
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
                        Items.push_back(string {"ee "}+err);
                    }
                    else if (nume<sizeof err-1) err[nume++]=c;
                    break;
                }
            }
        }
        Items.push_back("***** Fertig *****");
        for (auto&k: Items) printf("%s\n", k.c_str());
    }
    return 0;
}
