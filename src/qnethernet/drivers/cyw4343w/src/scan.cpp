// Scan.cpp

#include "cyw43_T4_SDIO.h"
#include "misc_defs.h"
#include "SdioRegs.h"
#include "ioctl_T4.h"
#include "event.h"
#include "scan.h"

extern W4343WCard wifiCard;
extern sdpcm_header_t iehh;
Event scnevt;

// Network scan parameters
brcmf_escan_params_le scan_params = {
    .version=1, .action=1, ._=0,
    .params_le {
      .ssid_le {
        .SSID_len=0, .SSID={0}
      }, 
      .bssid={0xff,0xff,0xff,0xff,0xff,0xff}, .bss_type=2,
      .scan_type=SCANTYPE_PASSIVE, .nprobes=-1, .active_time=-1,
      .passive_time=-1, .home_time=-1, 
#if SCAN_CHAN == 0
      .nchans=14, .nssids=0, 
//    .chans={{1,0x2b},{2,0x2b},{3,0x2b},{4,0x2b},{5,0x2b},{6,0x2b},{7,0x2b},
//      {8,0x2b},{9,0x2b},{10,0x2b},{11,0x2b},{12,0x2b},{13,0x2b},{14,0x2b}},
#else
      .nchans=1,
      .nssids=0,
      .chans={{SCAN_CHAN,0x2b}},
      .ssids={{0}}
#endif
    }
};

// Start a network scan
int Scan::scan_start(void)
{
    int ret;
    
    scnevt.ioctl_enable_evts((EVT_STR *)escan_evts);
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

