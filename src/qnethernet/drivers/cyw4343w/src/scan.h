// scan.h

#ifndef SCAN_H
#define SCAN_H

#include "event.h"

#define MAX_SCAN_ENTRIES 80 // Initial entry count (less if not showing hidden sites).
#define SCAN_CHAN_TIME      40
#define SCANTYPE_ACTIVE     0
#define SCANTYPE_PASSIVE    1

// Scan event enable command string.
const EVT_STR escan_evts[] = {EVT(WLC_E_ESCAN_RESULT), // PICOWI version
                              EVT(WLC_E_SET_SSID), 
                              EVT(-1)};

int compareBSSID(const void *a, const void *b);

typedef struct {
  uint32_t version;
  uint16_t action,
           sync_id;
  uint32_t ssidlen;
  uint8_t  ssid[SSID_MAXLEN],
           bssid[6],
           bss_type,
           scan_type;
  int32_t  nprobes,  // Needs to accommodate negative numbers. Was uint32_t
           active_time,
           passive_time,
           home_time;
  uint16_t nchans,
           nssids;
  uint8_t  chans[14][2],
           ssids[1][SSID_MAXLEN];
} SCAN_PARAMS;

// Scan result header (part of wl_escan_result_t)
typedef struct {
    uint32_t buflen;
    uint32_t version;
    uint16_t sync_id;
    uint16_t bss_count;
} SCAN_RESULT_HDR;

// Escan result event (excluding 12-byte IOCTL header and BDC header)
typedef struct {
    ETHER_HDR ether;
    BCMETH_HDR bcmeth;
    EVENT_HDR eventh;
    SCAN_RESULT_HDR scanh;
    wl_bss_info_t info;
} ESCAN_RESULT;

/**
 * Structure to store scan result parameters for each AP
 * Modified to add security_mask term.
 */
typedef struct simple_scan_result {
    uint8_t ssid[32];        /**< Service Set Identification (i.e. Name of Access Point)                    */
    uint8_t bssid[6];        /**< Basic Service Set Identification (i.e. MAC address of Access Point)       */
    int16_t signal_strength; /**< Receive Signal Strength Indication in dBm. <-90=Very poor, >-30=Excellent */
    uint8_t security[15];    /**< Security type (Simple mask version) Leave room for longer descryption     */
    uint8_t channel;         /**< Radio channel that the AP beacon was received on                          */
	uint8_t security_mask;   /**< Security Type mask. Used in RSN for decoding security type                */
} simple_scan_result_t;

// Scan result Array. A smaller scan struct for usage processing and display of results.
static simple_scan_result_t  __attribute__((unused)) scan_results[MAX_SCAN_ENTRIES];

#define PRINT_SCAN_TEMPLATE()                   printf("\n**********************************************************************************************\n" \
                                                "* #          SSID                             RSSI   Channel       BSSID          Security   *\n" \
                                                "**********************************************************************************************\n");
class Scan {
public:
  static int scan_event_handler(EVENT_INFO *eip);
  int scan_start(void);
  uint8_t getScanCount(void);
  simple_scan_result_t *getFilteredScanResults(void);
  static int compareBSSID(const void *a, const void *b);
  int removeDuplicates(void);
  static uint32_t parseScanResult(ESCAN_RESULT *evsrp);
  simple_scan_result_t *getScanResults(void);
protected:

private:
  simple_scan_result_t filtered_scan_results[MAX_SCAN_ENTRIES] = {};
};

// EOF
#endif

