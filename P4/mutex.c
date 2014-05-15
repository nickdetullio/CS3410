#include "mutex.h"
#include "kernel.h"

void mutex_lock(int *addr) {
   asm volatile(
        ".set mips2    \n"
        "try: LI $8, 1 \n"
        "LL $9, 0($4)  \n"
        "bnez $9, try  \n"
        "sc $8, 0($4)  \n"
        "beqz $8, try  \n"
        : "=r" (addr)           );
}

void mutex_unlock(int *addr) {
   asm (
      ".set mips2   \n"
      "SW $0, 0($4) \n"
      : "=r" (addr));
}

int statistics = 0;
int arraylists = 0;
int printlock = 0;
int queuel = 0;
int ringbuffer = 0;

