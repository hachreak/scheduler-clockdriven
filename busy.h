#ifndef BUSY_H
#define BUSY_H

/**
 * Busy wait
 * @param millisec how many millisec have to compute
 */
void busy_wait(unsigned int millisec);

/**
 * Calibration for the busywait
 */
void busy_calib();

#endif
