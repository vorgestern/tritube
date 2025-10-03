
#include <iostream>
#include <tritube/tritube.h>

using namespace std;
using namespace tritube;

static ostream&operator<<(ostream&out, const vector<string>&args)
{
    unsigned j=0;
    for (auto&k: args) out<<(j++>0?" ":"")<<k;
    return out;
}

void demo_default()
{
    if (auto p=applpath(xfpath, "git.exe"); p.has_value())
    {
        auto Text=piper_o(p.value(), {"--version"});
        if (Text.size()>0) cout<<"======================\n"<<Text<<"\n======================\n";
        else cout<<"No output for 'git.exe --version\n";
    }
    else cout<<"Executable not found for 'git.exe'\n";
}

int main(int argc, char*argv[])
{
    enum {print, out, roe, roev, linewise} usecase=out;
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
            else if (s=="roev") usecase=roev;
            else if (s=="linewise") usecase=linewise;
            else
            {
                cerr<<"Usecase '"<<s<<"' unknown (known: print, stdout, roe, roev, linewise).\n";
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
    string appl="git.exe";
    vector<string> args={"--version"};
    if (a<argc)
    {
        appl=argv[a++];
        args.clear();
        for (auto j=a; j<argc; ++j) args.push_back(argv[j]);
    }
    if (auto p=applpath(search, appl); p.has_value())
    {
        switch (usecase)
        {
            case print: cout<<"'"<<p.value()<<" "<<args<<"'\n"; exit(p.has_value()?0:1); break;
            case out:
            {
                auto Text=piper_o(p.value(), args);
                if (Text.size()>0) cout<<"======================\n"<<Text<<"\n======================\n";
                else cout<<"No output for '"<<p.value().string()<<" "<<args<<"'\n";
                exit(Text.size()>0?0:1);
                break;
            }
            case roe:
            {
                auto [rc, out, err]=piper_roe(p.value(), args);
                cout<<  "== return code =================================\n"<<rc
                    <<"\n== stdout ======================================\n"<<out
                    <<"\n== stderr ======================================\n"<<err
                    <<"\n================================================\n";
                exit(rc==0?0:1);
            }
            case roev:
            {
                auto [rc, Out, Err]=piper_roev(p.value(), args);
                cout<<  "== return code =================================\n"<<rc
                    <<"\n== stdout ======================================\n";
                for (auto&k: Out) cout<<"== "<<k<<"\n";
                cout<<"== stderr ======================================\n";
                for (auto&k: Err) cout<<"== "<<k<<"\n";
                cout<<"================================================\n";
                exit(rc==0?0:1);
            }
            case linewise:
            {
                auto rc=piper_linewise(p.value(), args,
                    [](const string&X){ cout<<">> "<<X<<" <<\n"; },
                    [](const string&X){ cout<<"?? "<<X<<" ??\n"; });
                cout<<  "== return code =================================\n"<<rc
                    <<"\n================================================\n";
                exit(rc==0?0:1);
            }
        }
    }
    else
    {
        cout<<"Executable not found for '"<<appl<<"'\n";
        exit(1);
    }
}
