// ping.cpp

#include "ping.h"
#include "cyw43_T4_SDIO.h"
#include "../src/qnethernet/drivers/cyw4343w/src/secrets.h"
#include "join.h"

using namespace qindesign::network;

#define EVENT_POLL_USEC     500 //200 //100000
#define PING_RESP_USEC      250000
#define PING_INTERVAL       1000000
#define GW_MS_DELAY 50

Event pingEvnt;
extern uint32_t ping_tx_time, ping_rx_time;
MACADDR gw_mac;
uint32_t wait_ticks, ping_poll_ticks, ping_ticks;
int ping_state = 0, t;

void initPing(void) {
  ustimeout(&wait_ticks, 0);
  ustimeout(&ping_poll_ticks, 0);
}	
	
bool cyw43_ping(IPADDR pingIP, BYTE *ping_data, int count) {
  int reps = count;
  if(link_check() != 1) return false; 
  while(1) {
    if(ustimeout(&wait_ticks, PING_INTERVAL)) {
     ustimeout(&ping_ticks, 0);
     pingEvnt.ip_tx_icmp(gw_mac, pingIP, ICREQ, 0, ping_data, sizeof(ping_data));
     ping_rx_time = 0;
     ping_state = 2;
  }
  // Check for timeout on ARP or ICMP request
//  if((ping_state==1 || ping_state==2) && ustimeout(&ping_ticks, PING_RESP_USEC)) {
  if((ping_state==2) && ustimeout(&ping_ticks, PING_RESP_USEC)) {
    Serial.printf("%s timeout\n", "ICMP");
    ping_state = 0;
  }
  // If ICMP response received, LED off, print time
  else if(ping_state == 2 && ping_rx_time) {
    t = (ping_rx_time - ping_tx_time + 50) / 100;
    Serial.printf("Round-trip time %d.%d ms\n", t/10, t%10);
    ping_state = 0;
    if(reps > 0 && !count--) return true;

  }
  // Get any events, poll the network-join state machine
  if(ustimeout(&ping_poll_ticks, EVENT_POLL_USEC)) {
    pingEvnt.pollEvents();
    join_state_poll((char *)MY_SSID, (char *)MY_PASSPHRASE, SECURITY);
    ustimeout(&ping_poll_ticks, 0);
  }

}
  return true;
}

// Get MAC Address for default gateway (needed for non local ping).
int get_gw_mac(struct netif *netif) { 
  uint32_t mscnt = 0;
  ip4_addr_t *ipaddr_ret;
  err_t err = ERR_OK;
  struct eth_addr *ethaddr_ret;

  while (mscnt < 3000) {
    err = etharp_query(netif, &(netif->gw), NULL);
    if(err != ERR_OK) {
      printf("Error could not perform ARP query!\n");
    } else {
      err = etharp_find_addr(netif, &(netif->gw), &ethaddr_ret, (const ip4_addr_t**)&ipaddr_ret);
      if(err > -1) {
        MAC_CPY(gw_mac, (BYTE *)ethaddr_ret);
        break;
      }
    }
    delay(GW_MS_DELAY);
    mscnt += GW_MS_DELAY;
  }
  return err;
}

