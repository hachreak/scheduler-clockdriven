#ifndef BARRIER_H
#define BARRIER_H

#include <pthread.h>

typedef struct{
  pthread_mutex_t mutex;
  pthread_cond_t cond;

  unsigned int SIZE;
  unsigned int count;
}excstate;

/**
 * task periodic/sporadic state
 */
typedef enum frame_state{ IDLE, WORKING, PENDING, READY, COMPLETED } frame_state;

/**
 * init a state
 * @param b the excstate
 * @param state which state have to set
 */
void excstate_init(excstate * b, int state);

/**
 * destroy a state
 * @param b the excstate
 */
void excstate_destroy(excstate * b);

/**
 * Wait the condition PENDING
 * @param b the excstate
 */
void excstate_wait_running(excstate * b);

/**
 * Set a state
 * @param b the state
 * @param state the new state
 */
void excstate_set_state(excstate *b, int state);

/**
 * Read a state
 * @param b the state
 */
int excstate_get_state(excstate *b);

#endif
