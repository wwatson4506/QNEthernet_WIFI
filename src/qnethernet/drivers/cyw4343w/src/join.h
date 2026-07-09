// join.h

#ifndef JOIN_H
#define JOIN_H

#include "event.h"
#include "WIFI_init.h"

// Flags for EVENT_INFO link state
#define LINK_UP_OK          0x01
#define LINK_AUTH_OK        0x02
#define LINK_OK            (LINK_UP_OK+LINK_AUTH_OK)
#define LINK_FAIL           0x04

#define JOIN_IDLE           0
#define JOIN_JOINING        1
#define JOIN_OK             2
#define JOIN_FAIL           3

#define JOIN_TRY_USEC       10000000
#define JOIN_RETRY_USEC     10000000

class Join {
public:
  bool join_start(const char *ssID, const char *passphrase, int security);
  bool join_stop(void);
  bool join_restart(const char *ssid, const char *passwd, int security);
  static int join_event_handler(EVENT_INFO *eip);
  void join_state_poll(const char *ssid, const char *passwd, int security);
  int link_check(void);
  int join_check(void);
  int ip_event_handler(EVENT_INFO *eip);

private:
  wl_country_t country_struct = {.country_abbrev=COUNTRY, .rev=COUNTRY_REV, .ccode=COUNTRY};
  const uint8_t mcast_addr[10*6] = {0x01,0x00,0x00,0x00,0x01,0x00,0x5E,0x00,0x00,0xFB};

};
// EOF
#endif
