// QNEthernet PING example for CYW4343W.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// =====================================================================
// = Modified for testing with Dogbone06 CYW4343W board and T4.1/DB5   =
// =====================================================================

#include <QNEthernet.h>
#include "../src/qnethernet/drivers/cyw4343w/src/cyw43_T4_SDIO.h"
#include "../src/qnethernet/drivers/cyw4343w/src/event.h"
#include "../src/secrets.h"
#include "../src/qnethernet/drivers/cyw4343w/src/join.h"
#include "../src/qnethernet/drivers/cyw4343w/src/ping.h"

using namespace qindesign::network;

Join pingJoin;
Ping myping;
Event evnt;

// After testing finished,
void waitForInput(); // to be removed later.
extern void cwydump(unsigned char *memory, unsigned int len); // To be removed later.

#define PING_RESP_USEC      200000
extern MACADDR gw_mac;

constexpr uint16_t pingCount    = 10; // Set to zero for continuous run. Default = 10.
constexpr uint32_t kDHCPTimeout = 15000;  // 15 seconds
constexpr unsigned long kPingInterval = 1000;  // 1 second

//******************************************
// Un-comment one and only one host to ping.
//******************************************
//constexpr char pHostname[]{"pjrc.com"};
//constexpr char pHostname[]{"pool.ntp.org"};
constexpr char pHostname[]{"arduino.cc"};
//constexpr char pHostname[]{"www.raspberrypi.org"};

constexpr char lHostname[]{"wwatsonT41"}; // Set to your desired host name.

namespace {  // Internal linkage section
bool running = false;  // Whether the program is still running
IPAddress pingIP;
uint8_t ping_data[pingDataSize];
uint16_t seq = 0; 
unsigned long pingTimer = millis() - kPingInterval;  // Start expired
bool replyReceived = false;  // Indicates if the current reply has
                             // been received
uint32_t pingCounter = 0;
uint32_t ping_ticks;
err_t err = ERR_OK;
}  // namespace

// Forward declarations
void echoCallback(const PING_DATA& reply);

// Ping object, for sending and receiving echo requests and replies.
static Ping ping{&echoCallback};

void setup() {
  Serial.begin(115200);
  // wait for serial port to connect.
  while (!Serial && millis() < 5000) {;}
  Serial.printf("%c",12);

  if(CrashReport) {
	Serial.print(CrashReport);
    waitForInput();
  }
  Serial.printf("CPU speed: %ld MHz\n", F_CPU_ACTUAL / 1'000'000);

  // Init data array.
  for(uint8_t i=0; i<sizeof(ping_data); i++) ping_data[i] = i;
  
  // Setup ARP and ICMP handelers.
  evnt.add_event_handler(evnt.arp_event_handler);
  evnt.add_event_handler(evnt.icmp_event_handler);

  Ethernet.setHostname(lHostname); // Set local host name.

  Serial.printf("Starting Ethernet with DHCP...\r\n");
  if (!Ethernet.begin()) {
    Serial.printf("Failed to start Ethernet\r\n");
    return;
  }

  printf("hostname = %s\n",Ethernet.hostname());

  // Get local MAC address and show it.
  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  Serial.printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Get local DHCP results. IP etc...
  Serial.printf("Waiting for local IP...\r\n");
  if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
    Serial.printf("Failed to get IP address from DHCP\r\n");
    return;
  }

  IPAddress ip = Ethernet.localIP();
  IPADDR ipcnvrt = {ip[0], ip[1], ip[2], ip[3]}; // Make this a macro
  evnt.ipInit(ipcnvrt); // Register the local IP.
  Serial.printf("    Local IP    = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.subnetMask();
  Serial.printf("    Subnet mask = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.gatewayIP();
  Serial.printf("    Gateway     = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.dnsServerIP();
  Serial.printf("    DNS         = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  Serial.printf("\r\n");
  
  // Look up the hostname to be pinged.
  Serial.printf("Looking up \"%s\"...", pHostname);
  if (Ethernet.hostByName(pHostname, pingIP)) {
    Serial.printf("\r\nIP = %u.%u.%u.%u\r\n",
           pingIP[0], pingIP[1], pingIP[2], pingIP[3]);
    running = true;
  } else {
    Serial.printf("HALTING!! Faied to get host IP: errno=%d\r\n", errno);
    while(1) {;}
  }
  myping.setCallback(echoCallback);
  initPing();
}

void loop() {
  if (!running || ((millis() - pingTimer) < kPingInterval)) {
    return;
  }
  replyReceived = false;
  PING_DATA pingReq { .dip = {pingIP[0], pingIP[1], pingIP[2], pingIP[3]},
	                  .ttl = pingTTL,
	                  .ident = pingId,
	                  .seq = seq++,
	                  .data = ping_data,
	                  .dataSize = pingDataSize};
  if(cyw43_ping(pingReq) == false) {
	Serial.printf("Ping Failed\n");
	pingJoin.join_state_poll((char *)MY_SSID, (char *)MY_PASSPHRASE, SECURITY);
  }
  pingTimer = millis();
  ustimeout(&ping_ticks, 0); // Clear ping response timer.
  // Perform one transfer. Poll for reply checking for a timeout.
  while(1) {
    evnt.pollEvents(); // Have to poll event handler.
    if(!replyReceived && (ustimeout(&ping_ticks, PING_RESP_USEC))) {
	  printf("%" PRIu32 ". Timeout\r\n", pingCounter);
	  break;
    } else if(replyReceived) {
	  break;
	}
  }
  pingCounter++;
  // If pingCount = 0 then run in continous mode else do pingCount interations.
  if((pingCount > 0) && (pingCounter == pingCount)) {
	waitForInput();
	pingCounter = seq = 0; // Reset loop and sequence counters.
  }
}

// The Echo Reply callback.
void echoCallback(const PING_DATA& reply) {
  if (reply.ident != pingId) {
    return;
  }

  // Check that the payload data matches
  bool payloadMatches =
      (reply.dataSize == pingDataSize) &&
      std::equal(&reply.data[0], &reply.data[pingDataSize], &ping_data[0]);
  Serial.printf("%lu ",pingCounter);
  Serial.printf("%d bytes from server (%u.%u.%u.%u)-""%s"" ",
  reply.dataSize, reply.dip[0], reply.dip[1], reply.dip[2], reply.dip[3],
  pHostname);
  Serial.printf("seq=%d ",htons(reply.seq));
  Serial.printf("ttl=%d",reply.ttl);
  Serial.printf(" time=%lu ms\r\n",millis() - pingTimer);
  Serial.printf("%s", payloadMatches ? "" : "(payload mismatch) ");

  replyReceived = true;
}

void initPing(void) {
  err = evnt.get_gw_mac(netif_default);
  if(err != ERR_OK) Serial.printf("Get Gateway MAC FAILED: %d\n",err);
  ustimeout(&ping_ticks, 0);
}	

bool cyw43_ping(PING_DATA &req) {
  if(pingJoin.link_check() != 1) return false; 
  evnt.ip_tx_icmp(gw_mac, req.dip, ICREQ, 0, &req);
  return true;
}

// After testing finished,
void waitForInput() // to be removed later.
{
  Serial.println("Finished: Press any key to run again...");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
