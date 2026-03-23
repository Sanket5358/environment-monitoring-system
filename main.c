#include <stdio.h>
#include <stdlib.h>   // ✅ FIX ADDED
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

pid_t p1, p2, p3, p4;

void cleanup(int sig)
{
    printf("\n[Supervisor] Shutting down system...\n");

    kill(p1, SIGINT);
    kill(p2, SIGINT);
    kill(p3, SIGINT);
    kill(p4, SIGINT);

    exit(0);
}

int main()
{
    signal(SIGINT, cleanup);

    printf("[Supervisor] Starting system...\n");

    int pipefd[2];
    pipe(pipefd);

    if((p1 = fork()) == 0)
    {
        close(pipefd[0]);
        close(pipefd[1]);
        execl("./sensor", "sensor", NULL);
        perror("execl sensor failed");   // ✅ FIX
        exit(1);
    }

    sleep(1);

    if((p2 = fork()) == 0)
    {
        close(pipefd[0]);
        close(pipefd[1]);
        execl("./processor", "processor", NULL);
        perror("execl processor failed"); // ✅ FIX
        exit(1);
    }

    sleep(1);

    if((p3 = fork()) == 0)
    {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        execl("./logger", "logger", NULL);
        perror("execl logger failed");   // ✅ FIX
        exit(1);
    }

    if((p4 = fork()) == 0)
    {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        execl("./display", "display", NULL);
        perror("execl display failed");  // ✅ FIX
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    while(wait(NULL) > 0);

    return 0;
}
