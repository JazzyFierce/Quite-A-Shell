#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/stat.h>

char buffer[1024];
char *args[256];
int numArgs = 0;
int runInBackground = 0;
int numBackgroundJobs = 0;

struct Process {
    int id;
    int pid;
    char* command;
};

struct Process currentBackgroundJobs[256];

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

void handlingCommands(char **commandstr, int* backgroundIsActiveArray)
{
    int exitStatus;
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
        
        if (strcmp("du",  commandstr[0]) ==0 ){
	        for (int i = 0; commandstr[i]; i++) {
                replace_with_env(&commandstr[i]);
            }
            execvp(commandstr[0], commandstr);
        }

        if (strcmp("kill", commandstr[0]) == 0) {
            int res = kill(atoi(commandstr[2]), atoi(commandstr[1]));
            for (int i; i < numBackgroundJobs; i++) {
                if (currentBackgroundJobs[i].pid == atoi(commandstr[1])) {
                    backgroundIsActiveArray[i] = 0;
                }
            }
        }

        if (strcmp("sleep", commandstr[0]) == 0) {
            sleep(atoi(commandstr[1]));
        }

        exit(0);
    }
    else
    {
        waitpid(pid, &exitStatus, 0);
    }
}

void handlingCommandsBackground(char** commandstr, char* cmd, int* backgroundIsActiveArray) {
    int pid = fork();

    if (pid == 0)
    {
        int index = numBackgroundJobs;
        printf("\nBackground job started: [%d] %d %s", numBackgroundJobs+1, getpid(), cmd); 
        handlingCommands(commandstr, backgroundIsActiveArray);
        printf("\nBackground job complete\n");
        backgroundIsActiveArray[numBackgroundJobs] = 0;
        exit(0);
    }
    else
    {
        currentBackgroundJobs[numBackgroundJobs].pid = pid;
        currentBackgroundJobs[numBackgroundJobs].command = strdup(cmd);
        currentBackgroundJobs[numBackgroundJobs].id = numBackgroundJobs+1;
        numBackgroundJobs++;
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

    return 0;
}

void changeDir(char** commandstr) {
    if (chdir(commandstr[1]) != 0) {
        printf("%s is not a directory\n", commandstr[1]);
    }
}

void tokenize(char *command)
{
    memset(args, 0, sizeof(args));
    char *token;
    token = strtok(command, " \n\"'");

    while (token != NULL)
    {
        // comments
        if (strcmp(token, "#") == 0 || token[0] == '#') {
            return;
        } 
        if (strcmp(token, "&") == 0) {
            runInBackground = 1;
            token = strtok(NULL, " \n\"'");
        }
        else {
            args[numArgs] = token;
            token = strtok(NULL, " \n\"'");
            numArgs++;
        }
    }
}

void jobs(int* backgroundIsActiveArray) {
    for (int i=0; i < numBackgroundJobs; i++) {
        if (backgroundIsActiveArray[i] == 1) {
            printf("[%d] %d %s\n", currentBackgroundJobs[i].id, currentBackgroundJobs[i].pid, currentBackgroundJobs[i].command);
        }
    }
}

int main()
{
    /* identifier for the shared memory segment */
    int segment_id;
    /* pointer to the shared memory segment */
    int *shared_buf;
    /* size of the shared memory segment */
    int size;
    /* allocate a shared memory segment */
    size = sizeof(int) * 256;
    segment_id = shmget(IPC_PRIVATE, size, S_IRUSR|S_IWUSR);
    /* attach the shared memory segment */
    shared_buf = (int *) shmat(segment_id, NULL, 0);

    while (1)
    {
        numArgs = 0;
        memset(buffer, 0, strlen(buffer));
        printf("[Quash]$ ");
        fgets(buffer, 1024, stdin);
        char* thing = strdup(buffer);
        tokenize(buffer);

        if (args[0] == NULL) {
            continue;
        }

        else if (runInBackground) {
            runInBackground = 0;
            shared_buf[numBackgroundJobs] = 1;
            handlingCommandsBackground(args, thing, shared_buf);
        } 
        else 
        {
            if (strcmp("exit", args[0]) == 0 || strcmp("quit", args[0]) == 0) { 
                exit(0);
            }
            else if (strcmp("export", args[0]) == 0) {
                export(args);
            }
            else if (strcmp("cd", args[0]) == 0) {
                changeDir(args);
            }
            else if (strcmp("jobs", args[0]) == 0) {
                jobs(shared_buf);
            }
            else
            {
                handlingCommands(args, shared_buf);
            }
        }
    }

    /* detach the shared memory segment */
    shmdt(shared_buf);
    /* remove the shared memory segment */
    shmctl(segment_id, IPC_RMID, NULL);
    return 0;
}
