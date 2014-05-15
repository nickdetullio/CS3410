#include "network.h"

#define RING_SIZE 16
#define QUEUE_SIZE 64
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

struct stats_queue {
  unsigned int rx_base;
  unsigned int rx_capacity;
  unsigned int rx_head;
  unsigned int rx_tail;
};

volatile struct dev_net *dev_net;
struct dma_ring_slot* ring; 
struct stats_queue* stats_queue;
struct dma_ring_slot* queue;
struct arraylist* spammers; //source addresses of the spammers
struct arraylist* spammer_count; //how many packets of the source addresses arrived
struct arraylist* evils; //fingerprint values
struct arraylist* evil_count; //how many pacets with that fingerprint arrived
struct arraylist* vulnerables; //destination ports of the spammers
struct arraylist* vulnerable_count; //how many packets of destination ports arrived

int packets_so_far = 0; 
int packets_interval = 0;
int bytes_interval = 0;
double num_seconds = 0.0;
double transfer_rate = 0.0;

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

      // allocate ring buffer to use as queue for handling statistics
      queue = (struct dma_ring_slot*)malloc(sizeof(struct dma_ring_slot)*QUEUE_SIZE);
      stats_queue = (struct stats_queue*)malloc(sizeof(struct stats_queue));
      stats_queue->rx_base = virtual_to_physical(queue);
      stats_queue->rx_capacity = QUEUE_SIZE;
      stats_queue->rx_head = 0;
      stats_queue->rx_tail = 0;
      for (int i = 0; i < QUEUE_SIZE; ++i)
      {
        void* space = malloc(BUFFER_SIZE);
        queue[i].dma_base = virtual_to_physical(space);
        queue[i].dma_len = BUFFER_SIZE;
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

void handle_packet()
{
  // QUEUE MUTEX LOCK
  mutex_lock(&queuel);
  // get its physical address
  if (stats_queue->rx_tail != stats_queue->rx_head){
    // get the address of the packet
    unsigned int queue_num = (stats_queue->rx_tail % stats_queue->rx_capacity);
    struct honeypot_command_packet* packet;
    packet = (struct honeypot_command_packet *)queue[queue_num].dma_base;
 
    // convert to a virtual address so we can reference it
    packet = (struct honeypot_command_packet *)physical_to_virtual((int)packet);
  
  // HONEYPOT MUTEX LOCK
  mutex_lock(&arraylists);
  if (packet->secret_big_endian == to_little_endian(HONEYPOT_SECRET)) {
    if (packet->cmd_big_endian == to_little_endian(HONEYPOT_ADD_SPAMMER)) {
      //printf("COMMAND: add spammer");
      arraylist_add(spammers, packet->data_big_endian);
      arraylist_add(spammer_count, 0);
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_ADD_EVIL)){
      //printf("COMMAND: add evil");
      arraylist_add(evils, packet->data_big_endian);
      arraylist_add(evil_count, 0);
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_ADD_VULNERABLE)) {
      //printf("COMMAND: add vulnerable");
      arraylist_add(vulnerables, packet->data_big_endian);
      arraylist_add(vulnerable_count, 0);
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_DEL_SPAMMER)) {
      //printf("COMMAND: delete spammer");
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
      //printf("COMMAND: delete evil");
      for (int i = 0; i < evils->length; ++i)
      {
        if (arraylist_get(evils, i) == packet->data_big_endian)
        {
          arraylist_remove(evils, i);
          arraylist_remove(evil_count, i);
        }
      }
    }
    else if (packet->cmd_big_endian == to_little_endian(HONEYPOT_DEL_VULNERABLE)) {
      //printf("COMMAND: delete vulnerable");
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
    {
      //printf("COMMAND: print statistics");
      print_stats();
    }    
  }
  else {
    //printf("Other");
    int fingerprint = hash((unsigned char*)packet, queue[queue_num].dma_len);
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
}
//print_stats();
  // HONEYPOT MUTEX UNLOCK
  mutex_unlock(&arraylists);
// QUEUE MUTEX UNLOCK
mutex_unlock(&queuel);
}

