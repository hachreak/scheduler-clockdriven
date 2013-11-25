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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "task.h"
#include "executive.h"
#include "excstate.h"

// absolute time useful to jump in the next frame
struct timespec abstime;
// how log waiting for the next frame
long long time2wait;

#ifdef SLACK_STEALING
// absolute time to jump in exhausted slack time
struct timespec sp_abstime;
// how long waiting for the finish of slack time in this frame
long long slacktime2wait;
#endif

// mutex used to simulate a wait condition and wait the timeout
static pthread_cond_t efb_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t efb_mutex = PTHREAD_MUTEX_INITIALIZER;

// frames descriptors
frame_descriptor *frame_descs;

// frame index
excstate executive_frame_index;

// sporadic thread descriptor
frame_descriptor executive_sp_frame_desc;

// how many frame to wait before test the deadline
excstate executive_sp_count_frame;

// executive frame descriptor
frame_descriptor executive_exec_desc;

/**
 * Check the deadline for a frame
 * @param index to understand which frame
 */
void executive_check_deadline_frame(int index){
  printf("[FRAME %d] [DEADLINE CHECK]\n", index);
  int state = excstate_get_state(&frame_descs[index].excstate);
  if(state == WORKING || state == PENDING){
    int task = excstate_get_state(&frame_descs[index].task_in_execution);
    printf("[FRAME %d] [DEADLINE MISS] [TASK %d]!\n", index, task);
    exit(1);
  }
}

/**
 * Test if the sporadic thread can respect the deadline.
 * Compute how many frame the executive have to wait before test the sp task deadline (executive_sp_count_frame)
 * @param frame_index to understand where the sporadic thread start (which frame)
 */
bool executive_sp_check_static_deadline(int frame_index){
  int index = frame_index;
  index++;
  int quant = H_PERIOD / NUM_FRAMES;
  int qi = index * quant;
  int y = 1;
  int i = 0;
  int count_slack = 0;
  while((i = qi + (y*quant)) <= (qi + SP_DLINE)){
    int slack = SLACK[(index+y-1)%NUM_FRAMES];
    count_slack += slack;
    printf("SLACK[%d] = %d; CURMAX[%d] DEADLINE[%d]\n", (index+y-1)%NUM_FRAMES, slack, i, (qi + SP_DLINE));
    y++;
  }
  printf("[SP] [WAIT] %d frame\n", --y);
  // set how many frame wait for check deadline
  excstate_set_state(&executive_sp_count_frame, y);
  // return static test of the deadline
  return (count_slack >= SP_WCET ? true : false);
}

/**
 * Check any deadline miss of the sp task if the deadline for this task is arrived.
 */
void executive_sp_check_deadline(){
  // update counter of frame to wait
  int state = excstate_get_state(&executive_sp_frame_desc.excstate);

  if(state == WORKING || state == COMPLETED){
    //get which frame we are executing
    int frames = excstate_get_state(&executive_sp_count_frame);
    // set how many frame to wait before test deadline of sp task
    excstate_set_state(&executive_sp_count_frame, --frames);
    printf("[SP] [WAIT] [%d FRAMES]\n", excstate_get_state(&executive_sp_count_frame));

    // test if execute should check deadline of sp task
    if(frames < 0){
      printf("[SP] [DEADLINE CHECK]\n");
      if(state == WORKING || state == PENDING){
        printf("[SP] [ERROR] SP not respect deadline!\n");
        exit(1);
      }

      // set state IDLE
      excstate_set_state(&executive_sp_frame_desc.excstate, IDLE);
    }
  }
}

/**
 * Try to schedule a sporadic task
 */
bool sp_task_request(){
  // Check if another sporadic thread is already in execution
  if(excstate_get_state(&executive_sp_frame_desc.excstate) != IDLE){
    // another sp task was already scheduled
    printf("[SP] [ALREADY SCHEDULED] can't schedule new sporadic job\n");
    return false;
  }

  // read where we are
  int index = excstate_get_state(&executive_frame_index);
  printf("[SP] [CHECK SP REQ] frame %d\n", index);
  
  // acceptance test
  if(!executive_sp_check_static_deadline(index)){
    printf("[SP] [ERROR] Can't schedule SP task\n");
    exit(1);
  }

  printf("[SP] [TASK SCHEDULED]\n");

  // wakeup next frame
  printf("[SP] [READY] SP frame!\n");
  excstate_set_state(&executive_sp_frame_desc.excstate, READY);
  return true;
}

