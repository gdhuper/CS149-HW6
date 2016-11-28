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

#define BUFFER_SIZE 256
#define READ_END 0
#define WRITE_END 1
#define NUM_PIPES 5
#define NUM_CHILD 5
#define MAX_TIME 10.0

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
	char read_msg[BUFFER_SIZE];

	time_t start, end;
	double elapsed;
	pid_t pid;
	int fd[NUM_PIPES * 2];
	int i, j, k, l;
	int minsElapsed;
	double secsElapsed;

	char buffer[128];
	int result, nread;

	fd_set inputs, inputfds;
	struct timeval timeout;

	FD_ZERO(&inputs);

	for (i = 0; i < NUM_PIPES * 2; i += 2) {
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

		k = NUM_CHILD;
		start = time(NULL);

		while (k > 0) {
			inputfds = inputs;

			timeout.tv_sec = 2;
			timeout.tv_usec = 500000;

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
							
							close(fd[WRITE_END +j]);
							nread = read(fd[READ_END + j], buffer, nread);

							if (nread == 0) {
								--k;
							} else {
								buffer[nread] = 0;
								printf("%s\n", buffer);
							}
							--result;
						}
					}
				}
			}
		}
		puts("All children exited");
	} else if (pid == 0) {
		// child process

		char buf[BUFFER_SIZE];

		srand(time(NULL) ^ (getpid()<<16));
		start = time(NULL);
		elapsed = 0;
		l = 1; // message number

		// do stuff for 30 seconds
		for (;;) {
			sleep(rand() % 3);
			end = time(NULL);
			elapsed = difftime(end, start);

			if (elapsed < MAX_TIME) {
				snprintf(buf, sizeof buf, "%2.3f: Child %d message %d", elapsed, i + 1, l++);
				close(fd[READ_END + j]);
				write(fd[WRITE_END + j], buf, strlen(buf) + 1);
			} else {
				break;	
			}
		}
		close(fd[WRITE_END + j]);
		printf("Child %d done.\n", i + 1);
		exit(0);
	}

	//    for (int i = 0; i < 10; ++i) {
	//        getElapsedSeconds(&minsElapsed, &secsElapsed);
	//        printf("%1d:%2.3f", 0, secsElapsed);
	//        printf("\n");
	//        sleep(1); //for testing generating time stamps
	//    }
	return 0;
}
