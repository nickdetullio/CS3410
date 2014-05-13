#include "mutex.h"

#define RING_BUFFER 0
#define QUEUE 1
#define STATS 2

void mutex_lock(int data_struct) {
   __asm__ __volatile__ (
        "MOV %r1, #0x1 /n"
        "1: LDREX %r0, %[data_struct]  /n"
        "CMP %r0, #0 /n"
        "STREXEQ %r0, %r1, %[data_struct] /n"  
        "CMPEQ r0, #0  /n"    
        "BNE 1b /n"           );
}

void mutex_unlock(int data_struct) {
   __asm__ __volatile__ ("MOV %data_struct, #0x0");
}