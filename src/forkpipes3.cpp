
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "forkpipes.h"

using namespace std;

forkpipes3::forkpipes3()
{
    if (pipe(pipe_in)<0)
    {
        perror("allocating pipe for child input redirect");
        // return -1;
    }
    if (pipe(pipe_out)<0)
    {
        close(pipe_in[PIPE_READ]);
        close(pipe_in[PIPE_WRITE]);
        perror("allocating pipe for child output redirect");
        // return -1;
    }
    if (pipe(pipe_err)<0)
    {
        close(pipe_in[PIPE_READ]);
        close(pipe_in[PIPE_WRITE]);
        close(pipe_out[PIPE_READ]);
        close(pipe_out[PIPE_WRITE]);
        perror("allocating pipe for child error output redirect");
        // return -1;
    }
    childpid=fork();
    if (childpid==0)
    {
        // child continues here
        if (dup2(pipe_in[PIPE_READ],   STDIN_FILENO)==-1
         || dup2(pipe_out[PIPE_WRITE], STDOUT_FILENO)==-1
         || dup2(pipe_err[PIPE_WRITE], STDERR_FILENO)==-1
        )
        {
            exit(errno);
        }
        // all these are for use by parent only
        close(pipe_in[PIPE_READ]);
        close(pipe_in[PIPE_WRITE]);
        close(pipe_out[PIPE_READ]);
        close(pipe_out[PIPE_WRITE]);
        close(pipe_err[PIPE_READ]);
        close(pipe_err[PIPE_WRITE]);
    }
    else if (childpid>0)
    {
        // parent continues here
        // close unused file descriptors, these are for child only
        close(pipe_in[PIPE_READ]);
        close(pipe_out[PIPE_WRITE]);
        close(pipe_err[PIPE_WRITE]);
    }
    else
    {
        // failed to create child
        close(pipe_in[PIPE_READ]);
        close(pipe_in[PIPE_WRITE]);
        close(pipe_out[PIPE_READ]);
        close(pipe_out[PIPE_WRITE]);
        close(pipe_err[PIPE_READ]);
        close(pipe_err[PIPE_WRITE]);
    }
}

forkpipes3::~forkpipes3()
{
    // done with these in this example program, you would normally keep these
    // open of course as long as you want to talk to the child
    close(pipe_in[PIPE_WRITE]);
    close(pipe_out[PIPE_READ]);
    close(pipe_err[PIPE_READ]);
}

pair<char,int>forkpipes3::readchar()
{
    const int kstdin=parent_read(*this);
    const int kerrin=parent_readerr(*this);
    fd_set toread;
    if (closed==3) return {0,-4};
    int maxfd=0;
    FD_ZERO(&toread);
    if ((closed&1)==0){ FD_SET(kstdin, &toread); if (maxfd<kstdin) maxfd=kstdin; }
    if ((closed&2)==0){ FD_SET(kerrin, &toread); if (maxfd<kerrin) maxfd=kerrin; }
    struct timeval*const timeout_inf=nullptr;
    const int rc=select(maxfd+1, &toread, nullptr, nullptr, timeout_inf);
    if (rc==-1 && errno!=EINTR) return {0,-3};
    if (rc==-1) return {0,-1};
    if (rc==0) return {0,-2};
    if (rc>0)
    {
        char c;
        if (FD_ISSET(kstdin, &toread))
        {
            ssize_t nr=read(kstdin, &c, 1);
            if (nr==1) return {c,1}; // printf(c!='\n'?"(%c)":"%c", c);
            else if (nr==0) closed|=1;
        }
        if (FD_ISSET(kerrin, &toread))
        {
            ssize_t nr=read(kerrin, &c, 1);
            if (nr==1) return {c,2}; // printf(c!='\n'?"<%c>":"%c", c);
            else if (nr==0) closed|=2;
        }
        return {0,-2};
    }
    else return {0,-4};
}