void network_poll()
{
  while(1) {
    // RING BUFFER MUTEX LOCK
    mutex_lock(&ringbuffer);
    if (dev_net->rx_tail != dev_net->rx_head){
      // get the address of the packet
      unsigned int ring_num = (dev_net->rx_tail % dev_net->rx_capacity);
/*
      struct honeypot_command_packet* temp_packet;
      temp_packet = (struct honeypot_command_packet *)ring[ring_num].dma_base;
 
      // convert to a virtual address so we can reference it
      temp_packet = (struct honeypot_command_packet *)physical_to_virtual((int)temp_packet);
      struct honeypot_command_packet* packet;
      packet = (struct honeypot_command_packet *)malloc(HONEYPOT_CMD_PKT_MIN_LEN);
      packet->headers.ip_version = temp_packet->headers.ip_version;
      packet->headers.ip_qos = temp_packet->headers.ip_qos;
      packet->headers.ip_len = temp_packet->headers.ip_len;
      packet->headers.ip_id = temp_packet->headers.ip_id;
      packet->headers.ip_flags = temp_packet->headers.ip_flags;
      packet->headers.ip_ttl = temp_packet->headers.ip_ttl;
      packet->headers.ip_protocol = temp_packet->headers.ip_protocol;
      packet->headers.ip_checksum = temp_packet->headers.ip_checksum;
      packet->headers.ip_source_address_big_endian = temp_packet->headers.ip_source_address_big_endian;
      packet->headers.ip_dest_address_big_endian = temp_packet->headers.ip_dest_address_big_endian;
      packet->headers.udp_source_port_big_endian = temp_packet->headers.udp_source_port_big_endian;
      packet->headers.udp_dest_port_big_endian = temp_packet->headers.udp_dest_port_big_endian;
      packet->headers.udp_len = temp_packet->headers.udp_len;
      packet->headers.udp_checksum = temp_packet->headers.udp_checksum;
      packet->secret_big_endian = temp_packet->secret_big_endian;
      packet->cmd_big_endian = temp_packet->cmd_big_endian;
      packet->data_big_endian = temp_packet->data_big_endian; */
      // enqueue this packet
      if (stats_queue->rx_tail != (stats_queue->rx_head-1))
      {
        queue[stats_queue->rx_head].dma_base = ring[ring_num].dma_base;//virtual_to_physical(packet);
        queue[stats_queue->rx_head].dma_len = ring[ring_num].dma_len;
        stats_queue->rx_head = (stats_queue->rx_head+1) % stats_queue->rx_capacity;
      }

       // STATISTICS MUTEX LOCK
      // store packet statistics
      mutex_lock(&statistics);
      packets_so_far++;
      packets_interval++;
      bytes_interval += ring[ring_num].dma_len;
      double time_since_boot = (double) current_cpu_cycles() / CPU_CYCLES_PER_SECOND;
      if (time_since_boot - num_seconds > 1.0)
      {
        int mbits = (4*bytes_interval) / 1000000;
        transfer_rate = mbits / (time_since_boot - num_seconds);
        packets_interval = 0;
        bytes_interval = 0;
      }
      // STATISTICS MUTEX UNLOCK
      mutex_unlock(&statistics);
      
      //printf("Packets so far:%d\tDropped packets:%d\tTransfer rate:%f\n", 
      //  packets_so_far, drop_count, transfer_rate);

      ring[ring_num].dma_len = BUFFER_SIZE;
      dev_net->rx_tail = (dev_net->rx_tail+1) % dev_net->rx_capacity;
      // RING BUFFER MUTEX UNLOCK
      mutex_unlock(&ringbuffer);
      handle_packet();
    }
  }
}

unsigned short to_little_endian(unsigned short x)
{
  unsigned short valueShifted = (x>>8) | (x<<8);
  return valueShifted;
}

void print_stats()
{
  double time_since_boot = (double) current_cpu_cycles() / CPU_CYCLES_PER_SECOND;
  dev_net->cmd = NET_GET_DROPCOUNT;
  int drop_count = dev_net->data;
  double drop_rate = (drop_count*NET_MINPKT*4) / (1000000*time_since_boot);
  mutex_lock(&printlock);
  mutex_lock(&statistics);
  printf("time since boot: %d cycles (%f sec) packets: %d recent: %d (%f Mbits/s) drops: %d (%f Mbits/s)",
    current_cpu_cycles(), time_since_boot, packets_so_far, packets_interval, transfer_rate, drop_count, drop_rate);
  printf("Packets so far:%d\tDropped packets:%d\tTransfer rate:%f\n", 
    packets_so_far, drop_count, transfer_rate);
  mutex_unlock(&statistics);
  mutex_lock(&arraylists);
  printf("Num spammers:%d\tNum evils:%d\tNum vulnerables:%d\n", spammers->length, evils->length, vulnerables->length);
  mutex_unlock(&arraylists);
  mutex_unlock(&printlock);
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
