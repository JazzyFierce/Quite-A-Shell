#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

char buffer[1024];
char *args[256];
int numArgs = 0;

void replace_with_env(char **arg);

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
 		    int count =0;           
            for (int i = 1; commandstr[i]; i++) {
                if (commandstr[i][0] == '$' && commandstr[i][1] != ' ') {
                    printf("%s ", getenv(commandstr[i] + 1));
                } else {
                    printf("%s ", commandstr[i]);
                }
            }
            printf("\n");
        }
        
        if (strcmp("cd", commandstr[0]) ==0)
        {
            if(  chdir(args[1]) != 0) {
            	printf("%s is not a directory\n", args[1]);
            };
        }
        
        if (strcmp("du",  commandstr[0]) ==0 ){
	        for (int i = 0; commandstr[i]; i++) {
                replace_with_env(&commandstr[i]);
            }
            execvp(commandstr[0], commandstr);
        }

        exit(0);
    }
    else
    {
        wait(NULL);
    }
}

void replace_with_env(char **arg) {
    if ((*arg)[0] == '$') {
        char *env_name_ending = strpbrk(*arg, "/ \n\t"); 
        char *rem = NULL;
        
        if (env_name_ending) {
            rem = strdup(env_name_ending);  
            *env_name_ending = '\0';  
        }

        char *env_value = getenv(*arg + 1);
        if (env_value) {
            char *new_arg = (char *) malloc(strlen(env_value) + (rem ? strlen(rem) : 0) + 1);
            strcpy(new_arg, env_value);
            if (rem) {
                strcat(new_arg, rem);
                free(rem);
            }
            *arg = new_arg;
        } else {
            *arg = rem ? rem : ""; 
        }
    	
    }
}

// TODO: add case for initializing arbitrary variable without assignment
int export(char** commandstr) {
    if (strchr(commandstr[1], '$') == NULL) {
        // assumes valid input
        // eg export testvar=testval
        char* e = strchr(commandstr[1], '=');
        int index = (int)(e - commandstr[1]);
        
        char* temp1 = malloc(index+1);
        strncpy(temp1, commandstr[1], index);
        temp1[index] = 0;

        setenv(temp1, commandstr[1]+index+1, 1);
        
        free(temp1);
    } else { 
        // assumes valid input
        // eg export PATH=$HOME
        char* e = strchr(commandstr[1], '=');
        int index = (int)(e - commandstr[1]);
        
        char* temp1 = malloc(index+1);
        strncpy(temp1, commandstr[1], index);
        temp1[index] = 0;

        setenv(temp1, getenv(commandstr[1]+index+2), 1);
        
        free(temp1);
    }
}

void tokenize(char *command)
{
    memset(args, 0, sizeof(args));
    char *token;
    token = strtok(command, " \n\"'");

    while (token != NULL)
    {
        args[numArgs] = token;
        token = strtok(NULL, " \n\"'");
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
