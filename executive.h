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
