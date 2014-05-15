#include "network.h"

#define RING_SIZE 16
#define BUFFER_SIZE 4000
#define NULL 0

typedef struct arraylist {
    int *buffer;     // pointer to allocated memory
    int buffer_size; // the max number of integers the buffer can hold
    int length;      // the number of integers stored in the list
} arraylist;



////////////////////////////////////////////////////
//Functions that you need to implement:

void arraylist_add(arraylist *a, int x) {
    // Appending the value x to the end of the arraylist
    if (a->buffer == NULL) {
        a->buffer_size = 2;
        a->buffer = (int *)malloc(a->buffer_size*sizeof(int));
    }
    if (a->buffer_size == a->length) {
        a->buffer_size = a->buffer_size*2;
        int* temp = (int *)malloc(a->buffer_size*sizeof(int));
        for (int i = 0; i < a->length; ++i)
            temp[i] = a->buffer[i];
        a->buffer = temp;
    }
    a->buffer[a->length] = x;
    ++a->length;
}

void arraylist_insert(arraylist *a, int index, int x) {
    // Storing the value x at the specified index in the arraylist.
    // Previously stored values should be moved back rather than overwritten.
    
    // if index out of bounds, just return
    if (index > a->length)
        return;

    // take care of allocation/reallocation of memory
    if (a->buffer == NULL) {
        a->buffer_size = 2;
        a->buffer = (int *)malloc(a->buffer_size*sizeof(int));
    }
    if (a->buffer_size == a->length) {
        a->buffer_size = a->buffer_size*2;
        int* temp = (int *)malloc(a->buffer_size*sizeof(int));
        for (int i = 0; i < a->length; ++i)
            temp[i] = a->buffer[i];
        a->buffer = temp;
    }

    // move all elements to make space for insertion
    int i,temp,val;
    val = a->buffer[index];
    for(i = (index+1); i <= a->length; i++) {
        temp = a->buffer[i];
        a->buffer[i] = val;
        val = temp;
    }

    // insert element
    a->buffer[index] = x;
    ++a->length;    
}

void arraylist_free(arraylist *a) {
    // freeing any memory used by that arraylist
    if (a->buffer != NULL)
        free(a->buffer);
    free(a);
}

////////////////////////////////////////////////////



arraylist* arraylist_new() {
    arraylist *a = (arraylist *)malloc(sizeof(arraylist));
    a->buffer = NULL;
    a->buffer_size = 0;
    a->length = 0;

    return a;
}

void arraylist_remove(arraylist *a, int index) {
    int i;
    for(i = index; i < a->length-1; i++)
        a->buffer[i] = a->buffer[i+1];
    
    --a->length; 
}

int arraylist_get(arraylist *a, int index) {
    return(a->buffer[index]); 
}

void arraylist_print(arraylist *a) {
    printf("[");
    if (a->length > 0) {
        int i;
        for(i = 0; i < a->length-1; i++)
            printf("%d, ",arraylist_get(a,i));
        printf("%d", arraylist_get(a,a->length-1));
    }

    printf("]\n");
}

int main(int argc, char *argv[]) {
    // START OF TEST
    int i;
    arraylist *a = arraylist_new();

    arraylist_add(a, 0);
    arraylist_add(a, 1);
    arraylist_add(a, 2);
    arraylist_print(a);
    
    for (i = 0; i < a->length + 1; i++) {
        arraylist_insert(a, i, 100);
        printf("Insert position %d: ", i);
        arraylist_print(a);
        arraylist_remove(a, i);
    }
    printf("Clean: ");
    arraylist_print(a);

    arraylist_free(a);
    // END OF TEST

    return 0;
}


volatile struct dev_net *dev_net;
struct dma_ring_slot* ring; 
struct arraylist* spammers; //source addresses of the spammers
struct arraylist* spammer_count; //how many packets of the source addresses arrived
struct arraylist* evils; //fingerprint values
struct arraylist* evil_count; //how many pacets with that fingerprint arrived
struct arraylist* vulnerables; //destination ports of the spammers
struct arraylist* vulnerable_count; //how many packets of destination ports arrived

int packets_so_far = 0; 
int packets_interval = 0;
double num_seconds = 0.0;
double transfer_rate = 0.0;
int drop_count = 0;

void network_init()
{
  /* Find out where network region is in memory. */
  for (int i = 0; i < 16; i++) {
    if (bootparams->devtable[i].type == DEV_TYPE_NETWORK) {
      puts("Detected network...");
      // find a virtual address that maps to this network region
      dev_net = physical_to_virtual(bootparams->devtable[i].start);
    }
  }
      // allocate ring buffer for receiving packets
      ring = (struct dma_ring_slot*)malloc(sizeof(struct dma_ring_slot)*RING_SIZE);
      dev_net->rx_base = virtual_to_physical(ring);
      dev_net->rx_capacity = RING_SIZE;
      dev_net->rx_head = 0;
      dev_net->rx_tail = 0;
      for (int i = 0; i < RING_SIZE; ++i)
      {
        void* space = malloc(BUFFER_SIZE);
        ring[i].dma_base = virtual_to_physical(space);
        ring[i].dma_len = BUFFER_SIZE;
      }
      // initialize honeypot data structures
      spammers = arraylist_new();
      spammer_count = arraylist_new();
      evils = arraylist_new();
      evil_count = arraylist_new();
      vulnerables = arraylist_new();
      vulnerable_count = arraylist_new();
      puts("...network driver is ready.");
      return;
}

