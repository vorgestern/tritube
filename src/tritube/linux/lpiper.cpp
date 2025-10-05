
#include <tritube/tritube.h>
#include <thread>
#include <chrono>
#include <numeric>
#include <cstring>
#include <sys/wait.h>

#include "forkpipes.h"

using namespace std;
using namespace chrono_literals;
using namespace tritube;
using fspath=filesystem::path;

static int exec_neu(const fspath&appl, const vector<string>&args)
{
    vector<char*>argv(args.size()+2);
    auto j=0u;
    argv[j++]=strdup(appl.string().c_str());
    for (auto&k: args) argv[j++]=strdup(k.c_str());
    argv[j++]=nullptr;
    return execvp(argv[0], &argv[0]);
}

static int process_inputs(forkpipes3&PP, vector<string>&Out, vector<string>&Err)
{
    Out.clear();
    Err.clear();
    string OutLine, ErrLine;
    while (!isclosed(PP))
    {
        const auto [c,fd]=PP.readchar();
        switch (fd)
        {
            case 1: // stdin
            {
                if (c=='\n')
                {
                    Out.push_back(OutLine);
                    OutLine.clear();
                }
                else if (c!='\r') OutLine.push_back(c);
                break;
            }
            case 2:
            {
                if (c=='\n')
                {
                    Err.push_back(ErrLine);
                    ErrLine.clear();
                }
                else if (c!='\r') ErrLine.push_back(c);
                break;
            }
        }
    }
    if (const int pid=childpid(PP); pid>0)
    {
        int stat=0;
        waitpid(pid, &stat, 0);
        return WEXITSTATUS(stat);
    }
    else return -1;
}

static int process_inputs(forkpipes3&PP, function<void(const string&)>process_stdout, function<void(const string&)>process_stderr)
{
    string OutLine, ErrLine;
    while (!isclosed(PP))
    {
        const auto [c,fd]=PP.readchar();
        switch (fd)
        {
            case 1: // stdin
            {
                if (c=='\n')
                {
				    process_stdout(OutLine);
                    OutLine.clear();
                }
                else if (c!='\r') OutLine.push_back(c);
                break;
            }
            case 2:
            {
                if (c=='\n')
                {
				    process_stderr(ErrLine);
                    ErrLine.clear();
                }
                else if (c!='\r') ErrLine.push_back(c);
                break;
            }
        }
    }
    if (const int pid=childpid(PP); pid>0)
    {
        int stat=0;
        waitpid(pid, &stat, 0);
        return WEXITSTATUS(stat);
    }
    else return -1;
}

pair<bool, size_t>write_initial_input(int fd, string_view initial_input)
{
    bool receiver_closed=false;
    size_t numwritten=0;
    while (numwritten<initial_input.size() && !receiver_closed)
    {
        const auto nw=write(fd, initial_input.data()+numwritten, initial_input.size());
        if (nw<0)
        {
            const auto e=errno;
            switch (e)
            {
                case EAGAIN:
                {
                    this_thread::sleep_for(50ms);
                    break;
                }
                // EAGAIN or EWOULDBLOCK ... (sockets)
                // EBADF fd is not a valid file descriptor or is not open for writing.
                // EDESTADDRREQ fd refers to a datagram socket for which a peer address has not been set using connect(2).
                // EDQUOT The user's quota of disk blocks on the filesystem containing the file referred to by fd has been exhausted.
                // EFAULT buf is outside your accessible address space.
                // EFBIG An attempt was made to write a file that exceeds the implementation-defined maximum file size or the process's file size limit, or to write at a position past the maximum allowed offset.
                // EINTR The call was interrupted by a signal before any data was written; see signal(7).
                // EINVAL fd is attached to an object which is unsuitable for writing; or the file was opened with the O_DIRECT flag ...
                // EIO    A low-level I/O error occurred while modifying the inode ...
                // ENOSPC The device containing the file referred to by fd has no room for the data.
                // EPERM  The operation was prevented by a file seal; see fcntl(2).
                case EPIPE:
                {
                    // fd is connected to a pipe or socket whose reading end is closed.  When this happens the writing
                    // process will also receive a SIGPIPE signal. (Thus, the write return value is seen only if the program
                    // catches, blocks or ignores this signal.)
                    receiver_closed=true;
                    break;
                }
            }
        }
        else numwritten+=(size_t)nw;
    }
    return {receiver_closed, numwritten};
}

string tritube::piper_o(fspath&fullpath, const vector<string>&args, string_view initial_input)
{
    forkpipes3 PP;
    if (ischild(PP))
    {
        exit(exec_neu(fullpath, args));
    }
    else if (isparent(PP))
    {
        [[maybe_unused]] auto [receiver_closed,numwritten]=write_initial_input(parent_write(PP), initial_input);
        close(parent_write(PP));
        vector<string> Out, Err;
        if (const int rc=process_inputs(PP, Out, Err); rc==0)
		{
			auto total=accumulate(Out.begin(), Out.end(), 0, [](size_t acc, string&X){ return acc+X.size(); });
			string Result;
			Result.reserve(total);
			for (auto&j: Out) Result.append(j+"\n");
			return Result;
		}
		else return {};
    }
    else return {};
}

rc_out_err tritube::piper_roe(fspath&fullpath, const vector<string>&args, string_view initial_input)
{
    forkpipes3 PP;
    if (ischild(PP))
    {
        exit(exec_neu(fullpath, args));
    }
    else if (isparent(PP))
    {
        [[maybe_unused]] auto [receiver_closed,numwritten]=write_initial_input(parent_write(PP), initial_input);
        close(parent_write(PP));
        vector<string> OutLines, ErrLines;
        [[maybe_unused]] const int rc=process_inputs(PP, OutLines, ErrLines);
        string Out, Err;
        auto totalout=accumulate(OutLines.begin(), OutLines.end(), 0, [](size_t acc, string&X){ return acc+X.size(); }),
             totalerr=accumulate(ErrLines.begin(), ErrLines.end(), 0, [](size_t acc, string&X){ return acc+X.size(); });
        Out.reserve(totalout); for (auto&k: OutLines) Out.append(k+"\n");
        Err.reserve(totalerr); for (auto&k: ErrLines) Err.append(k+"\n");
        return {rc, Out, Err};
    }
    return {-1, {}, {}};
}

rc_Out_Err tritube::piper_roev(fspath&fullpath, const vector<string>&args, string_view initial_input)
{
    forkpipes3 PP;
    if (ischild(PP))
    {
        exit(exec_neu(fullpath, args));
    }
    else if (isparent(PP))
    {
        [[maybe_unused]] auto [receiver_closed,numwritten]=write_initial_input(parent_write(PP), initial_input);
        close(parent_write(PP));
        vector<string> Out, Err;
        [[maybe_unused]] const int rc=process_inputs(PP, Out, Err);
        return {rc, Out, Err};
    }
    else return {-1, {}, {}};
}

int tritube::piper_linewise(fspath&fullpath, const vector<string>&args, function<void(const string&)>process_stdout, function<void(const string&)>process_stderr, string_view initial_input)
{
    forkpipes3 PP;
    if (ischild(PP))
    {
        exit(exec_neu(fullpath, args));
    }
    else if (isparent(PP))
    {
        [[maybe_unused]] auto [receiver_closed,numwritten]=write_initial_input(parent_write(PP), initial_input);
        close(parent_write(PP));
        return process_inputs(PP, process_stdout, process_stderr);
    }
    else return -1;
}
