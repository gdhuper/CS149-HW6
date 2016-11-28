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

    int minsElapsed;
    double secsElapsed;
    for (int i = 0; i < 10; ++i) {
        getElapsedSeconds(&minsElapsed, &secsElapsed);
        printf("%1d:%2.3f", 0, secsElapsed);
        printf("%\n");
        sleep(1); //for testing generating time stamps
    }
    getElapsedSeconds(&minsElapsed, &secsElapsed);
    printf("%2.3f", secsElapsed);
    return 0;
}