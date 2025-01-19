
#include <cstdio>
#include <cstring>

int main()
{
    setvbuf(stdin, 0, 0, _IONBF);
    printf("%s\n", "Startup: This is your echo-server with benefits, listening on stdin.");
    while (true)
    {
        char pad[1000];
        char*s=fgets(pad, sizeof(pad)-1, stdin);
        if (s==nullptr) break;
        char*e=strchr(s, '\n');
        if (e!=nullptr) *e=0;
        printf(">> '%s'\n", s);
        if (0==strcmp(s, "exit")) break;
    }
    return 0;
}
