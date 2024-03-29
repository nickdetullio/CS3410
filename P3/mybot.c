#include "cacheemall.h"

void __start(int core_id, int num_crashes, unsigned char payload) {

    if (core_id == 3) {
      unsigned char *opp_ptr = OPPONENT_DATA_SEGMENT;
      int i = 0; 
      int no_snorlaxes = 15;
      int no_thunderbolts = 30; 
      while(1) {
          if (no_snorlaxes > 0) {
            Team_Rocket();
            opp_ptr[i] = SNORLAX;
            retreat();
            no_snorlaxes--;
            i++;
          }
          if (no_thunderbolts > 0) {
            Team_Rocket();
            opp_ptr[i] = THUNDERBOLT;
            retreat();
            no_thunderbolts--;
            i++;
         }
         else { 
            for (i = 0; i < TAUNT_SIZE; i++) {
              if (HOME_STATUS->taunt[i] >= 0) {
                Call_Jenny(HOME_STATUS->taunt[i]);
              }            
            }
         }
         if (no_snorlaxes == 0 && no_thunderbolts == 0) {
           invalidate(opp_ptr);
         }
         else {
           opp_ptr += CACHE_LINE;
           prefetch(opp_ptr);
         }
       }
    }	

    else { 
   
      unsigned int *ptr = (unsigned int *) HOME_DATA_SEGMENT; // start on even cache line
      ptr += core_id * (CACHE_LINE/3)/4;

      while (1) {
        int i;
        for (i = 0; i < CACHE_LINE/12; i++) {
          ptr[i] = payload << 24 | payload << 16 | payload << 8 | payload;
        }
        ptr += (2*CACHE_LINE)/4;
        prefetch(ptr);
      } 
   }
}