/**
 * SP task handler
 */
void *sp_task_handler(){
  while(1){
    if(excstate_get_state(&executive_sp_frame_desc.excstate) == WORKING){
      // set state COMPLETED
      excstate_set_state(&executive_sp_frame_desc.excstate, COMPLETED);
    }

    // wait activation (PENDING) from executer
    excstate_wait_running(&executive_sp_frame_desc.excstate);

    // set state WORKING
    excstate_set_state(&executive_sp_frame_desc.excstate, WORKING);

    printf("[SP] [WAKE UP] SP frame\n");
    // execute sp task
    printf("[SP] [EXEC SP TASK]\n");
    SP_TASK();
    printf("[SP] [END]\n");
  }
  
  return NULL;
}

/**
 * Init frames: compute how is long a frame
 */
void executive_init_frame(){
  float quant = H_PERIOD / NUM_FRAMES;
  time2wait = quant * EXECUTIVE_QUANT * 1000000;
  printf("[INIT TIME] [QUANT %f] [EXECUTIVE QUANT %d] [NANOSEC FRAME %lld]\n", quant, EXECUTIVE_QUANT, time2wait);
}

/**
 * Frame handler
 * @param arg index to understand which frame is
 */
void *frame_handler(void *arg){
  // get index
  int index = * (int *) arg; 

  while(1){
    // set state IDLE
    excstate_set_state(&frame_descs[index].excstate, IDLE);

    printf("[FRAME %d] [WAIT WAKE UP]\n", index);
    // wait activation (PENDING) from executer
    excstate_wait_running(&frame_descs[index].excstate);

    // set state WORKING
    excstate_set_state(&frame_descs[index].excstate, WORKING);

    printf("[FRAME %d] [WAKE UP]\n", index);

    // execute task list
    int task_index = 0, i = 0;
    while((task_index = SCHEDULE[index][i]) != -1){
      printf("[FRAME %d] [EXECUTE %d] [INIT]\n", index, task_index);
      // save which task you are performing
      excstate_set_state(&frame_descs[index].task_in_execution, task_index);
      // execute!
      P_TASKS[task_index]();
      printf("[FRAME %d] [EXECUTE %d] [END]\n", index, task_index);
      // next task
      i++;
    }

  }

  return NULL;
}

/**
 * Executive handler
 */
void *executive(){
  struct timeval utime;
  gettimeofday(&utime,NULL);

  abstime.tv_sec  = utime.tv_sec;
  abstime.tv_nsec = utime.tv_usec * 1000;

  int index = 0;
  while(true){
    printf("\n------[FRAME %d]-------\n",index);

    // compute next timeout
    abstime.tv_sec  += ( abstime.tv_nsec + time2wait ) / 1000000000;
    abstime.tv_nsec  = ( abstime.tv_nsec + time2wait ) % 1000000000;

#ifdef SLACK_STEALING
    // if sp thread is not IDLE
    if(excstate_get_state(&executive_sp_frame_desc.excstate) != IDLE){
       // compute next timeout: end of slack time available (nsec)
       slacktime2wait = SLACK[index] * EXECUTIVE_QUANT * 1000000;
       // set HIGH priority for sp thread
       struct sched_param param;
       param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 2;
       pthread_setschedparam(executive_sp_frame_desc.pchild, SCHED_FIFO, &param);
       printf("[SP] [SET PRIORITY %d] [HIGH]\n", param.sched_priority);
    // endif
    }
#endif

    // wakeup next frame
    printf("[FRAME %d] [PENDING]!\n", index);
    excstate_set_state(&frame_descs[index].excstate, PENDING);

    // wakeup sporadic thread
    if(excstate_get_state(&executive_sp_frame_desc.excstate) == READY){
      printf("[SP] [PENDING] sp thread!\n");
      excstate_set_state(&executive_sp_frame_desc.excstate, PENDING);
    }
    
#ifdef SLACK_STEALING
    // if sp thread is WORKING
    int state = excstate_get_state(&executive_sp_frame_desc.excstate);
    if(state != IDLE){
       // wait slack time
       sp_abstime.tv_sec  += ( abstime.tv_nsec + slacktime2wait ) / 1000000000;
       sp_abstime.tv_nsec  = ( abstime.tv_nsec + slacktime2wait ) % 1000000000;
       pthread_cond_timedwait(&efb_cond, &efb_mutex, &sp_abstime);
       // set LOW priority for sp thread
       struct sched_param param;
       param.sched_priority = sched_get_priority_max(SCHED_FIFO) - NUM_FRAMES - 3;
       pthread_setschedparam(executive_sp_frame_desc.pchild, SCHED_FIFO, &param);
       printf("[SP] [SET PRIORITY %d] [LOW]\n", param.sched_priority);
    // endif
    }
#endif

    // wait frame finish to compute
    pthread_cond_timedwait(&efb_cond, &efb_mutex, &abstime);
    printf("------[END FRAME]------ wait %lld nsec - next %lld sec %li nsec\n\n", time2wait, (long long)abstime.tv_sec, abstime.tv_nsec); 

    // check deadline of the frame
    executive_check_deadline_frame(index);
    // check deadline of the SP task!
    executive_sp_check_deadline();
 
    // next frame
    index++;
    index = index % NUM_FRAMES;
    
    // set frame_index
    excstate_set_state(&executive_frame_index, index);
  }
}

