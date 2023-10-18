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
        if (strcmp("pwd", args[0]) == 0)
        {
            char cwd[1024];
            getcwd(cwd, 1024);
            printf("%s\n", cwd);
        }

        if (strcmp("ls", args[0]) == 0)
        {
            execlp("/bin/ls", *(args + 1));
        }

        if (strcmp("echo", args[0]) == 0)
        {
            // execl("/bin/echo", *args);
            int i = 1;
            while (i < numArgs)
            {
                if (strcmp("\"", args[i]) == 0)
                {
                    i++;
                }
                printf("%s ", args[i]);
                i++;
            }
            printf("\n");
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
        // printf("%s\n", token);
        args[numArgs] = token;
        token = strtok(NULL, " \n\"");
        numArgs++;
        printf("%s\n", args[numArgs - 1]);
    }
}

int main()
{
    while (1)
    {
        printf("[Quash]$ ");
        fgets(buffer, 1024, stdin);
        tokenize(buffer);

        if (strcmp("exit", args[0]) == 0)
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
