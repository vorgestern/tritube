
#include <iostream>
#include <tritube/tritube.h>

using namespace std;
using namespace tritube;

static size_t numlines(const string&X)
{
    if (X.empty()) return 0;
    size_t num=0;
    size_t p=0;
    while (p!=X.npos)
    {
        auto neu=X.find_first_of('\n', p);
        if (neu!=X.npos)
        {
            ++num;
            p=neu+1;
        }
        else if (p<X.size())
        {
            ++num;
            p=X.npos;
        }
        else p=X.npos;
    }
    return num;
}

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
    enum {print, out, out_raw, roe, roe_raw, roev, roev_raw, linewise, linewise_raw} usecase=out;
    xfind search=xfpath;
    bool output_raw=false;
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
        else if (arg=="--raw") output_raw=true;
    }
    if (output_raw) switch (usecase)
    {
        case out: usecase=out_raw; break;
        case roe: usecase=roe_raw; break;
        case roev: usecase=roev_raw; break;
        case linewise: usecase=linewise_raw; break;
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
        const auto pp=p.value();
        switch (usecase)
        {
            case print:
            {
                auto f=pp.string();
                if (auto wo=f.find_first_of(' ', 0); wo!=string::npos) cout<<'"'<<f<<'"'<<args<<"\n";
                else                                                   cout<<f<<args<<"\n";
                exit(0);
                break;
            }
            case out:
            {
                auto Text=piper_o(p.value(), args);
                if (Text.size()>0) cout<<"======================\n"<<Text<<"\n======================\n";
                else cout<<"No output for '"<<p.value().string()<<" "<<args<<"'\n";
                exit(Text.size()>0?0:1);
                break;
            }
            case out_raw:
            {
                auto Text=piper_o(p.value(), args);
                if (Text.size()>0) cout<<Text;
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
            case roe_raw:
            {
                auto [rc, out, err]=piper_roe(p.value(), args);
                cout<<rc<<" "<<numlines(out)<<" "<<numlines(err)<<"\n";
                cout<<out;
                if (out.back()!='\n') cout<<"\n";
                cout<<err;
                if (err.back()!='\n') cout<<"\n";
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
            case roev_raw:
            {
                auto [rc, Out, Err]=piper_roev(p.value(), args);
                cout<<rc<<" "<<Out.size()<<" "<<Err.size()<<"\n";
                for (auto&k: Out) cout<<k<<"\n";
                for (auto&k: Err) cout<<k<<"\n";
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
            case linewise_raw:
            {
                vector<string> Out, Err;
                auto rc=piper_linewise(p.value(), args,
                    [&Out](const string&X){ Out.push_back("out: "+X); },
                    [&Err](const string&X){ Err.push_back("err: "+X); });
                cout<<rc<<" "<<Out.size()<<" "<<Err.size()<<"\n";
                for (auto&k: Out) cout<<k<<"\n";
                for (auto&k: Err) cout<<k<<"\n";
                exit(rc==0?0:1);
            }
        }
    }
    else
    {
        cerr<<"Executable not found for '"<<appl<<"'\n";
        exit(1);
    }
}
