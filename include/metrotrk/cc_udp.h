#ifndef TRK_CC_UDP_H
#define TRK_CC_UDP_H

#include "revolution/os/OSInterrupt.h"
#include "revolution/types.h"

int udp_cc_initialize(void* flagOut, OSInterruptHandler handler);
int udp_cc_shutdown();
int udp_cc_open();
int udp_cc_close();
int udp_cc_read(u8* dest, int size);
int udp_cc_write(const u8* src, int size);
int udp_cc_pre_continue();
int udp_cc_post_stop();
int udp_cc_peek();
int udp_cc_initinterrupts();

#endif
