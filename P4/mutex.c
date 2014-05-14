#include "mutex.h"

#define BUFFER1 1
#define BUFFER2 2
#define STATS 3

void mutex_lock_buffer1() {
   __asm__ __volatile__ (
        "MOV %r1, #0x1 /n"
        "1: LDREX %r0, %[BUFFER1]  /n"
        "CMP %r0, #0 /n"
        "STREXEQ %r0, %r1, %[BUFFER1] /n"  
        "CMPEQ r0, #0  /n"    
        "BNE 1b /n"           );
}

void mutex_unlock_buffer1() {
   __asm__ __volatile__ ("MOV %BUFFER1, #0x0");
}

void mutex_lock_buffer2() {
   __asm__ __volatile__ (
        "MOV %r1, #0x1 /n"
        "1: LDREX %r0, %[BUFFER2]  /n"
        "CMP %r0, #0 /n"
        "STREXEQ %r0, %r1, %[BUFFER2] /n"  
        "CMPEQ r0, #0  /n"    
        "BNE 1b /n"           );
}

void mutex_unlock_buffer2() {
   __asm__ __volatile__ ("MOV %STATS, #0x0");
}

void mutex_lock_stats() {
   __asm__ __volatile__ (
        "MOV %r1, #0x1 /n"
        "1: LDREX %r0, %[STATS] /n"
        "CMP %r0, #0 /n"
        "STREXEQ %r0, %r1, %[STATS] /n"  
        "CMPEQ r0, #0  /n"    
        "BNE 1b /n"           );
}

void mutex_unlock_stats() {
   __asm__ __volatile__ ("MOV %STATS, #0x0");
}