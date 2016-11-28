//
// Created by Gurpreet on 11/26/2016.
//

#include <math.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define BUFFER_SIZE 32
#define READ_END 0
#define WRITE_END 1
#define NUM_PIPES 5
#define NUM_CHILD 5

struct timeval start;


/*
 * Calculates elapsed time and formats it
 */
void getElapsedSeconds(int* minsElapsed, double* secsElapsed)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    double seconds = (double) now.tv_sec - start.tv_sec;
    double useconds = (double) now.tv_usec - start.tv_usec;
    double result = seconds + (useconds / 1000000);
    double temp =  round(result * 1000.0) / 1000.0;

    int intSec = floor(temp);
    *secsElapsed = temp - intSec;
    *minsElapsed = intSec / 60;
    intSec %= 60;
    *secsElapsed += intSec;

}




int main() {
	char write_msg[BUFFER_SIZE] = "You're my child process";
	char read_msg[BUFFER_SIZE];

	pid_t pid;
	pid_t pids[NUM_CHILD];
	int fd[2 * NUM_PIPES];
	int i, j;
    int minsElapsed;
    double secsElapsed;

	for (i = 0; i < NUM_PIPES * 2; i += 2) {
		if (pipe(fd + i) == -1) {
			fprintf(stderr, "pipe() failed");	
			return 1;
		}
	}

	for (i = 0, j = 0; i < NUM_CHILD; ++i, j = 2 * i) {
		pid = fork();
		if (pid > 0) {
			close(fd[READ_END + j]);
			write(fd[WRITE_END + j], write_msg, strlen(write_msg) + 1);

			printf("Parent %d: Wrote '%s' to the pipe.\n", pid, write_msg);

			close(fd[WRITE_END + j]);
		} else if (pid == 0) {
			close(fd[WRITE_END + j]);
			read(fd[READ_END + j], read_msg, BUFFER_SIZE);

			printf("Child of %d: Read '%s' from the pipe.\n", getpid(), read_msg);

			close(fd[READ_END + j]);
			exit(0);
		} else {
			fprintf(stderr, "fork() failed");
			return 1;
		}
	}

//    for (int i = 0; i < 10; ++i) {
//        getElapsedSeconds(&minsElapsed, &secsElapsed);
//        printf("%1d:%2.3f", 0, secsElapsed);
//        printf("\n");
//        sleep(1); //for testing generating time stamps
//    }
    return 0;
}
