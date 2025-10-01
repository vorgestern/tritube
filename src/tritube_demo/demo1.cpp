
#include <iostream>
#include <tritube/tritube.h>

using namespace std;
using namespace tritube;

void demo_default()
{
    if (auto p=applpath(xfpath, "git.exe"); p.has_value())
    {
        auto Text=exec_simple<string>(p.value(), "--version");
        if (Text.size()>0) cout<<"======================\n"<<Text<<"\n======================\n";
        else cout<<"No output for 'git.exe --version\n";
    }
    else cout<<"Executable not found for 'git.exe'\n";
}

int main(int argc, char*argv[])
{
    enum {print, out, roe} usecase=out;
    xfind search=xfdirect;
    auto a=1;
    for (a=1; a<argc; ++a)
    {
        const string arg=argv[a];
        if (arg=="--"){ ++a; break; }
        else if (arg=="--usecase" && a<argc-1)
        {
            const string s=argv[++a];
            if (s=="print") usecase=print;
            else if (s=="stdout") usecase=out;
            else if (s=="roe") usecase=roe;
            else
            {
                cerr<<"Usecase '"<<s<<"' unknown (known: stdout, roe).\n";
                exit(1);
            }
        }
        else if (arg=="--search" && a<argc-1)
        {
            const string s=argv[++a];
            if (s=="direct") search=xfdirect;
            else if (s=="path") search=xfpath;
            else
            {
                cerr<<"Search method '"<<s<<"' unknown (known: direct, path).\n";
                exit(1);
            }
        }
    }
    string appl="git.exe", args="--version";
    if (a<argc)
    {
        appl=argv[a++];
        args.clear();
        for (auto j=a; j<argc; ++j) args.append(j>a?" "s+argv[j]:argv[j]);
    }
    // cout<<"usecase "<<(int)usecase<<"\nsearch "<<(int)search<<"\nappl "<<appl<<"\nargs '"<<args<<"'\n";
    if (auto p=applpath(search, appl); p.has_value())
    {
        switch (usecase)
        {
            case print: cout<<"'"<<p.value()<<" "<<args<<"'\n"; break;
            case out:
            {
                auto Text=exec_simple<string>(p.value(), args);
                if (Text.size()>0) cout<<"======================\n"<<Text<<"\n======================\n";
                else cout<<"No output for '"<<p.value().string()<<" "<<args<<"\n";
                break;
            }
            case roe:
            {
                auto [rc, out, err]=exec_simple<rc_out_err>(p.value(), args);
                cout<<  "== return code =================================\n"<<rc
                    <<"\n== stdout ======================================\n"<<out
                    <<"\n== stderr ======================================\n"<<err
                    <<"\n================================================\n";
                break;
            }
        }
    }
    else cout<<"Executable not found for '"<<appl<<"'\n";
    return 0;
}
