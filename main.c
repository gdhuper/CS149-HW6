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
	double seconds = (double) now.tv_sec - old.tv_sec;
	double useconds = (double) now.tv_usec - old.tv_usec;
	double result = seconds + (useconds / 1000000);
	double temp =  round(result * 1000.0) / 1000.0;

	int intSec = floor(temp);
	*secsElapsed = temp - intSec;
	*minsElapsed = intSec / 60;
	intSec %= 60;
	*secsElapsed += intSec;
	

}

void runChildProcess(int childID, int* fd)
{
	char write_msg[BUFFER_SIZE];
	int minsElapsed;
	double secsElapsed;
	double tempSecs;
	int messageNumber;
	time_t start, end;
	double elapsed;

	srand(time(NULL) ^ (getpid()<<16));
	start = time(NULL);
	elapsed = 0;
	messageNumber = 1;

	// do stuff for 30 seconds
	//getElapsedSeconds(&minsElapsed, &tempSecs);
	gettimeofday(&old, NULL);
	for (;;) {
		sleep(rand() % 3);
		end = time(NULL);
		elapsed = difftime(end, start);
		
		if (elapsed < MAX_TIME) {
			getElapsedSeconds(&minsElapsed, &secsElapsed);
			snprintf(write_msg, sizeof write_msg, "%d:%06.3f: Child %d message %d", minsElapsed, secsElapsed, childID, messageNumber++);
			close(*fd + READ_END);
			write(*fd + WRITE_END, write_msg, strlen(write_msg) + 1);
		} else {
			break;	
		}
	}
	// close pipe, next read will return 0
	close(*fd + WRITE_END);
	printf("Child %d done.\n", childID);
	exit(0);
}

void runParentProcess(int fd[])
{
	fd_set inputs, inputfds;
	int i, j;
	char read_message[BUFFER_SIZE * NUM_CHILD];
	int result, nread;
	int runningChildren[NUM_CHILD]; 

	FILE *f = fopen("output.txt", "w");
	if (f == NULL) {
		perror("Error opening file!\n");
		exit(1);
	}
	
	for (i = 0; i < NUM_CHILD; ++i) runningChildren[i] = 1;

	j = NUM_CHILD;

	while (j > 0) {
		FD_ZERO(&inputs);
		for (i = 0; i < NUM_CHILD; ++i) {
			if (runningChildren[i]) {
				FD_SET(fd[i * 2], &inputs);
			}
		}
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
							FD_CLR(fd[READ_END + i], &inputs);
							runningChildren[i / 2] = 0;
							--j;
						} else {
							read_message[nread] = 0;
							printf("%s\n", read_message);
							// TODO: add timestamp
							fprintf(f, "%s\n", read_message);
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

	// Create pipes
	for (i = 0; i < NUM_CHILD * 2; i += 2) {
		if (pipe(fd + i) == -1) {
			fprintf(stderr, "pipe() failed");	
			return 1;
		}
	}

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
		runParentProcess(fd);
	} else if (pid == 0) {
		// child process
		runChildProcess(i + 1, &fd[i * 2]);
	}

	return 0;
}
