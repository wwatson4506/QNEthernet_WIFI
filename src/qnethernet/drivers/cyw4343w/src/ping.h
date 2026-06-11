// ping.h

#ifndef PING_H
#define PING_H

#include "event.h"

typedef struct {
    IPADDR dip;
    uint8_t   ttl = QNETHERNET_DEFAULT_PING_TTL;  /* Time to live */
    uint16_t   ident = QNETHERNET_DEFAULT_PING_ID; /* Identifier */
    uint16_t   seq = 0;                            /* Sequence number */
	const uint8_t* data = nullptr;
    size_t dataSize     = 0;
} PING_DATA;

class Ping {
public:
  void initPing(void);
  bool cyw43_ping(PING_DATA &req, int count);
};
#endif // PING_H
