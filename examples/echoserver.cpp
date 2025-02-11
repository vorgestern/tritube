
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

using namespace std;

const auto nix=string::npos;

// _IOFBF    Full buffering: On output, data is written once the buffer is full (or flushed). On Input, the buffer is filled when an input operation is requested and the buffer is empty.
// _IOLBF    Line buffering: On output, data is written when a newline character is inserted into the stream or when the buffer is full (or flushed), whatever happens first. On Input, the buffer is filled up to the next newline character when an input operation is requested and the buffer is empty.
// _IONBF    No buffering: No buffer is used. Each I/O operation is written as soon as possible. In this case, the buffer and size parameters are ignored.

int main1()
{
    while (true)
    {
        char pad[1000];
        char*s=fgets(pad, sizeof(pad)-1, stdin);
        if (s==nullptr) break;
        char*e=strchr(s, '\n');
        if (e!=nullptr) *e=0;
        string inp(pad);
        if (inp=="exit") break;                            // Special treatment for 'exit'
        else if (inp.starts_with("I am "))                 // Special treatment for 'I am <name>'
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
        else printf(">> '%s'\n", s);                       // Otherwise echo.
    }
    return 0;
}

static bool react(string&X)
{
    if (X=="exit") return printf("You could at least say good bye.\n"),false;
    else if (X.starts_with("I am "))
    {
        size_t start=nix, end=nix;
        for (auto j=5u; j<X.size(); ++j)
        {
            const auto a=isalpha(X[j]);
            if (start==nix && a) start=j;
            else if (start==nix) continue;
            else if (!a){ end=j; break; }
        }
        const string name(X, start, end-start);
        printf("Hello, %s!\n", name.c_str());
    }
    else if (X.starts_with("Don't be "))
    {
        size_t start=nix, end=nix;
        for (auto j=9u; j<X.size(); ++j)
        {
            const auto a=isalpha(X[j]);
            if (start==nix && a) start=j;
            else if (start==nix) continue;
            else if (!a){ end=j; break; }
        }
        this_thread::sleep_for(2000ms);
        const string what(X, start, end-start);
        printf("'... %s'? That's easy for you to say!\n", what.c_str());
    }
    else printf("Generic reaction to '%s'\n", X.c_str());
    return true;
}

int main2()
{
    string inputs;
    while (true)
    {
        fd_set toread;
        FD_ZERO(&toread);
        FD_SET(0, &toread);
        struct timeval timeout_s={7, 0};
        const int rc=select(1, &toread, nullptr, nullptr, &timeout_s);
        if (rc<0)
        {
            // Error management here
            fprintf(stderr, "Error (select returns %d, errno is %d)", rc, errno);
        }
        else if (rc==0)
        {
            // Timeout has occurred.
            static auto index=0;
            const vector<string> prompts={
                "Don't you have anything to say?",
                "Come on, say something!",
                "If you have nothing to say, you can at least say something nice.",
                "You could at least say hello?"
            };
            const auto k=index==2?stderr:stdout;
            fprintf(k, "%s\n", prompts[index].c_str());
            index=(index+1)%prompts.size();
        }
        else if (rc==1)
        {
            // This can only mean something is available from stdin.
            char pad[1000];
            char*s=fgets(pad, sizeof(pad)-1, stdin);
            if (s==nullptr) break;
            char*e=strchr(s, '\n');
            if (e!=nullptr && e>s)
            {
                for (auto n=0u; n<e-s; ++n) inputs.push_back(s[n]);
                const auto a=react(inputs);
                inputs.clear();
                if (!a) break;
            }
        }
        else
        {
            fprintf(stderr, "%s (select returns %d)\n", "Now this is unexpected!", rc);
        }
    }
    return 0;
}

int main()
{
    // Unbuffered or linebuffered output is required here.
    // Buffering must be setup before use or it will be upset.
    setvbuf(stdin, 0, _IONBF, 0);
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);
    printf("%s\n", "This is your echo-server with benefits, listening on stdin.\n");
    return main2();
}
