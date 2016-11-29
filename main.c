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

#define BUFFER_SIZE 32
#define READ_END 0
#define WRITE_END 1
#define NUM_CHILD 5
#define MAX_TIME 30.0

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

void runChildProcess(int childID, int* fd)
{
	char buf[BUFFER_SIZE];
	int minsElapsed;
	double secsElapsed;
	int messageNumber;
	time_t start, end;
	double elapsed;

	srand(time(NULL) ^ (getpid()<<16));
	start = time(NULL);
	elapsed = 0;
	messageNumber = 1; // message number

	// do stuff for 30 seconds
	for (;;) {
		sleep(rand() % 3);
		end = time(NULL);
		elapsed = difftime(end, start);

		if (elapsed < MAX_TIME) {
			getElapsedSeconds(&minsElapsed, &secsElapsed);
			snprintf(buf, sizeof buf, "0:%2.3f: Child %d message %d", secsElapsed, childID + 1, messageNumber++);
			close(*fd + READ_END);
			write(*fd + WRITE_END, buf, strlen(buf) + 1);
		} else {
			break;	
		}
	}
	// close pipe, next read will return 0
	close(*fd + WRITE_END);
	printf("Child %d done.\n", childID + 1);
	exit(0);
}

int main()
{
	char read_msg[BUFFER_SIZE];

	pid_t pid;
	int fd[NUM_CHILD * 2];
	int i, j, k;

	char buffer[BUFFER_SIZE * NUM_CHILD];
	int result, nread;

	fd_set inputs, inputfds;

	FD_ZERO(&inputs);

	FILE *f = fopen("output.txt", "w");
	if (f == NULL) {
		perror("Error opening file!\n");
		exit(1);
	}

	// Create pipes
	for (i = 0; i < NUM_CHILD * 2; i += 2) {
		if (pipe(fd + i) == -1) {
			fprintf(stderr, "pipe() failed");	
			return 1;
		}
		FD_SET(fd[i], &inputs);
	}

	// Create child processes
	for (i = 0; i < NUM_CHILD; ++i) {
		pid = fork();	
		if (pid == 0) {
			// child process
			j = i * 2;

			// break here to continue with code below
			break;
		} else if (pid < 0) {
			fprintf(stderr, "fork() failed");
			return 1;
		}
	}

	if (pid > 0) {
		// parent process

		// number of running child processes
		k = NUM_CHILD;

		while (k > 0) {
			inputfds = inputs;

			result = select(fd[NUM_CHILD * 2 - 1], &inputfds,
					NULL, NULL, NULL);

			switch(result) {
				case 0: {
					fflush(stdout);
					break;
				}
				case -1: {
					perror("select");
					exit(1);
				}
				default: {
					for (j = 0; j < NUM_CHILD * 2 && result != 0; j += 2) {
						if (FD_ISSET(fd[READ_END + j], &inputfds)) {
							ioctl(fd[READ_END + j], FIONREAD, &nread);
							
							close(fd[WRITE_END + j]);
							nread = read(fd[READ_END + j], buffer, nread);

							if (nread == 0) {
								// pipe is closed, child is done
								--k;
							} else {
								buffer[nread] = 0;
								// TODO: add timestamp
								fprintf(f, "%s\n", buffer);
							}
							--result;
						}
					}
				}
			}
		}

		// close file
		fclose(f);

		puts("All children exited.");
	} else if (pid == 0) {
		// child process
		runChildProcess(i, &fd[j]);
	}

	return 0;
}
