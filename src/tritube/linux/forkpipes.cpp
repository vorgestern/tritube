
#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "forkpipes.h"

forkpipes::forkpipes()
{
    if (pipe(pipe_in)<0)
    {
        perror("Allocating pipe for child input redirect");
        // return -1;
    }
    if (pipe(pipe_out)<0)
    {
        close(pipe_in[PIPE_READ]);
        close(pipe_in[PIPE_WRITE]);
        perror("Allocating pipe for child output redirect");
        // return -1;
    }
    childpid=fork();
    if (childpid==0)
    {
        // child continues here
        if (dup2(pipe_in[PIPE_READ], STDIN_FILENO)==-1 || dup2(pipe_out[PIPE_WRITE], STDOUT_FILENO)==-1
          // || dup2(pipe_out[PIPE_WRITE], STDERR_FILENO)==-1
        )
        {
            exit(errno);
        }
        // all these are for use by parent only
        close(pipe_in[PIPE_READ]);
        close(pipe_in[PIPE_WRITE]);
        close(pipe_out[PIPE_READ]);
        close(pipe_out[PIPE_WRITE]);
    }
    else if (childpid>0)
    {
        // parent continues here
        // close unused file descriptors, these are for child only
        close(pipe_in[PIPE_READ]);
        close(pipe_out[PIPE_WRITE]);
    }
    else
    {
        // failed to create child
        close(pipe_in[PIPE_READ]);
        close(pipe_in[PIPE_WRITE]);
        close(pipe_out[PIPE_READ]);
        close(pipe_out[PIPE_WRITE]);
    }
}

forkpipes::~forkpipes()
{
    // done with these in this example program, you would normally keep these
    // open of course as long as you want to talk to the child
    close(pipe_in[PIPE_WRITE]);
    close(pipe_out[PIPE_READ]);
}