void network_start_receive()
{
  // turn on card
  dev_net->cmd = NET_SET_RECEIVE;
  dev_net->data = 1;
  // start receiving packets
  dev_net->cmd = NET_SET_POWER;
  dev_net->data = 1;

}

void network_trap()
{
  while(1) {
    //puts("Got a packet, %d cycles", current_cpu_cycles());
  }
}

void network_poll()
{
  while(1) {
    if (dev_net->rx_tail != dev_net->rx_head){
      // get the address of the packet
      unsigned int ring_num = (dev_net->rx_tail % dev_net->rx_capacity);
      
      // get its physical address
      struct honeypot_command_packet* packet;
      packet = (struct honeypot_command_packet *)ring[ring_num].dma_base;
     
      // convert to a virtual address so we can reference it
      packet = (struct honeypot_command_packet *)physical_to_virtual((int)packet);
     
      // TODO: enqueue this packet 

      // store packet statistics
      packets_so_far++;
      packets_interval++;
      dev_net->cmd = NET_GET_DROPCOUNT;
      drop_count += dev_net->data;
      double time_since_boot = (double) current_cpu_cycles() / CPU_CYCLES_PER_SECOND;
      if (time_since_boot - num_seconds > 10.0)
      {
        transfer_rate = packets_interval / (time_since_boot - num_seconds);
        packets_interval = 0;
      }
      
      printf("Packets so far:%d\tDropped packets:%d\tTransfer rate:%f\n", 
        packets_so_far, drop_count, transfer_rate);

 if (packet->secret_big_endian == 0x1034) {
    if (packet->cmd_big_endian == to_little_endian(HONEYPOT_ADD_SPAMMER)) {
      arraylist_add(spammers, packet->data_big_endian);
      arraylist_add(spammer_count, 0);
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_ADD_EVIL)){
      int fingerprint = hash((unsigned char*)packet, ring[ring_num].dma_len);
      arraylist_add(evils, fingerprint);
      arraylist_add(evil_count, 0);
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_ADD_VULNERABLE)){
      arraylist_add(vulnerables, packet->data_big_endian);
      arraylist_add(vulnerable_count, 0);
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_DEL_SPAMMER)) {
      for (int i = 0; i < spammers->length; ++i)
      {
        if (arraylist_get(spammers, i) == packet->data_big_endian)
        {
          arraylist_remove(spammers, i);
          arraylist_remove(spammer_count, i);
        }
      }
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_DEL_EVIL)) {
      int fingerprint = hash((unsigned char*)packet, ring[ring_num].dma_len);
      for (int i = 0; i < evils->length; ++i)
      {
        if (arraylist_get(evils, i) == fingerprint)
        {
          arraylist_remove(evils, i);
          arraylist_remove(evil_count, i);
        }
      }
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_DEL_VULNERABLE)) {
      for (int i = 0; i < vulnerables->length; ++i)
      {
        if (arraylist_get(vulnerables, i) == packet->data_big_endian)
        {
          arraylist_remove(vulnerables, i);
          arraylist_remove(vulnerable_count, i);
        }
      }
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_PRINT))
      print_stats();
  }
  else {
    int fingerprint = hash((unsigned char*)packet, ring[ring_num].dma_len);
    // check if matches 
    for (int i = 0; i < spammers->length; ++i)
    {
      if (arraylist_get(spammers, i) == packet->data_big_endian)
      {
        int count = arraylist_get(spammer_count, i);
        arraylist_remove(spammer_count, i);
        ++count;
        arraylist_insert(spammer_count, i, count);
      }
    }
    for (int i = 0; i < evils->length; ++i)
    {
      if (arraylist_get(evils, i) == fingerprint)
      {
        int count = arraylist_get(evil_count, i);
        arraylist_remove(evil_count, i);
        ++count;
        arraylist_insert(evil_count, i, count);
      }
    }
    for (int i = 0; i < vulnerables->length; ++i)
    {
      if (arraylist_get(vulnerables, i) == packet->data_big_endian)
      {
        int count = arraylist_get(vulnerable_count, i);
        arraylist_remove(vulnerable_count, i);
        ++count;
        arraylist_insert(vulnerable_count, i, count);
      }
    }
  }

      ring[ring_num].dma_len = BUFFER_SIZE;
      dev_net->rx_tail++;
    }
  }
}
 /*
void handle_packet()
{
  // TODO
 
  //mutex.lock();
  packets_so_far++;
  dev_net->cmd = NET_GET_DROPCOUNT;
  drop_count += dev_net->data;
  
  struct honeypot_command_packet packet* = arraylist.get()
  // check if command packet
  if (packet->secret_big_endian == 0x1034) {
    if (packet->cmd_big_endian = to_little_endian(HONEYPOT_ADD_SPAMMER)) {
      arraylist_add(spammers, packet->data_big_endian);
      arraylist_add(spammer_count, 0);
    }
    else if (packet->cmd_big_endian = to_little_endian(HONEYPOT_ADD_EVIL)){
      int fingerprint = hash(packet);
      arraylist_add(evils, fingerprint);
      arraylist_add(evil_count, 0);
    }
    else if (packet->cmd_big_endian = to_little_endian(HONEYPOT_ADD_VULNERABLE)){
      arraylist_add(vulnerables, packet->data_big_endian);
      arraylist_add(vulnerable_count, 0);
    }
    else if (packet->cmd_big_endian = to_little_endian(HONEYPOT_DEL_SPAMMER)) {
      for (int i = 0; i < spammers->length; ++i)
      {
        if (arraylist_get(spammers, i) == packet->data_big_endian)
        {
          arraylist_remove(spammers, i);
          arraylist_remove(spammer_count, i);
        }
      }
    }
    else if (packet->cmd_big_endian = to_little_endian(HONEYPOT_DEL_EVIL)) {
      int fingerprint = hash(packet);
      for (int i = 0; i < evils->length; ++i)
      {
        if (arraylist_get(evils, i) == fingerprint)
        {
          arraylist_remove(evils, i);
          arraylist_remove(evil_count, i);
        }
      }
    else if (packet->cmd_big_endian = to_little_endian(HONEYPOT_DEL_VULNERABLE)) {
      for (int i = 0; i < vulnerables->length; ++i)
      {
        if (arraylist_get(vulnerables, i) == packet->data_big_endian)
        {
          arraylist_remove(vulnerables, i);
          arraylist_remove(vulnerable_count, i);
        }
      }
    //else if (packet->cmd_big_endian = to_little_endian(HONEYPOT_PRINT))
      //print_stats();
  }
  else {
    int fingerprint = hash(packet);
    // check if matches 
    for (int i = 0; i < spammers->length; ++i)
    {
      if (arraylist_get(spammers, i) == packet->data_big_endian)
      {
        arraylist_get(spammer_count, i)++;
      }
    }
    for (int i = 0; i < evils->length; ++i)
    {
      if (arraylist_get(evils, i) == fingerprint)
      {
        arraylist_get(evil_count, i)++;
      }
    }
    for (int i = 0; i < vulnerables->length; ++i)
    {
      if (arraylist_get(vulnerables, i) == packet->data_big_endian)
      {
        arraylist_get(vulnerable_count, i)++;
      }
    }
  }
  */

