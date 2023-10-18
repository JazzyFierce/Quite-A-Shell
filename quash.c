#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

char buffer[1024];
char *args[256];
int numArgs = 0;

void handlingCommands(char **commandstr)
{

    int pid = fork();

    if (pid == 0)
    {
        if (strcmp("pwd", commandstr[0]) == 0)
        {
            char cwd[1024];
            getcwd(cwd, 1024);
            printf("%s\n", cwd);
        }

        if (strcmp("ls", commandstr[0]) == 0)
        {
            execvp("/bin/ls", args);
        }

        if (strcmp("echo", commandstr[0]) == 0)
        {
            execvp("/bin/echo", args);
        }

        exit(0);
    }
    else
    {
        wait(NULL);
    }
}

void tokenize(char *command)
{
    memset(args, 0, sizeof(args));
    char *token;
    token = strtok(command, " \n\"");

    while (token != NULL)
    {
        args[numArgs] = token;
        token = strtok(NULL, " \n\"");
        numArgs++;
    }
}

int main()
{
    while (1)
    {
        printf("[Quash]$ ");
        fgets(buffer, 1024, stdin);
        tokenize(buffer);

        if (strcmp("exit", args[0]) == 0 || strcmp("quit", args[0]) == 0)
        {
            exit(0);
        }
        else
        {
            handlingCommands(args);
            numArgs = 0;
            memset(buffer, 0, strlen(buffer));
        }
    }

    return 0;
}
