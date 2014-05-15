#ifndef MUTEX_H_
#define MUTEX_H_

void mutex_lock(int *addr);

void mutex_unlock(int *addr);

int statistics;

int arraylists;

int printlock;

int queuel;

int ringbuffer;

#endif

