#ifndef _EXECUTIVE_H
#define _EXECUTIVE_H

#define EXECUTIVE_QUANT 100

#include "excstate.h"

typedef struct{
  // tasks
  pthread_t pchild;
  excstate   excstate;
  //void *function;
  int index;
  // task in execution
  excstate task_in_execution;
} frame_descriptor;

//typedef enum frame_state{ IDLE, WORKING, PENDING } frame_state;

/**
 * Executive initialization
 */
void executive_init();

#endif
