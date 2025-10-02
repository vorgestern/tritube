
#include <string>
#include <thread>
#include <chrono>

using namespace std;
using namespace chrono_literals;

int main(int argc, char*argv[])
{
    int rc=0, pause_ms=0;
    for (int a=1; a<argc; ++a)
    {
        const string arg=argv[a];
        if (arg=="-r" && a<argc-1) rc=atoi(argv[++a]);
        else if (arg=="-p" && a<argc-1)
        {
            auto p=atoi(argv[++a]);
            if (p>0 && p<=10000) pause_ms=p;
        }
    }
    for (int j=0; j<10; ++j)
    {
        if (j==5) fprintf(stderr, "%d: something unexpected happened.\n", j);
        else      fprintf(stdout, "%d: ok\n", j);
        if (j<10 && pause_ms>0) this_thread::sleep_for(pause_ms*1ms);
    }
    return rc;
}
