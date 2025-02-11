
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <forkpipes.h>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

static int exec_neu(const vector<string>&args)
{
    vector<char*>argv(args.size()+1);
    for (auto j=0u; j<args.size(); ++j) argv[j]=strdup(args[j].c_str());
    argv[args.size()]=nullptr;
    return execvp(argv[0], &argv[0]);
}

int main()
{
    printf("== %s\n", "This is your echodemo, launching echoserver.");
    forkpipes3 PP;
    if (ischild(PP))
    {
        exit(exec_neu({"./echoserver"}));
    }
    if (isparent(PP))
    {
        string input, input_err;
        if (true)
        {
            const string hello="I am echodemo\n";
            write(parent_write(PP), hello.c_str(), hello.size());
        }
        while (!isclosed(PP))
        {
            const auto [c,rc]=PP.readchar();
            switch (rc)
            {
                case 1:
                {
                    if (c!='\n')
                    {
                        input.push_back(c);
                    }
                    else
                    {
                        cout<<".. "<<input<<'\n';
                        input.clear();
                    }
                    break;
                }
                case 2:
                {
                    if (c!='\n')
                    {
                        input_err.push_back(c);
                    }
                    else
                    {
                        cout<<"== server complaining ('"<<input_err<<"').\n";
                        input_err.clear();
                        const string comfort="Don't be childish!";
                        cout<<"== Trying to calm him down ('"<<comfort<<"').\n";
                        write(parent_write(PP), comfort.c_str(), comfort.size());
                        write(parent_write(PP), "\n", 1);
                    }
                    break;
                }
            }
        }
    }
    return 0;
}
