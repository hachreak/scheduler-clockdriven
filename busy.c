/**
 * Copyright (C) 2013 Leonardo Rossi <leonardo.rossi@studenti.unipr.it>
 *
 * This source code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this source code; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>

#include "busy.h"

unsigned int calib = UINT_MAX/32;

void busy_wait(unsigned int millisec){
  printf("[BUSY WAIT] [%d msec]\n", millisec);
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

#ifdef BUSYCALIB_ADVANCED

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

  calib-=10000000;

#else

  gettimeofday(&before, NULL);
  busy_wait(1);
  gettimeofday(&after, NULL);

  duration = (after.tv_sec - before.tv_sec) * 1000000;
  duration += after.tv_usec;
  duration -= before.tv_usec;

#endif

  printf("[BUSY WAIT] [DURATION 1 msec ] [%d nsec]\n", duration);

  calib /= duration;
  calib *= 1000;
}
