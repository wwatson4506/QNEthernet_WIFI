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
#include "../src/qnethernet/drivers/cyw4343w/src/secrets.h"
#include "../src/qnethernet/drivers/cyw4343w/src/join.h"
#include "../src/qnethernet/drivers/cyw4343w/src/ping.h"
#include "../src/qnethernet/drivers/cyw4343w/src/scan.h"

using namespace qindesign::network;

MACADDR mac;

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

  //////////////////////////////////////////
  //Begin parameters: 
  //SDIO1 (false), SDIO2 (true)
  //WL_REG_ON pin 
  //WL_IRQ pin (-1 to ignore)
  //EXT_LPO pin (optional, -1 to ignore)
  //////////////////////////////////////////
  if (wifiCard.begin(true, 33, 34, -1) == true) { 

    wifiCard.wifiSetup(); // Only needed for wifi scan usage
  
    wifiCard.postInitSettings();
    
    Serial.println("initialization done");

  if (wifiCard.getMACAddress(mac) > 0) {
    Serial.printf("MAC address - ");
    for (uint8_t i = 0; i < 6; i++) {
      Serial.printf("%s%02X", i ? ":" : "", mac[i]);
    }
    Serial.printf("\n");
  }

    wifiCard.getFirmwareVersion();
  } else {
    Serial.println("initialization failed!");
  }
  Serial.println("Setup complete");
}

void loop() {
  int entries = ScanNetworks();
  if(entries < 0) {
	Serial.printf("Scan error occured...");
  } else {
	Serial.printf("Number of scan entries: %d, Unfiltered scan (duplicate and hidden entries shown!)\n", entries);
  }

waitForInput();
//  Serial.printf("Wait for next scan...\n");
//  delay(10000);
}

// After testing finished,
void waitForInput() // to be removed later.
{
  Serial.println("Press any key to continue...");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
