// CYW4343W Join network class.
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
// =====================================================================
// = Modified for testing with Dogbone06 CYW4343W board and T4.1/DB5   =
// =====================================================================

#ifndef JOIN_H
#define JOIN_H

#include "event.h"
#include "WIFI_init.h"

//==============================================================================
// Flags for EVENT_INFO link state.
//==============================================================================
#define LINK_UP_OK          0x01
#define LINK_AUTH_OK        0x02
#define LINK_OK            (LINK_UP_OK+LINK_AUTH_OK)
#define LINK_FAIL           0x04

//==============================================================================
// Join networks states.
//==============================================================================
#define JOIN_IDLE           0
#define JOIN_JOINING        1
#define JOIN_OK             2
#define JOIN_FAIL           3
//==============================================================================

//==============================================================================
// Join networks timeouts.
//==============================================================================
#define JOIN_TRY_USEC       10000000
#define JOIN_RETRY_USEC     10000000
//==============================================================================

//==============================================================================
// Join event enable string.
//==============================================================================
#define JOIN_EVTS   {EVT(WLC_E_SET_SSID), EVT(WLC_E_LINK), EVT(WLC_E_AUTH), \
        EVT(WLC_E_DEAUTH_IND), EVT(WLC_E_DISASSOC_IND), EVT(WLC_E_PSK_SUP), EVT(-1)}
//==============================================================================

//==============================================================================
// Join networks country defines.
//==============================================================================
#define COUNTRY         "US"
#define COUNTRY_REV     -1
//==============================================================================

//==============================================================================
// Join networks class.
//==============================================================================
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
//==============================================================================

// EOF
#endif
