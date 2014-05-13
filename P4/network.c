#include "kernel.h"
#include "network.h"

#define RING_SIZE 16
#define BUFFER_SIZE 4000

volatile struct dev_net *dev_net;
struct dma_ring_slot* ring;
int packets_so_far = 0;
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
      // also allow keyboard interrupts
      set_cpu_status(current_cpu_status() | (1 << (8+INTR_NETWORK)));
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
  //while(rx_head == rx_tail) {}
  while(1){
    puts("I got a packet!");
    if (dev_net->rx_tail != dev_net->rx_head){
      // get the address of the packet
      unsigned int ring_num = (dev_net->rx_tail % dev_net->rx_capacity);
      
      // get its physical address
      packet = (struct honeypot_command_packet *)ring[ring_num].dma_base;
     
      // convert to a virtual address so we can reference it
      packet = (struct honeypot_command_packet *)physical_to_virtual((int *)packet);
     
      // store packet statistics
      packets_so_far++;
      dev_net->cmd = NET_GET_DROPCOUNT;
      drop_count += dev_net->data;
      dev_net->rx_tail++;
      printf("Packets so far:%d\tDropped packets:%d\n", packets_so_far, drop_count);
    }
  }
  
}
