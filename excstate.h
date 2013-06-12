#ifndef BARRIER_H
#define BARRIER_H

#include <pthread.h>

typedef struct{
  pthread_mutex_t mutex;
  pthread_cond_t cond;

  unsigned int SIZE;
  unsigned int count;
}excstate;

typedef enum frame_state{ IDLE, WORKING, PENDING, READY, COMPLETED } frame_state;

void excstate_init(excstate * b, int state);
void excstate_destroy(excstate * b);

void excstate_wait_running();

void excstate_set_state(excstate *b, int state);
int excstate_get_state(excstate *b);

#endif
