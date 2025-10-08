
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;
using namespace chrono_literals;

int main(int argc, char*argv[])
{
    int rc=0, pause_ms=0;
    size_t avalanche=0;
    for (int a=1; a<argc; ++a)
    {
        const string arg=argv[a];
        if (arg=="-a" && a<argc-1) avalanche=atoi(argv[++a]);
        else if (arg=="-r" && a<argc-1) rc=atoi(argv[++a]);
        else if (arg=="-p" && a<argc-1)
        {
            auto p=atoi(argv[++a]);
            if (p>0 && p<=10000) pause_ms=p;
        }
    }
    if (avalanche>1)
    {
#ifdef _WIN32
        _setmode(_fileno(stdout), O_BINARY); // Make stdout binary
#endif
        string A(avalanche-1, ' ');
        for (auto j=0; j<A.size(); ++j)
        {
            const auto k=j%27;
            A[j]=k==26?'\n':(char)('A'+k);
        }
        puts(A.c_str());
    }
    else for (int j=0; j<10; ++j)
    {
        if (j==5) fprintf(stderr, "%d: something unexpected happened.\n", j);
        else      fprintf(stdout, "%d: ok\n", j);
        if (j<10 && pause_ms>0) this_thread::sleep_for(pause_ms*1ms);
    }
    return rc;
}
