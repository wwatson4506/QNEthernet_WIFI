// Scan.cpp

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cyw43_T4_SDIO.h"
#include "misc_defs.h"
#include "SdioRegs.h"
#include "ioctl_T4.h"
#include "event.h"
#include "scan.h"

extern W4343WCard wifiCard;
extern sdpcm_header_t iehh;
Event scnevt;
Scan scn;
extern uint8_t scan_count;
//simple_scan_result_t filtered_scan_results[MAX_SCAN_ENTRIES] = {};

// Network scan parameters
SCAN_PARAMS scan_params = {
    .version=1,
    .action=1,
    .sync_id=1,
    .ssidlen=0,
    .ssid={0},
    .bssid={0xff,0xff,0xff,0xff,0xff,0xff},
    .bss_type=2,
    .scan_type=SCANTYPE_PASSIVE,
    .nprobes=-1,
    .active_time=-1,
    .passive_time=-1,
    .home_time=-1,
    .nchans=14,
    .nssids=0,
    .chans{{0,0}},
    .ssids{{0}}
};

// Start a network scan
int Scan::scan_start(void)
{
    int ret;
    
    scan_count = 0; // Zero out scan entry counter.
    scnevt.ioctl_enable_evts((EVT_STR *)escan_evts); // Enable scan events.
    ret = wifiCard.ioctl_wr_int32(WLC_SET_SCAN_CHANNEL_TIME, 10, SCAN_CHAN_TIME) > 0 &&
        wifiCard.ioctl_set_uint32("pm2_sleep_ret", IOCTL_WAIT, 0xc8) > 0 &&
        wifiCard.ioctl_set_uint32("bcn_li_bcn", IOCTL_WAIT, 1) > 0 &&
        wifiCard.ioctl_set_uint32("bcn_li_dtim", IOCTL_WAIT, 1) > 0 &&
        wifiCard.ioctl_set_uint32("assoc_listen", IOCTL_WAIT, 0x0a) > 0 &&
        wifiCard.ioctl_wr_int32(WLC_SET_BAND, IOCTL_WAIT, WIFI_BAND_ANY) > 0 &&
        wifiCard.ioctl_wr_int32(WLC_UP, IOCTL_WAIT, 0) > 0 &&
        wifiCard.ioctl_set_data("escan", IOCTL_WAIT, &scan_params, sizeof(scan_params)) > 0;
    wifiCard.ioctl_err_display(ret);
    return(ret);
}

// Remove duplicate scan entries and return new scan entry count.
uint8_t Scan::getScanCount(void) {
  return removeDuplicates();	
}

// Return pointer to filtered scan results.
simple_scan_result_t *Scan::getFilteredScanResults(void) {
  return filtered_scan_results;
}

// Comparison function for qsort
int Scan::compareBSSID(const void *a, const void *b) {
    simple_scan_result_t *scnA = (simple_scan_result_t *)a;
    simple_scan_result_t *scnB = (simple_scan_result_t *)b;
    // memcmp returns (< 0) if a < b, returns (0) if a == b, returns (> 0) if a > b.
    return memcmp(scnA->bssid, scnB->bssid, MACLEN);
}

// Remove duplicate scan entries and return new scan entry count.
int Scan::removeDuplicates(void) {
    // Need at least two entries to do compare.
    if (scan_count < 2) return 0;
    // Get pointer to scan results array.
    simple_scan_result_t *scnrslt = scnevt.getScanResults();    
    // Valid filtered_scan_results[] entry counter index.
    uint8_t index = 0;
    // We need to sort the entries first before checking for and removing
    // duplicate entries into the user result array.
    // First entry (0) is empty so add 1 to scnrslt to start at entry (1).
    qsort(scnrslt+1, scan_count, sizeof(simple_scan_result_t), scn.compareBSSID);
    // Remove duplicate entries.
    for(int i=1; i <= scan_count; i++) {
      if(MAC_CMP(scnrslt[i].bssid, scnrslt[i+1].bssid)) {
		continue;
	  } else {
		filtered_scan_results[index++] = scnrslt[i];   
	  }
    }	
    return index; // Returns the new entry count (index).
}
