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

void executive_init();
void executive_wait_next_frame();
void *executive();
void *sp_task_handler();

#endif
