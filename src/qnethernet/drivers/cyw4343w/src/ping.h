// ping.h

#ifndef PING_H
#define PING_H

#include "event.h"
#include <lwip/etharp.h>

int get_gw_mac(struct netif *netif);
void initPing(void);
bool cyw43_ping(IPADDR pingIP, BYTE *ping_data, int count);

#endif // PING_H
