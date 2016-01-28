//miniShell.c
//Jacob Shumate

#include <stdlib.h>
#include <stdint.h>     //uint32_t
#include <stdio.h>
#include <stdbool.h>    //bool
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>   //gettimeofday
#include <time.h>       //struct tm, localtime, 
#include <unistd.h>     //execv
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

char *fileParse(char *input);
void parse(char *input, char **command);
char *findPath(char **command);
char *fullPath(char *path, char *command);
void prompt(char *input);
void execute(char **command, char *path, char *file);
uint32_t timerStart();
void timerStop(uint32_t start);
void signalHandler(int signum);
void directory(char **command);

bool f = false;

int main()
{
        //register signal SIGINT and signal handler
        //ctrl-c to exit
        signal(SIGINT, signalHandler);

        char *command[50];
        char input[500];
        char *file;
        char *path;
        uint32_t timer;
        bool t = true;

        printf("\nEnter ctrl-c to close shell.\n");
        printf("Enter 'timer' to turn timer on/off.\n");

        while(1)
        {
                prompt(input);
                file = fileParse(input);
                parse(input, command);

		if(strcmp(command[0], "cd") == 0)
		{
			directory(command);
			continue;
		}
		else
                	path = findPath(command);

                //printf("command: %s\n", command[0]);
                //printf("path: %s\n", path);

                //if(strcmp(command[0], "exit") == 0)
                        //exit(0);

                if(strcmp(command[0], "timer") == 0)
                {
                        t = !t;
                        //go to next while loop
                        continue;
                }

                //deug print out each string of chars
                char *p = command[0];
                int i = 0;
                while(p != NULL)
                {
                        printf("%d - %s\r\n", i, p);
                        i++;
                        p = command[i];
                }
                printf("\n");

                timer = timerStart();

                execute(command, path, file);

                if(t)
                        timerStop(timer);

                //free up memory block
                free(path);
                f = false;
        }

        return 0;
}


char *fileParse(char *input)
{
        char *ptr;
        char *token;
        char *inputBuffer;
        char *file = NULL;

        //if string contains first occurence
        //of >
        if(strchr(input, '>') != NULL)
        {
                f = true;

                //printf("input: %s\n", input);

                //left of > is copied to inputBuffer
                token = strtok_r(input, ">", &ptr);
                inputBuffer = token;
                //right of > is saved
                input = ptr;
                //printf("inputBuffer: %s\n", inputBuffer);

                //right of > is copied to file
                token = strtok_r(input, " ", &ptr);
                file = token;
                //now inputBuffer can be copied over
                //first input
                input = inputBuffer;
                //printf("new file: %s\n", file);
                //printf("new input: %s\n", input);         
        }

        return file;
}

void parse(char *input, char **command)
{
        char *ptr;
        char *token;
        int i = 0;

        while ((token = strtok_r(input, " ", &ptr)) != NULL)
        {
                command[i] = token;
                i++;

                //start from last place
                input = ptr;
        }

        command[i] = '\0';
}

char *findPath(char **command)
{
        char *ptr;
        char *path = getenv("PATH");
        char pathCopy[1024];
        strcpy(pathCopy, path);
        path = pathCopy;

        //printf("Path getenv: %s\n\n", path);  

        char *token;
        char *finalPath;
        //store string of path into token up until :
        //if end of string of path then exit while loop
        while((token = strtok_r(path, ":", &ptr)) != NULL)
        {
                //printf("Searching Path: %s\n", token);

                //finalPath is token with / and command[0]
                //concatenated
                finalPath = fullPath(token, command[0]);

                //if file exists, break out of while loop
                if(access(finalPath, F_OK) != -1)
                        break;
                //if not then start from last place
                //of path string
                else
                        path = ptr;
        }

        return finalPath;
}

char *fullPath(char *path, char *command)
{
        int len = strlen(path) + strlen(command) + 3;
        //allocate memory block
        char *buffer = malloc(len);
        strcpy(buffer, path);
        int bufflen = strlen(buffer);
        //if(strncmp(buffer[bufflen-1], "/", 1) != 0)
                strcat(buffer, "/");
        strcat(buffer, command);
        return buffer;
}

void prompt(char *input)
{
        printf("Shell -> ");
        fgets(input, 500, stdin);
        int l = strlen(input);
        input[l-1] = '\0';
        printf("\n");
}


void execute(char **command, char *path, char *file)
{
        pid_t pid;
        int status;

        //printf("\n Path is %s\n", path);

        //fork a child process
        if((pid = fork()) < 0)
        {
                printf("*** ERROR: forking child process failed\n");
                exit(1);
        }

        //for the child process: execute the command
        else if(pid == 0)
        {
                if(f)
                {
                        int F = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                        close(STDOUT_FILENO);
                        dup(F);
                }
		
		execv(path, command);

                printf("*** ERROR: exec failed\n");
                exit(1);
        }

        //for the parent: wait for completion
        else
        {
                waitpid(-1, &status, 0);
        }
}

uint32_t timerStart()
{
        struct timeval tv;
        struct timezone tz;
        struct tm *tm;
        uint32_t start;

        gettimeofday(&tv, &tz);
        tm = localtime(&tv.tv_sec);

        start = tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
                tm->tm_sec * 1000 + tv.tv_usec / 1000;

        return (start);
}

void timerStop(uint32_t start)
{
        struct timeval tv;
        struct timezone tz;
        struct tm *tm;
        uint32_t stop;

        gettimeofday(&tv, &tz);
        tm = localtime(&tv.tv_sec);

        stop = tm->tm_hour * 3600 * 1000 + tm->tm_min * 60 * 1000 +
               tm->tm_sec * 1000 + tv.tv_usec / 1000;

        printf("\nCommand took %d ms\n", stop - start);
}

void signalHandler(int signum)
{
        printf("\n\nClosing shell...\n");
        exit(0);
}

void directory(char **command)
{
	//if directory is specified
	if(command[1] != NULL)
	{
		printf("changing dir to: %s\n", command[1]);
		
		chdir(command[1]);
	}
	//if no directory is specified
	else
	{
		char *home = getenv("HOME");
		printf("changing to home dir: %s\n", home);

		chdir(home);
	}
}
