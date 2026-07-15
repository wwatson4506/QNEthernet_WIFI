// QNEthernet SCAN example for CYW4343W.
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
#include "../src/qnethernet/drivers/cyw4343w/src/scan.h"

using namespace qindesign::network;

Scan scan;
extern Event scnevt;
MACADDR mac;

uint32_t scnpoll_ticks;
uint8_t scan_entries = 0;
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
  scnevt.add_event_handler(scan.scan_event_handler);

  //////////////////////////////////////////
  //Begin parameters: 
  //SDIO1 (false), SDIO2 (true)
  //WL_REG_ON pin 
  //WL_IRQ pin (-1 to ignore)
  //EXT_LPO pin (optional, -1 to ignore)
  //////////////////////////////////////////
  if (WIFIcard.begin(true, 33, 34, -1) == true) { 
    WIFIcard.wifiSetup(); // Only needed for wifi scan usage
    WIFIcard.postInitSettings();
    Serial.println("initialization done");
    if(WIFIcard.getMACAddress(mac) > 0) {
      Serial.printf("MAC address - ");
      for (uint8_t i = 0; i < 6; i++) {
        Serial.printf("%s%02X", i ? ":" : "", mac[i]);
      }
      Serial.printf("\n");
    }
  } else {
    Serial.println("initialization failed!");
  }
  Serial.println("Setup complete");

}

void loop() {
  Serial.println("\n\n====== STARTING SCAN ======"); 
  if(!scan.scan_start())
    Serial.printf("Error: can't start scan\n");
  WIFIcard.ustimeout(&scnpoll_ticks, 0);
  while (1) {
    // Get any events
    if(WIFIcard.ustimeout(&scnpoll_ticks, 10000)) {
      if(scnevt.pollEvents() < 0) { // -1, scan finished.
        break;
      }
      WIFIcard.ustimeout(&scnpoll_ticks, 0);
    }
  }

  scan_entries = scan.getScanCount();
  simple_scan_result_t *sr = scan.getFilteredScanResults();
  Serial.printf("\nScan Entries = %u\n",scan_entries);
  PRINT_SCAN_TEMPLATE();
  for(int i = 0; i < scan_entries; i++) {
    Serial.printf(" %2u    %-32s     %4d dBm   %2d    %02X:%02X:%02X:%02X:%02X:%02X    %-5s  (%lu)\n",
         i+1,
         sr[i].ssid,
         sr[i].signal_strength,
         sr[i].channel&0xff,
         sr[i].bssid[0],
         sr[i].bssid[1],
         sr[i].bssid[2],
         sr[i].bssid[3],
         sr[i].bssid[4],
         sr[i].bssid[5],
         sr[i].security,
         sr[i].security_mask);
  }

  Serial.printf("Scan complete\n");

//  waitForInput();
  Serial.printf("Wait for next scan...\n");
  delay(5000);
}

// After testing finished,
void waitForInput() // to be removed later.
{
  Serial.println("Press any key to continue...");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
