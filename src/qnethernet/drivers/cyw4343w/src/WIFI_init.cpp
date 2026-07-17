// Teensy 4.1 and CYW4343W initialization functions.
//
// Copyright (c) 2026, Warren Watson
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
// ===============================================================
// = Used for testing with Dogbone06 CYW4343W board and T4.1/DB5 =
// ===============================================================

#include "cyw43_T4_SDIO.h"
#include "misc_defs.h"
#include "WIFI_init.h"

using namespace qindesign::network::driver;

extern W4343WCard WIFIcard;
Join join;
Event evt;
MACADDR llmac;

// =====================================================================
// Initialize Teensy SDIO and CYW4343x WIFI chip.
// =====================================================================
int WIFIinit(const char *ssID, const char *passphrase, int security) {
  // Add our event handler to the array of event handlers.
  evt.add_event_handler(join.join_event_handler);
  delayMicroseconds(1000);

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
    delay(500); // Give the CYW4343W time to settle in.
    WIFIcard.getMACAddress(llmac); // Get Low Level MAC Addrees.
    // Use "secrets.h" to set MY_SSID, MY_PASSPHRASE, SECURITY.
    if(!join.join_start(ssID, passphrase, security)) {
      printf("*************** Error: Join Network Failed! ***************\n");
      while(1);
    }
    // Keep polling until link and join happens.
    while((join.link_check() != LINK_OK) && (join.join_check() != JOIN_OK)) {
      evt.pollEvents();
      join.join_state_poll(ssID, passphrase, security);
    }
  } else {
    printf("*************** Initialization Failed! ****************\n");
    return -1;
  }
  return 1;
}
