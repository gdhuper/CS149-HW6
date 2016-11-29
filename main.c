/*
 * CS149 Section-1 Group-9
 */
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define BUFFER_SIZE 64
#define READ_END 0
#define WRITE_END 1
#define NUM_CHILD 5
#define MAX_TIME 30.0

struct timeval old;

/*
 * Calculates elapsed time and formats it
 */
void getElapsedSeconds(int* minsElapsed, double* secsElapsed)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    double secs = (double) now.tv_sec - old.tv_sec;
    double usecs = (double) now.tv_usec - old.tv_usec;
    double result = secs + (usecs / 1000000);
    double temp =  round(result * 1000.0) / 1000.0;

    int intSec = floor(temp);
    *secsElapsed = temp - intSec;
    *minsElapsed = intSec / 60;
    intSec %= 60;
    *secsElapsed += intSec;


}

/*
 * Simulates a child process
 */
void runChildProcess(int childID, int* fd)
{
    char buf[BUFFER_SIZE];
    int minsElapsed;
    double secsElapsed;
    double tempSecs;
    int messageNumber;
    time_t start, end;
    double elapsed;
	char str[32];

    srand(time(NULL) ^ (getpid()<<16));
    start = time(NULL);
    elapsed = 0;
    messageNumber = 1; // message number

    // do stuff for 30 seconds
    //getElapsedSeconds(&minsElapsed, &tempSecs);
    for (;;) {
		if (childID != 5) {
			sleep(rand() % 3);
		}
		end = time(NULL);
		elapsed = difftime(end, start);

        if (elapsed < MAX_TIME) {
            if (childID == 5) {
                printf( "Enter a message for Child 5: ");
                fgets(str, sizeof(str), stdin);
				str[strcspn(str, "\n")] = 0; // remove /n

				if (strlen(str) > 0) {
					getElapsedSeconds(&minsElapsed, &secsElapsed);
					snprintf(buf, sizeof buf, "%d:%06.3f: Child %d: %s \n", minsElapsed, secsElapsed, childID, str);
					close(*fd + READ_END);
					write(*fd + WRITE_END, buf, strlen(buf) + 1);
				}
            } else {
                getElapsedSeconds(&minsElapsed, &secsElapsed);
                snprintf(buf, sizeof buf, "%d:%06.3f: Child %d message %d \n", minsElapsed, secsElapsed, childID, messageNumber++);
				close(*fd + READ_END);
				write(*fd + WRITE_END, buf, strlen(buf) + 1);
            }
        } else {
            break;
        }
    }
    // close pipe, next read will return 0
    close(*fd + WRITE_END);
    printf("Child %d done.\n", childID);
    exit(0);
}

/**
 * Simulates parent process
 */
void runParentProcess(fd_set inputs, int fd[])
{
    fd_set inputfds;
    int i, j;
    char read_message[BUFFER_SIZE * NUM_CHILD];
    int result, nread;
    int minsElapsed;
    double secsElapsed;

    FILE *f = fopen("output.txt", "w");
    if (f == NULL) {
        perror("Error opening file!\n");
        exit(1);
    }

    j = NUM_CHILD;

    while (j > 0) {
        inputfds = inputs;

        result = select(fd[NUM_CHILD * 2 - 1], &inputfds,
                        NULL, NULL, NULL);

        switch(result) {
            case 0:
                fflush(stdout);
                break;
            case -1:
                perror("select");
                exit(1);
            default:
                for (i = 0; i < NUM_CHILD * 2 && result > 0; i += 2) {
                    if (FD_ISSET(fd[READ_END + i], &inputfds)) {
                        ioctl(fd[READ_END + i], FIONREAD, &nread);

                        close(fd[WRITE_END + i]);
                        nread = read(fd[READ_END + i], read_message, nread);

                        if (nread == 0) {
                            // pipe is closed, child is done
                            --j;
                        } else {
                            read_message[nread] = 0;
							getElapsedSeconds(&minsElapsed, &secsElapsed);
                            fprintf(f, "%d:%06.3f: %s\n", minsElapsed, secsElapsed, read_message);
                        }
                        --result;
                    }
                }
        }
    }

    // close file
    fclose(f);

    puts("All children exited.");
}

int main()
{
    pid_t pid;
    int fd[NUM_CHILD * 2];
    int i;
    fd_set inputs;

    FD_ZERO(&inputs);

    // Create pipes
    for (i = 0; i < NUM_CHILD * 2; i += 2) {
        if (pipe(fd + i) == -1) {
            fprintf(stderr, "pipe() failed");
            return 1;
        }
        FD_SET(fd[i], &inputs);
    }
 	gettimeofday(&old, NULL);
 	
    // Create child processes
    for (i = 0; i < NUM_CHILD; ++i) {
        pid = fork();
        if (pid == 0) {
            // child process
            // break here to continue with code below
            break;
        } else if (pid < 0) {
            fprintf(stderr, "fork() failed");
            return 1;
        }
    }

   

    if (pid > 0) {
        // parent process
        runParentProcess(inputs, fd);
    } else if (pid == 0) {
        // child process
        runChildProcess(i + 1, &fd[i * 2]);
    }

    return 0;
}