
struct prochelper
{
    HANDLE process;
    HANDLE write_to_stdin;
    HANDLE     read_from_stdout, read_from_stderr;
    OVERLAPPED olout,            olerr;
};

// return {op,errorcode}:
//     op          the operation that failed
//     errorcode   the result of GetLastError for the failed operation
// Operation:
// 0        Ok
// 1,2,3    Failed to create pipe for stdin,stdout,stderr or child process.
// 11,12,13 Failed to make pipe handles heritable.
// 4        Failed to create child process.
std::pair<int,int> startpiped(prochelper&, const std::string&exec, const vector<string>&args);

struct inputchannel_async
{
    HANDLE read_from_child;
    OVERLAPPED ol;
    char*buffer;
    DWORD cap;

    inputchannel_async(HANDLE input, const OVERLAPPED&ol1, char*buffer1, DWORD cap1): read_from_child(input), ol(ol1), buffer(buffer1), cap(cap1){}

    bool closed {false};
    DWORD nr {0};
    void read();
    void closehandles();
};

int entertain(HANDLE hprocess, inputchannel_async&out, std::function<void()>handle_out, inputchannel_async&err, std::function<void()>handle_err);
