
#include <iostream>
#include <tritube/tritube.h>

using namespace std;
using namespace tritube;

void demo_default()
{
    auto Text=exec_sync<string, xfpath>("git.exe", "--version");
    if (Text.size()>0) cout<<"======================\n"<<Text<<"\n======================\n";
    else cout<<"No output for 'git.exe --version\n";
}

int main(int argc, char*argv[])
{
    if (argc>1)
    {
        const string prog=argv[1];
        string args;
        for (auto j=2; j<argc; ++j) args.append(j>2?" "s+argv[j]:argv[j]);
        auto Text=exec_sync<string, xfpath>(prog, args);
        if (Text.size()>0) cout<<"======================\n"<<Text<<"\n======================\n";
        else cout<<"No output for '"<<prog<<" "<<args<<"\n";
    }
    else demo_default();
    return 0;
}
