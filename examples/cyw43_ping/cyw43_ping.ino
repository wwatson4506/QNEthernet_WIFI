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
#include "../src/qnethernet/drivers/cyw4343w/src/secrets.h"
#include "../src/qnethernet/drivers/cyw4343w/src/join.h"
#include "../src/qnethernet/drivers/cyw4343w/src/ping.h"

using namespace qindesign::network;

Event evnt;

// After testing finished,
void waitForInput(); // to be removed later.
extern void cwydump(unsigned char *memory, unsigned int len); // To be removed later.

#define PING_DATA_SIZE      64 //32
#define PING_COUNT     0 // Set to zero for continuous run.

constexpr uint32_t kDHCPTimeout = 15000;  // 15 seconds

//constexpr char kHostname[]{"pjrc.com"};
//constexpr char kHostname[]{"pool.ntp.org"};
constexpr char kHostname[]{"arduino.cc"};
//constexpr char kHostname[]{"www.raspberrypi.org"};

IPAddress pingIP;
BYTE ping_data[PING_DATA_SIZE];
err_t err = ERR_OK;

void setup()
{
  Serial.begin(115200);
  // wait for serial port to connect.
  while (!Serial && millis() < 5000) {}
  Serial.printf("%c",12);

  if(CrashReport) {
	Serial.print(CrashReport);
    waitForInput();
  }
  Serial.printf("CPU speed: %ld MHz\n", F_CPU_ACTUAL / 1'000'000);

  // Init data array.
  for(uint8_t i=0; i<sizeof(ping_data); i++) ping_data[i] = i;
  
  // Setup ARP and ICMP handelers.
  evnt.add_event_handler(arp_event_handler);
  evnt.add_event_handler(icmp_event_handler);

  Serial.printf("Starting Ethernet with DHCP...\r\n");
  if (!Ethernet.begin()) {
    Serial.printf("Failed to start Ethernet\r\n");
    return;
  }

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
  
  // Look up the hostname
  Serial.printf("Looking up \"%s\"...", kHostname);
  if (Ethernet.hostByName(kHostname, pingIP)) {
    Serial.printf("\r\nIP = %u.%u.%u.%u\r\n",
           pingIP[0], pingIP[1], pingIP[2], pingIP[3]);
  } else {
    Serial.printf("HALTING!! Faied to get host IP: errno=%d\r\n", errno);
    while(1) {;}
  }
  initPing();
  err = get_gw_mac(netif_default);
  if(err != ERR_OK) Serial.printf("Get Gateway MAC FAILED: %d\n",err);
}

void loop() {
  IPADDR pingIPcnvrt = {pingIP[0], pingIP[1], pingIP[2], pingIP[3]};
  if(cyw43_ping(pingIPcnvrt, ping_data, PING_COUNT) == false)
	Serial.printf("Ping Failed\n");
  waitForInput();
}

// After testing finished,
void waitForInput() // to be removed later.
{
  Serial.println("Finished: Press any key to try again...");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
