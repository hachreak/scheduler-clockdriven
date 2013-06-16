#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>

#include "busy.h"

unsigned int calib = UINT_MAX/32;

void busy_wait(unsigned int millisec){
  unsigned int i, j;
  volatile unsigned int value;

  for (i=0; i < millisec; ++i)
    for (j=0; j < calib; ++j)
      value *= i+j; // dummy code
}

void busy_calib(){
  struct timeval before;
  struct timeval after;
  unsigned int duration;

  int i = 0;
  do{
    gettimeofday(&before, NULL);
    busy_wait(1+i);
    gettimeofday(&after, NULL);

    duration = (after.tv_sec - before.tv_sec) * 1000000;
    duration += after.tv_usec;
    duration -= before.tv_usec;

    calib += 1000000;

    printf("[BUSY WAIT] [CALIBRATION] [%d nsec] %d\n", duration, calib);
  }while(duration < 1000000);

  printf("[BUSY WAIT] [DURATION 1 msec ] [%d nsec]\n", duration);

  calib-=10000000;

  calib /= duration;
  calib *= 1000;
}
