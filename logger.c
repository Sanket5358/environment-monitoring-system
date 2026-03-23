#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

int fd;

void cleanup(int sig)
{
    printf("\n[Logger] Cleaning up...\n");
    close(fd);
    exit(0);
}

int main()
{
    signal(SIGINT, cleanup);

    fd = open("environment_log.txt",
              O_CREAT | O_WRONLY | O_APPEND, 0666);

    if(fd < 0)
    {
        perror("File open failed");
        exit(1);
    }

    printf("[Logger] Started...\n");

    char buffer[512];

    while(1)
    {
        int n = read(STDIN_FILENO, buffer, sizeof(buffer));

        if(n > 0)
        {
            /* 🔥 Move file pointer to end using lseek */
            lseek(fd, 0, SEEK_END);

            /* Write data to file */
            write(fd, buffer, n);
        }
        else
        {
            sleep(1);
        }
    }
}
