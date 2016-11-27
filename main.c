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
  * Calculates elapsed time
  */
 double getElapsedSeconds()
 {
     struct timeval now;
     gettimeofday(&now, NULL);
     double seconds = (double) now.tv_sec - start.tv_sec;
     double useconds = (double) now.tv_usec - start.tv_usec;
     double result = seconds + (useconds / 1000000);
     return round(result * 1000.0) / 1000.0;
 }

/**
 * formats current time
 */
void currentMinSec(int* minVal, double* secVal)
{
    double timeElapsed = getElapsedSeconds();
    int intSec = floor(timeElapsed);
    *secVal = timeElapsed - intSec;
    *minVal = intSec / 60;
    intSec %= 60;
    *secVal += intSec;
}


int main() {

    int minVal;
    double secVal;
    for (int i = 0; i < 10; ++i) {

         printf("%lf", getElapsedSeconds());
        printf("%\n");
        sleep(1);
       // currentMinSec(&minVal, &secVal);
        //printf("\n%d:%06.3f: Parent received message:\n", minVal, secVal);
    }
    return 0;
}
