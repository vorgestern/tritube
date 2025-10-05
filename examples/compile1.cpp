
#include <tritube/tritube.h>
#include <iostream>

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

static int task_compile(string_view tocompile)
{
    auto exec=tritube::applpath(tritube::xfpath, "g++");
    if (exec.has_value())
    {
        cout<<"ttdemo "<<exec.value()<<"\n";
        cout<<"***** Start  *****\n";
        const auto [rc,out,err]=tritube::piper_roe(exec.value(), {"-Werror", "-Wall", "-o", "hoppla.o", "-x", "c++", "-"}, tocompile);
        cout<<err;
        cout<<"***** Finish *****\n";
        return 0;
    }
    else return -1;
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
    switch (task)
    {
        case ttdemo: return task_compile(A);
        case ttcompile: return task_compile(tocompile);
        default: break;
    }
    return 0;
}