/**
 * Set priority attribute
 * @param attr attribute
 * @param priority which priority have to set
 */
void executive_new_pthread_attr(pthread_attr_t *attr, int priority){
  // attr init
  if(pthread_attr_init(attr)){
    perror("error init attr\n");
    exit(1);
  }

  // set scheduler FIFO
  if(pthread_attr_setschedpolicy(attr, SCHED_FIFO)){
    perror("error init attr - set policy\n");
    exit(1);
  }

  // set inherit scheduler
  pthread_attr_setinheritsched(attr,  PTHREAD_EXPLICIT_SCHED);

  // set priority
  struct sched_param p;
  p.sched_priority = priority; 
  // set scheduler Sched Param
  if(pthread_attr_setschedparam(attr, &p)){
    perror("error init attr - set param\n");
    exit(1);
  }
}

/**
 * Executive initalization
 */
void executive_init(){
  // task init
  task_init();

  // init frame
  executive_init_frame();

  // init frame index
  excstate_init(&executive_frame_index, 0);

  // init count frame
  excstate_init(&executive_sp_count_frame, 0);

  // init tasks descriptor
  frame_descs = (frame_descriptor *)malloc(sizeof(frame_descriptor) * NUM_FRAMES);

  int max_priority = sched_get_priority_max(SCHED_FIFO);

  // [INIT PTHREADS]
  int i = 1;
  for(i=0; i<NUM_FRAMES; i++){
    // [new task descriptor]
    frame_descriptor *fd = &frame_descs[i];

    // [init excstate]
    excstate_init(&fd->excstate, IDLE);

    // [set attr]
    pthread_attr_t attr;

    int frame_priority = max_priority - 3;

#ifndef FRAME_HANDLER_SAME_PRIORITY
    frame_priority -= i;
#endif

    executive_new_pthread_attr(&attr, frame_priority);

    printf("[FRAME %d] [TASK INIT] priority %d\n", i, frame_priority);

    // set index
    fd->index = i;

    // [create task]
    pthread_create( &fd->pchild, &attr, frame_handler, &fd->index);
  }

  // [INIT SPORADIC THREAD]

  // init sporadic thread state
  excstate_init(&executive_sp_frame_desc.excstate, IDLE);

  // set attr thread sp task handler
  printf("[SP] [TASK INIT] priority %d\n", max_priority - i - 3);
  pthread_attr_t attr_sp;
  executive_new_pthread_attr(&attr_sp, max_priority - i - 3);

  // create task
  pthread_create(&executive_sp_frame_desc.pchild, &attr_sp, &sp_task_handler, NULL);

  // [INIT EXECUTIVE THREAD]

  // set attr thread executive task handler
  printf("[EXECUTIVE] [TASK INIT] priority %d\n", max_priority - 1);
  pthread_attr_t attr_exec;
  executive_new_pthread_attr(&attr_exec, max_priority - 1);

  // create task
  pthread_create(&executive_exec_desc.pchild, &attr_exec, &executive, NULL);

  // wait executive thread
  pthread_join(executive_exec_desc.pchild, NULL);
 
  // wait thread
  /*for(i=0; i<NUM_P_TASKS; i++){
    frame_descriptor fd = frame_descs[i];
    pthread_join( fd.pchild, NULL );
  }*/
}
