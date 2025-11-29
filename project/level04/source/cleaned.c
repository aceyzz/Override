#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

int ptrace(int request, ...);

int main(int argc, char **argv)
{
    pid_t pid = fork();
    int status = 0;
    char buffer[128];

    memset(buffer, 0, sizeof(buffer));

    if (pid == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGHUP, 0, 0, 0);
        ptrace(0);
        puts("Give me some shellcode, k");
        gets(buffer);
        return 0;
    }

    wait(&status);

    while (status % 128 != 0)
    {
        char v = (char)status % 128 + 1;

        if (v >= 2 && v >= 0)
            break;

        if (ptrace(3) == 11)
        {
            puts("no exec() for you");
            kill(pid, SIGKILL);
            return 0;
        }

        wait(&status);
    }

    puts("child is exiting...");
    return 0;
}