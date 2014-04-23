#include "cacheemall.h"

void __start(int core_id, int num_crashes, unsigned char payload) {

  if (core_id == 3) {
    while (1) {
      int i;
      for (i = 0; i < TAUNT_SIZE; i++) {
        if (HOME_STATUS->taunt[i] >= 0) {
	      Call_Jenny(HOME_STATUS->taunt[i]);
        }
      }
    }
  } 
  if (core_id == 2) {
    while (1) {
      int i = 0
      while (i < 30) {
        // write snorlax
      }
      int i = 0
      while (i < 30) {
        // write thunderbolt
      }
    }
  }


  else {
    int *ptr = (int *) (HOME_DATA_SEGMENT); // start on even cache line
    // using an int pointer to write 4x as quickly
    int payload_int = payload << 3 | payload << 2 | payload << 1 | payload;

    // each core fills 1/2 of the cache line, spaced out evenly
    // must divide by four when incrementing by ptr due to increased size
    ptr += core_id * ((CACHE_LINE / 2) / 4);

    while (1) {
      int i;
      for (i = 0; i < CACHE_LINE/8; i++) {
        ptr[i] = payload_int;
      }
      ptr += (2*CACHE_LINE)/4; // only fill even numbered line
    }
  }
}
