#include "mutex.h"

#define RING_BUFFER 1
#define QUEUE 2
#define STATS 3

void mutex_lock(int data_struct) {
   asm (
        "try: LI $t1, 1            \n"
        "LL $t0, 0($data_struct)     \n" 
        "BNEZ $t0, try             \n"
        "SC $t1, 0($data_struct)     \n"
        "BEQZ $1, try              \n");
}

void mutex_unlock(int data_struct) {
   asm ("SW $zero, 0($data_struct)");
}