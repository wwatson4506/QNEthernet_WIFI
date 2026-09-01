// driver_CYW4343W.h

#pragma once

#define MTU           4018 //1500
#define MAX_FRAME_LEN 4018 // 1518 /* Does not include the 4-byte FCS (frame check sequence) */

#define ETH_PAD_SIZE 0  /* 2 */
#define EVENT_POLL_USEC    100000 //100000

#define LINK_UP 1
#define LINK_DOWN 0
