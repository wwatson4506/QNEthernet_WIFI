// WIFI_init.cpp

#include "cyw43_T4_SDIO.h"
#include "misc_defs.h"
#include "WIFI_init.h"

using namespace qindesign::network::driver;

extern W4343WCard WIFIcard;
Join join;
Event evt;
MACADDR llmac;

int WIFIinit(const char *ssID, const char *passphrase, int security) {
  // Add our event handler to the array of event handlers.
  evt.add_event_handler(join.join_event_handler);
  delayMicroseconds(1000);

  //////////////////////////////////////////
  // Begin parameters: 
  // SDIO1 (false), SDIO2 (true)
  // WL_REG_ON pin 
  // WL_IRQ pin (-1 to ignore)
  // EXT_LPO pin (optional, -1 to ignore)
  // WIFIcard.begin(true, 30, 29, -1). 
  //////////////////////////////////////////
  if (WIFIcard.begin(true, REG_ON, WL_IRQ, -1) == true) { 
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