unsigned short to_little_endian(unsigned short x)
{
  unsigned short valueShifted = (x>>8) | (x<<8);
  return valueShifted;
}

void print_stats()
{
  //printf("Packets so far:%d\tDropped packets:%d\tTransfer rate:%f\n", 
  //  packets_so_far, drop_count, transfer_rate);
  printf("Num spammers:%d\tNum evils:%d\tNum vulnerables:%d\n", spammers->length, evils->length, vulnerables->length);
}

unsigned long hash(unsigned char *pkt, int n)
{
  unsigned long hash = 5381;
  int i = 0;
  while (i < n-8) {
    hash = hash * 33 + pkt[i++];
    hash = hash * 33 + pkt[i++];
    hash = hash * 33 + pkt[i++];
    hash = hash * 33 + pkt[i++];
    hash = hash * 33 + pkt[i++];
    hash = hash * 33 + pkt[i++];
    hash = hash * 33 + pkt[i++];
    hash = hash * 33 + pkt[i++];
  }
  while (i < n)
    hash = hash * 33 + pkt[i++];
  return hash;
}

/*
      if (packet->secret_big_endian == 0x1034) {
        if (packet->cmd_big_endian == to_little_endian(HONEYPOT_ADD_SPAMMER)) {
          printf("COMMAND: add spammer %d\n", packet->data_big_endian);
        }
        else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_ADD_EVIL)){
          int fingerprint = hash((unsigned char*)packet, ring[ring_num].dma_len);
          printf("COMMAND: add evil %d\n", fingerprint);
        }
        else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_ADD_VULNERABLE)){
          printf("COMMAND: add vulnerable %d\n", packet->data_big_endian);
        }
        else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_DEL_SPAMMER)) {
          printf("COMMAND: delete spammer %d\n", packet->data_big_endian);
        }
        else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_DEL_EVIL)) {
          int fingerprint = hash((unsigned char*)packet, ring[ring_num].dma_len);
          printf("COMMAND: delete evil %d\n", fingerprint);
        }
        else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_DEL_VULNERABLE)) {
          printf("COMMAND: delete vulnerable %d\n", packet->data_big_endian);
        }
        else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_PRINT))
          printf("COMMAND: print stats\n");
      }
      else {
        printf("Some other packet\n");
      }
*/
