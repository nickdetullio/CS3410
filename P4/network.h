#ifndef NETWORK_H_
#define NETWORK_H_

#include "kernel.h"
#include "honeypot.h"
#include "machine.h"

void network_init();

void network_start_receive();

void network_trap();

void handle_packet();

void network_poll();

unsigned short to_little_endian(unsigned short x);

void print_stats();

unsigned long hash(unsigned char *pkt, int n);

#endif







