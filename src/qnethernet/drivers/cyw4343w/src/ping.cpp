// ping.cpp

#include "ping.h"
#include "cyw43_T4_SDIO.h"
#include "../src/qnethernet/drivers/cyw4343w/src/secrets.h"
#include "join.h"

using namespace qindesign::network;
Join pingJoin;

#define EVENT_POLL_USEC     750 //200 //100000
#define PING_RESP_USEC      600000
#define PING_INTERVAL       1000000

Event pingEvnt;
extern uint32_t ping_tx_time, ping_rx_time;
extern MACADDR gw_mac;
uint32_t wait_ticks, ping_poll_ticks, ping_ticks;
int ping_state = 0, t;
err_t err = ERR_OK;


void Ping::initPing(void) {
  err = pingEvnt.get_gw_mac(netif_default);
  if(err != ERR_OK) Serial.printf("Get Gateway MAC FAILED: %d\n",err);

  ustimeout(&wait_ticks, 0);
  ustimeout(&ping_poll_ticks, 0);
}	
	
//bool cyw43_ping(IPADDR pingIP, uint8_t *ping_data, uint8_t ping_data_size, int count) {
bool Ping::cyw43_ping(PING_DATA &req, int count) {
  int reps = count;
  if(pingJoin.link_check() != 1) return false; 
  while(1) {
    if(ustimeout(&wait_ticks, PING_INTERVAL)) {
     ustimeout(&ping_ticks, 0);
     ping_tx_time = micros();
     pingEvnt.ip_tx_icmp(gw_mac, req.dip, ICREQ, 0, (uint8_t *)req.data, req.dataSize);
     req.seq++;
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
      if(reps > 0 && !count--) {
        ustimeout(&ping_poll_ticks, 0);
        pingEvnt.pollEvents();
		return true;
      }
    }
    // Get any events, poll the network-join state machine
    if(ustimeout(&ping_poll_ticks, EVENT_POLL_USEC)) {
      ustimeout(&ping_poll_ticks, 0);
      pingEvnt.pollEvents();
      pingJoin.join_state_poll((char *)MY_SSID, (char *)MY_PASSPHRASE, SECURITY);
    }
  }
  return true;
}


