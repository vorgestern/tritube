
#include <iostream>
#include <tritube/tritube.h>

using namespace std;
using namespace tritube;

void demo1()
{
    auto Text1=exec_sync<string, xfrelative>("git.exe", "--version");
    cout<<"demo1: "<<Text1<<"\n";
}

void demo2()
{
    auto Text2=exec_sync<string, xfpath>("git.exe", "--version");
    cout<<"demo2: "<<Text2<<"\n";
}

int main()
{
    // demo1();
    demo2();
    return 0;
}
