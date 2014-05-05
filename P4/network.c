#include "kernel.h"

#define RING_SIZE 8
#define BUFFER_SIZE 1024

volatile struct dev_net *dev_net;

void network_init()
{
  /* Find out where network region is in memory. */
  for (int i = 0; i < 16; i++) {
    if (bootparams->devtable[i].type == DEV_TYPE_NETWORK) {
      puts("Detected network...");
      // find a virtual address that maps to this network region
      dev_net = physical_to_virtual(bootparams->devtable[i].start);
      // allocate ring buffer for receiving packets
      struct dma_ring_slot* ring = (struct dma_ring_slot*)malloc(sizeof(struct dma_ring_slot)*RING_SIZE);
      dev_net->rx_base = virtual_to_physical(ring);
      dev_net->rx_capacity = RING_SIZE;
      dev_net->rx_head = 0;
      dev_net->rx_tail = 0;
      for (int i = 0; i < RING_SIZE; ++i)
      {
        void* space = malloc(BUFFER_SIZE);
        ring[i].dma_base = virtual_to_physical(ring + i*BUFFER_SIZE);
        ring[i].dma_len = BUFFER_SIZE;
      }
      // also allow keyboard interrupts
      set_cpu_status(current_cpu_status() | (1 << (8+INTR_NETWORK)));
      puts("...keyboard driver is ready.");
      return;
    }
  }
}

void network_start_receive()
{
  // turn on card
  dev_net->cmd = NET_SET_POWER;
  dev_net->data = 1;
  // start receiving packets
  dev_net->cmd = NET_SET_POWER;
  dev_net->data = 1;

}

void network_poll()
{
  while(rx_head == rx_tail) {}
  printf("I got a packet!");
}