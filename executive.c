/* traccia dell'executive (pseudocodice) */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "task.h"
#include "executive.h"
#include "excstate.h"

struct timespec abstime;
long long time2wait;

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

void executive_check_deadline_frame(int index){
  printf("[FRAME %d] [DEADLINE CHECK]\n", index);
  if(excstate_get_state(&frame_descs[index].excstate) == WORKING){
    int task = excstate_get_state(&frame_descs[index].task_in_execution);
    printf("[FRAME %d] [DEADLINE MISS] [TASK %d]!\n", index, task);
    exit(1);
  }
}

void executive_init_frame(){
  float quant = H_PERIOD / NUM_FRAMES;
  time2wait = quant * EXECUTIVE_QUANT * 1000000;
  printf("[INIT TIME] [QUANT %f] [EXECUTIVE QUANT %d] [NANOSEC FRAME %lld]\n", quant, EXECUTIVE_QUANT, time2wait);
}

bool executive_sp_check_static_deadline(int frame_index){
  int index = frame_index;
  index++;
  int qi = index * (H_PERIOD / NUM_FRAMES);
  int y = 1;
  int i = 0;
  int count_slack = 0;
  while((i = qi + (y*(H_PERIOD/NUM_FRAMES))) <= (qi + SP_DLINE)){
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

bool sp_task_request(){
  // Check if another sporadic thread is already in execution
  if(excstate_get_state(&executive_sp_frame_desc.excstate) != IDLE){
    printf("[SP] [ALREADY SCHEDULED] can't schedule new sporadic job\n");
    return false;
  }

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
  //exit(1);
  return false;
}


void *frame_handler(void *arg){
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

void executive_sp_check_deadline(){
  // update counter of frame to wait
  int state = excstate_get_state(&executive_sp_frame_desc.excstate);

  if(state != IDLE){
    //get which frame we are executing
    int frames = excstate_get_state(&executive_sp_count_frame);
    // set how many frame to wait before test deadline of sp task
    excstate_set_state(&executive_sp_count_frame, --frames);
    printf("[SP] [WAIT] [%d FRAMES]\n", excstate_get_state(&executive_sp_count_frame));

    // test if execute should check deadline of sp task
    if(frames < 0){
      printf("[SP] [DEADLINE CHECK]\n");
      if(state == WORKING){
        printf("[SP] [ERROR] SP not respect deadline!\n");
        exit(1);
      }

      // set state IDLE
      excstate_set_state(&executive_sp_frame_desc.excstate, IDLE);
    }
  }
}

void *sp_task_handler(){
  while(1){
    // set state COMPLETED
    excstate_set_state(&executive_sp_frame_desc.excstate, COMPLETED);

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

void *executive(/*...*/){
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

    // wakeup next frame
    printf("[FRAME %d] [PENDING]!\n", index);
    excstate_set_state(&frame_descs[index].excstate, PENDING);

    // wakeup sporadic thread
    if(excstate_get_state(&executive_sp_frame_desc.excstate) == READY){
      printf("[SP] [PENDING] sp thread!\n");
      excstate_set_state(&executive_sp_frame_desc.excstate, PENDING);
    }
    
    // wait frame finish to compute
    //executive_wait_next_frame();
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
  p.sched_priority = priority; //sched_get_priority_max(SCHED_FIFO) - i - 1;
  // set scheduler Sched Param
  if(pthread_attr_setschedparam(attr, &p)){
    perror("error init attr - set param\n");
    exit(1);
  }
}

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
  frame_descs = (frame_descriptor *)malloc(sizeof(frame_descriptor) * NUM_FRAMES);//NewList();

  // Init pthreads
  int i = 1;
  for(i=0; i<NUM_FRAMES; i++){
    // [new task descriptor]
    frame_descriptor *fd = &frame_descs[i];// = (frame_descriptor *)malloc(sizeof(frame_descriptor));

    // [init excstate]
    excstate_init(&fd->excstate, IDLE);

    // [set attr]
    pthread_attr_t attr;
    executive_new_pthread_attr(&attr, sched_get_priority_max(SCHED_FIFO) - i - 1);

    // set index
    fd->index = i;

    // [create task]
    pthread_create( &fd->pchild, &attr, frame_handler, &fd->index);
  }

  // [init sporadic thread]

  // init sporadic thread state
  excstate_init(&executive_sp_frame_desc.excstate, IDLE);

  // set attr thread sp task handler
  printf("[SP] [TASK INIT] priority %d\n", sched_get_priority_max(SCHED_FIFO) - i - 1);
  pthread_attr_t attr_sp;
  executive_new_pthread_attr(&attr_sp, sched_get_priority_max(SCHED_FIFO) - i - 1);

  // create task
  pthread_create(&executive_sp_frame_desc.pchild, &attr_sp, &sp_task_handler, NULL);

  // set attr thread executive task handler
  printf("[EXECUTIVE] [TASK INIT] priority %d\n", sched_get_priority_max(SCHED_FIFO) - 1);
  pthread_attr_t attr_exec;
  executive_new_pthread_attr(&attr_exec, sched_get_priority_max(SCHED_FIFO) - 1);

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
