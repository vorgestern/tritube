
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
        if (auto p=applpath<xfpath>(argv[1]); p.has_value())
        {
            string args;
            for (auto j=2; j<argc; ++j) args.append(j>2?" "s+argv[j]:argv[j]);
            auto Text=exec_simple<string>(p.value(), args);
            if (Text.size()>0) cout<<"======================\n"<<Text<<"\n======================\n";
            else cout<<"No output for '"<<p.value().string()<<" "<<args<<"\n";
        }
        else cout<<"Executable not found for '"<<argv[1]<<"'\n";
    }
    else if constexpr (false && argc>1)
    {
        if (auto p=applpath<xfpath>(argv[1]); p.has_value())
        {
            string args;
            for (auto j=2; j<argc; ++j) args.append(j>2?" "s+argv[j]:argv[j]);
            auto [rc, out, err]=exec_simple<rc_out_err>(p.value(), args);
            cout<<  "== return code =================================\n"<<rc
                <<"\n== stdout ======================================\n"<<out
                <<"\n== stderr ======================================\n"<<err
                <<"\n================================================\n";
        }
        else cout<<"Executable not found for '"<<argv[1]<<"'\n";
    }
    else demo_default();
    return 0;
}
