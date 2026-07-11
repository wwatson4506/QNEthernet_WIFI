

#include "qnethernet/lwip_driver.h"

#ifdef QNETHERNET_INTERNAL_DRIVER_CYW4343W

#pragma message("Using CYW4343W QNEthernet Driver")

// C++ includes
#include <atomic>
#include <cstring>

#include <core_pins.h>
#include <imxrt.h>
#include <util/atomic.h>

#include "lwip/debug.h"
#include "lwip/err.h"
#include "lwip/stats.h"

#include "qnethernet/internal/macro_funcs.h"
#include "qnethernet/platforms/pgmspace.h"

#include "cyw4343w/src/cyw43_T4_SDIO.h"
#include "cyw4343w/src/join.h"
#include "../src/secrets.h"

namespace qindesign {
namespace network {
namespace driver {

Join qnjoin;
W4343WCard WIFIcard;
Event evt;

static int s_checkLinkStatusState = 0;
uint32_t poll_ticks;
static struct LinkInfo s_linkInfo;

FLASHMEM void get_capabilities(DriverCapabilities* const dc) {
  dc->isMACSettable                = false;
  dc->isLinkStateDetectable        = true;
  dc->isLinkSpeedDetectable        = false;
  dc->isLinkSpeedSettable          = false;
  dc->isLinkFullDuplexDetectable   = false;
  dc->isLinkFullDuplexSettable     = false;
  dc->isAutoNegotiationSettable    = false;
  dc->isLinkCrossoverDetectable    = false;
  dc->isAutoNegotiationRestartable = false;
  dc->isPHYResettable              = false;
}

typedef enum {
  kInitStateStart,           // Unknown hardware
  kInitStateNoHardware,      // No PHY
  kInitStateHasHardware,     // Has PHY
  kInitStatePHYInitialized,  // PHY's been initialized
  kInitStateInitialized,     // PHY and MAC have been initialized
} enet_init_states_t;

static enet_init_states_t s_initState = kInitStateStart;

void get_system_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  WIFIcard.getMACAddress((uint8_t *)mac);
}

bool get_mac(uint8_t mac[ETH_HWADDR_LEN]) {
  if(qnjoin.join_check() != JOIN_OK) return false;
  WIFIcard.getMACAddress((uint8_t *)mac);
  return true;
}

bool set_mac(const uint8_t mac[ETH_HWADDR_LEN]) {
  if(qnjoin.join_check() != JOIN_OK) return false;
  WIFIcard.getMACAddress((uint8_t *)mac);
  return true;
}

bool has_hardware() {
  switch (s_initState) {
    case kInitStateHasHardware:
      ATTRIBUTE_FALLTHROUGH;
    case kInitStatePHYInitialized:
      ATTRIBUTE_FALLTHROUGH;
    case kInitStateInitialized:
      return true;
    case kInitStateNoHardware:
      return false;
    default:
      break;
  }
  // Init WIFI hardware and join network.
  WIFIinit(MY_SSID, MY_PASSPHRASE, SECURITY);
  return true;
}

void set_chip_select_pin(int pin) {
	(void)pin;
}

bool init(void) {
  if (s_initState == kInitStateInitialized) {
    return true;
  } else {
    s_initState = kInitStateInitialized;
    return true;
  }
}

void deinit() {
  qnjoin.join_stop();
}

// driver_proc_input() NOT WORKING with ping!!!!
struct pbuf* proc_input(struct netif *netif, int counter) {
  // Finish any pending link and join status check
  if(netif_is_link_up(netif) == 0) return NULL; 
  sdpcm_header_t hp;
  uint8_t bf[MAX_FRAME_LEN];
  uint32_t data_len;
  if((data_len = evt.ioctl_get_event(&hp, bf, MAX_FRAME_LEN)) > 0) {
    struct pbuf *p = pbuf_alloc(PBUF_RAW, data_len+ETH_PAD_SIZE, PBUF_RAM);
    if (p == NULL) {
      printf("Failed to allocate pbuf\n");
      return NULL;
    }
    p->len = p->tot_len = data_len+ETH_PAD_SIZE+ETH_PAD_SIZE;
    LWIP_ASSERT("Expected space for pbuf fill",
      pbuf_take(p, (uint8_t *)bf+10, p->tot_len) == ERR_OK);
    return p;
  }
  return NULL;
}

void poll(struct netif *netif) {
  // Get any events, poll the joining state machine
  if(WIFIcard.ustimeout(&poll_ticks, EVENT_POLL_USEC)) {
    evt.pollEvents();
    qnjoin.join_state_poll(MY_SSID, MY_PASSPHRASE, SECURITY);
    WIFIcard.ustimeout(&poll_ticks, 0);
  }
  uint8_t link_up = qnjoin.join_check() ? 1 : 0;
  if (netif_is_link_up(netif) != link_up) {
    if (link_up) {
      printf("Setting link up\n");
      netif_set_link_up(netif);
      s_checkLinkStatusState = link_up;
    } else {
      printf("Setting link down\n");
      netif_set_link_down(netif);
      s_checkLinkStatusState = link_up;
    }
  }
}

err_t output(struct pbuf *p) {
  uint8_t *buffer;
  buffer = (uint8_t *)malloc(p->tot_len*sizeof(uint8_t));

  const uint16_t copied = pbuf_copy_partial(p, buffer, p->tot_len, 0);
  if (copied != p->tot_len) {
    return ERR_BUF;
  }
  evt.event_net_tx(buffer, copied + ETH_PAD_SIZE);
  free(buffer);
  return ERR_OK;
}

#if QNETHERNET_ENABLE_RAW_FRAME_SUPPORT
bool output_frame(const void *frame, size_t len) {
  if (len > (UINT16_MAX - ETH_PAD_SIZE)) {
    return false;
  }
  uint8_t *buffer;
  buffer = (uint8_t *)malloc(len*sizeof(uint8_t));
  (void)memcpy((uint8_t*)buffer + ETH_PAD_SIZE, frame, len);
  evt.event_net_tx(buffer, len + ETH_PAD_SIZE);
  free(buffer);
  return true;
}
#endif

void get_link_info(struct LinkInfo* const li) {
  *li = s_linkInfo;
}

bool set_incoming_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN],
                                             bool allow) {
  // CYW4343W MAC address is fixed in chip.
  return false;
}

#if !QNETHERNET_ENABLE_PROMISCUOUS_MODE
bool set_mac_address_allowed(const uint8_t mac[ETH_HWADDR_LEN], bool allow) {
  // CYW4343W MAC address is fixed in chip.
  return false;
}
#endif

bool is_unknown() {
  return s_initState == kInitStateStart;;
}

}  // namespace driver
}  // namespace network
}  // namespace qindesign

#endif
