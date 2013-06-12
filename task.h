#ifndef TASK_H
#define TASK_H

#include <stdbool.h>

/* Puntatore a funzione per una task-routine */
typedef void (* task_routine)();

/* Puntatori a funzione alle routine dei task periodici da mettere in esecuzione. */
extern task_routine P_TASKS[];

/* Puntatore a funzione alla routine del task sporadico da mettere in esecuzione. */
extern task_routine SP_TASK;

/* Numero di task registrati (lunghezza dell'array P_TASKS) */
extern const unsigned int NUM_P_TASKS;

/* Lunghezza dell'iperperiodo (in quanti temporali da 10ms) */
extern const unsigned int H_PERIOD;

/* Numero di frames in un iperperiodo (deve essere un divisore di H_PERIOD) */
extern const unsigned int NUM_FRAMES;

/* Lo schedule, cioe' la lista dei task periodici da eseguire nei singoli frame. */
extern int * SCHEDULE[];

/* Lo slack time (in quanti temporali da 10ms) presente nei singoli frame */
extern int SLACK[];

/* Il tempo di esecuzione del task sporadico (in quanti temporali da 10ms) */
extern int SP_WCET;

/* La deadline del task sporadico (in quanti temporali da 10ms) */
extern int SP_DLINE;

/* Inizializzazione delle strutture dati P_TASKS[], SP_TASK, SCHEDULE[] e SLACK */
void task_init();

/* Finalizzazione delle strutture dati */
void task_destroy();


/* Richiede il rilascio del task aperiodico, ritorna true se in task viene accettato */
bool sp_task_request();


/* Nota sulla lista SCHEDULE:

   Le routine dei task sono indicate in P_TASKS, quindi possiamo assegnare un id
   ad ogni task a partire da zero riferendoci a questa lista: T0, T1, ...

   Ogni SCHEDULE[i] punta alla lista dei task da eseguire in sequenza nel frame
   i-esimo, dove i=0..NUM_FRAMES-1. Ogni valore intero della lista rappresenta
   i'id di un task da eseguire. La lista termina con un valore pari a -1.

   Esempio: SCHEDULE[3] = { 0,3,4,-1}
     (nel frame 3 devono essere eseguiti in sequenza P_TASKS[0],P_TASKS[3],P_TASKS[4])
*/
#endif
