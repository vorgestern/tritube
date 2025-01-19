
#include <cstdio>
#include <cstring>
#include <string>

using namespace std;

const auto nix=string::npos;

int main()
{
    setvbuf(stdin, 0, 0, _IONBF);
    printf("%s\n", "This is your echo-server with benefits, listening on stdin.");
    while (true)
    {
        char pad[1000];
        char*s=fgets(pad, sizeof(pad)-1, stdin);
        if (s==nullptr) break;
        char*e=strchr(s, '\n');
        if (e!=nullptr) *e=0;
        string inp(pad);
        if (inp=="exit") break;
        else if (inp.starts_with("I am "))
        {
            string rest(inp, 5);
            size_t start=nix, end=nix;
            for (auto j=0u; j<rest.size(); ++j)
            {
                const auto c=rest[j];
                const auto a=isalpha(c);
                if (start==nix && a) start=j;
                else if (start==nix) continue;
                else if (!a){ end=j; break; }
            }
            const string name(rest, start, end-start);
            printf("Hello %s!\n", name.c_str());
        }
        else printf(">> '%s'\n", s);
    }
    return 0;
}
