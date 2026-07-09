// QNEthernet JOIN test example for CYW4343W.
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
#include "../src/qnethernet/drivers/cyw4343w/src/WIFI_init.h"

using namespace qindesign::network;

Join qnjoin;
Event qnevt;

MACADDR mac;
uint32_t poll_ticks;
#define EVENT_POLL_USEC    100000 //100000
constexpr uint32_t kDHCPTimeout = 30000;  // 15 seconds

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

  printf("Starting Ethernet with DHCP...\r\n");
  printf("Please Wait...\n");
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\r\n");
    return;
  }

  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Get local DHCP results. IP etc...
  Serial.printf("Waiting for local IP...\r\n");
  if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
    Serial.printf("Failed to get IP address from DHCP\r\n");
    while(1);
  }

  IPAddress ip = Ethernet.localIP();
  IPADDR ipcnvrt = {ip[0], ip[1], ip[2], ip[3]}; // Make this a macro
  qnevt.ipInit(ipcnvrt); // Register the local IP.
  Serial.printf("    Local IP    = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.subnetMask();
  Serial.printf("    Subnet mask = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.gatewayIP();
  Serial.printf("    Gateway     = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.dnsServerIP();
  Serial.printf("    DNS         = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  Serial.printf("\r\n");

}

void loop() {

  waitForInput();
//  Serial.printf("Wait for next scan...\n");
//  delay(5000);
}

// After testing finished,
void waitForInput() // to be removed later.
{
  Serial.println("Press any key to continue...");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
