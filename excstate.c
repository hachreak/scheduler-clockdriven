#include "excstate.h"

void excstate_init( excstate * b, int state){
  pthread_mutex_init( &b->mutex, NULL );
  pthread_cond_init( &b->cond, NULL );
  b->count = state;
}

void excstate_destroy(excstate * b){
  pthread_mutex_destroy( &b->mutex );
  pthread_cond_destroy( &b->cond );
}

void excstate_set_state(excstate *b, int state){
  pthread_mutex_lock(&b->mutex);
  b->count = state;
  pthread_cond_signal( &b->cond );
  pthread_mutex_unlock(&b->mutex);
}

int excstate_get_state(excstate *b){
  pthread_mutex_lock(&b->mutex);
  int state = b->count;
  //pthread_cond_signal( &b->cond );
  pthread_mutex_unlock(&b->mutex);
  return state;
}

void excstate_wait_running( excstate * b ){
  pthread_mutex_lock(&b->mutex);

  while (b->count != PENDING){
    pthread_cond_wait(&b->cond,&b->mutex);
  }

  pthread_cond_signal( &b->cond );
  pthread_mutex_unlock(&b->mutex);
}
