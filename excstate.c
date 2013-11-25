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
