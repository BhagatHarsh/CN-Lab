#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

int main()
{
    int pid1, pid2;
    pid1 = fork();

    if (pid1 == 0)
    {
        printf("Child 1 process\n");
        printf("Child 1 process id: %d\n", getpid());
        printf("Parent of child 1 process id: %d\n", getppid());

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        printf("Date Today is: %d-%02d-%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    }
    else
    {
        wait(NULL);
        printf("Parent process\n");
        printf("Parent process id: %d\n", getpid());
        pid2 = fork();
        if (pid2 == 0)
        {
            printf("Child 2 process 2\n");
            printf("Child 2 process id: %d\n", getpid());
            printf("Parent of child 2 process id: %d\n", getppid());

            system("ls -al");
        }
    }
    return 0;
}