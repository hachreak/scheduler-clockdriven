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
