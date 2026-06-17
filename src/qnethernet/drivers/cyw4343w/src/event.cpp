// PicoWi IP functions, see http://iosoft.blog/picowi for details
//
// Copyright (c) 2022, Jeremy P Bentham
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

#include "Arduino.h"
#include <stdint.h>
#include "cyw43_T4_SDIO.h"
#include "event.h"
#include "SdioRegs.h"
#include "misc_defs.h"
#include "ping.h"

extern Ping myping;

using namespace qindesign::network;

static int num_handlers = 0; // Counter for number of event handlers.
static IPADDR my_ip; // Local IP address variable.

Event local;
static event_handler_t event_handlers[MAX_HANDLERS];
EVENT_INFO event_info;

extern void cwydump(unsigned char *memory, unsigned int len);
extern uint8_t my_mac[6];
extern MACADDR gw_mac;
extern W4343WCard wifiCard;
extern int display_mode;
extern IOCTL_MSG ioctl_txmsg, ioctl_rxmsg;

// Event field displays
char ioctl_event_hdr_fields[] = "2:len 2: 1:seq 1:chan 1: 1:hdrlen 1:flow 1:credit";
char eth_hdr_fields[]         = "6:dest 6:srce 2;type";
char event_hdr_fields[]       = "2;sub 2;len 1: 3;oui 2;usr";
char event_msg_fields[]       = "2;ver 2;flags 4;type 4;status 4;reason 4:auth 4;dlen 6;addr 18:";

// ARP display string.
char arp_hdr_fields[]   = "2;hrd 2;pro 1;hln 1;pln 2;op 6:smac 4;sip 6:dmac 4;dip";

sdpcm_header_t iehh; // Used here and scan.cpp as well.

// Initialise the IP stack, using static address if provided
int Event::ipInit(IPADDR addr) {
  ip_cpy(my_ip,addr);
  return(1);
}

// Get MAC Address for default gateway (needed for non local ping).
int Event::get_gw_mac(struct netif *netif) { 
  uint32_t mscnt = 0;
  ip4_addr_t *ipaddr_ret;
  err_t err = ERR_OK;
  struct eth_addr *ethaddr_ret;

  while (mscnt < 3000) {
    err = etharp_query(netif, &(netif->gw), NULL);
    if(err != ERR_OK) {
      printf("Error could not perform ARP query!\n");
    } else {
      err = etharp_find_addr(netif, &(netif->gw), &ethaddr_ret, (const ip4_addr_t**)&ipaddr_ret);
      if(err > -1) {
        MAC_CPY(gw_mac, (uint8_t *)ethaddr_ret);
        break;
      }
    }
    delay(GW_MS_DELAY);
    mscnt += GW_MS_DELAY;
  }
  return err;
}

// Add an event handler to the chain
bool Event::add_event_handler(event_handler_t fn)
{
    return(add_server_event_handler(fn , 0));
}

// Add a server event handler to the chain (with local port number)
bool Event::add_server_event_handler(event_handler_t fn, uint16_t port)
{
    bool ok = num_handlers < MAX_HANDLERS;
    if (ok)
    {
        event_ports[num_handlers] = port;
        event_handlers[num_handlers++] = fn;
    }
    return (ok);
}

// Find saved ARP response
bool Event::ip_find_arp(IPADDR addr, MACADDR mac) {
    int n=0, i=arp_idx;
    bool ok=0;
    
    do
    {
        i = i == 0 ? NUM_ARP_ENTRIES-1 : i-1;
        ok = (IP_CMP(addr, arp_entries[i].ipaddr));
    } while (!ok && ++n<NUM_ARP_ENTRIES);
    if (ok)
        MAC_CPY(mac, arp_entries[i].mac);
    return(ok);
}

// Transmit an ARP frame
int Event::ip_tx_arp(MACADDR mac, IPADDR addr, uint16_t op) {
    int n = ip_make_arp(txbuff, mac, addr, op);
    return(local.ip_tx_eth(txbuff, n));
}

// Receive incoming ARP data
int Event::ip_rx_arp(uint8_t *data, int dlen) {
    ETHERHDR *ehp=(ETHERHDR *)data;
    ARPKT *arp = (ARPKT *)&data[sizeof(ETHERHDR)];
    uint16_t op = htons(arp->op);

    if (display_mode & DISP_ETH)
        ip_print_eth(data);
    if IP_CMP(arp->dip, my_ip)
    {
        if (display_mode & DISP_ARP)
            ip_print_arp(arp);
        if (op == ARPREQ)
            ip_tx_arp(ehp->srce, arp->sip, ARPRESP);
        else if (op == ARPRESP)
            ip_save_arp(arp->smac, arp->sip);
        return(1);
    }
    return(0);
}

// Create an ARP frame // Returning 12 bytes to many!!!!
int Event::ip_make_arp(uint8_t *buff, MACADDR mac, IPADDR addr, uint16_t op) {
    int n = ip_add_eth(buff, op==ARPREQ ? bcast_mac : mac, my_mac, PCOL_ARP);
    ARPKT *arp = (ARPKT *)&buff[n];

    MAC_CPY(arp->smac, my_mac);
    MAC_CPY(arp->dmac, op==ARPREQ ? bcast_mac : mac);
    arp->hrd = htons(HTYPE);
    arp->pro = htons(ARPPRO);
    arp->hln = MACLEN;
    arp->pln = sizeof(uint32_t);
    arp->op  = htons(op);
    ip_cpy(arp->dip, addr);
    ip_cpy(arp->sip, my_ip);
    if (display_mode & DISP_ARP) ip_print_arp(arp);
    return(n + sizeof(ARPKT));
}

// Save ARP result
void Event::ip_save_arp(MACADDR mac, IPADDR addr) {
    MAC_CPY(arp_entries[arp_idx].mac, mac);
    ip_cpy(arp_entries[arp_idx].ipaddr, addr);
    arp_idx = (arp_idx+1) % NUM_ARP_ENTRIES;
}

// Check if IP frame
int Event::ip_check_frame(uint8_t *data, int dlen) {
    uint8_t *p = data;
    ETHERHDR *ehp=(ETHERHDR *)p;
    IPHDR *ip = (IPHDR *)&p[sizeof(ETHERHDR)];

    return ((uint8_t)dlen >= sizeof(ETHERHDR)+sizeof(ARPKT) &&
        (!MAC_IS_BCAST(ehp->dest) || MAC_CMP(ehp->dest, my_mac)) && // Changed "(MAC_IS_BCAST(ehp->dest)" to "(!MAC_IS_BCAST(ehp->dest)"
        (htons(ehp->ptype) == PCOL_IP) &&
        (IP_IS_BCAST(ip->dip) || IP_CMP(ip->dip, my_ip) || IP_IS_ZERO(my_ip)) &&
        sizeof(ETHERHDR) + htons(ip->len) <= (uint16_t)dlen);
}

// Handler for incoming ARP frame
int Event::arp_event_handler(EVENT_INFO *eip) {
    uint8_t *p = eip->data;
    ETHERHDR *ehp=(ETHERHDR *)p;
    if(eip->chan == SDPCM_CHAN_DATA &&
      (uint8_t)eip->dlen >= sizeof(ETHERHDR)+sizeof(ARPKT) &&
      local.htons(ehp->ptype) == PCOL_ARP &&
      (MAC_IS_BCAST(ehp->dest) ||
      MAC_CMP(ehp->dest, my_mac) ))
    {
        return(local.ip_rx_arp(p, eip->dlen));
    }
    return(0);
}

// Handler for incoming ICMP frame
int Event::icmp_event_handler(EVENT_INFO *eip) {
    uint8_t *p = eip->data;
    IPHDR *ip = (IPHDR *)&p[sizeof(ETHERHDR)]; // Strip off ETHERHDR.
    if (eip->chan == SDPCM_CHAN_DATA &&
        ip->pcol == PICMP &&
        local.ip_check_frame(p, eip->dlen) &&
        IP_CMP(ip->dip, my_ip) &&
        (uint8_t)eip->dlen > sizeof(ETHERHDR)+sizeof(IPHDR)+sizeof(ICMPHDR))
    {
        return local.ip_rx_icmp(p, eip->dlen);
    }
    return(0);
}

/* Calculate TCP-style checksum, add to old value */
uint16_t Event::add_csum(uint16_t sum, void *dp, int count) {
    uint16_t n=count>>1, *p=(uint16_t *)dp, last=sum;
    while (n--) {
      sum += *p++;
      if(sum < last) sum++;
      last = sum;
    }
    if (count & 1) sum += *p & 0x00ff;
    if (sum < last) sum++;
    return(sum);
}

// Add data to buffer, return length
int Event::ip_add_data(uint8_t *buff, const void *data, int len) {
    if (len>0 && data) memcpy(buff, data, len);
    return(len);
}

// Add ICMP header to buffer, return byte count
int Event::ip_add_icmp(uint8_t *buff, uint8_t type, uint8_t code, void *pdata) {
    ICMPHDR *icmp = (ICMPHDR *)buff;
    uint16_t len = sizeof(ICMPHDR);
    PING_DATA* const pd = static_cast<PING_DATA*>(pdata);
    icmp->type = type;
    icmp->code = code;
    icmp->seq = htons(pd->seq);
    icmp->ident = 0x514E;
    icmp->check = 0;
    len += local.ip_add_data(&buff[len], pd->data, pd->dataSize);
    icmp->check = 0xffff ^ local.add_csum(0, icmp, len);
    return(len);
}

// Add IP header to buffer, return length
int Event::ip_add_hdr(uint8_t *buff, IPADDR dip, uint8_t pcol, uint16_t dlen) {
    static uint16_t ident=1;
    IPHDR *ip=(IPHDR *)buff;

    ip->ident = htons(ident++);
    ip->frags = 0;
    ip->vhl = 0x40+(sizeof(IPHDR)>>2);
    ip->service = 0;
    ip->ttl = 100; // Time To Live.
    ip->pcol = pcol;
    ip_cpy(ip->sip, my_ip);
    ip_cpy(ip->dip, dip);
    ip->len = htons(dlen + sizeof(IPHDR));
    ip->check = 0;
    ip->check = 0xffff ^ local.add_csum(0, ip, sizeof(IPHDR));
    return(sizeof(IPHDR));
}

// Send transmit data
int Event::ip_tx_eth(uint8_t *buff, int len) {
  if(display_mode & DISP_ETH) ip_print_eth(buff);
  return(event_net_tx(buff, len));
}

// Add Ethernet header to buffer, return byte count
int Event::ip_add_eth(uint8_t *buff, MACADDR dmac, MACADDR smac, uint16_t pcol) {
    ETHERHDR *ehp = (ETHERHDR *)buff;
    MAC_CPY(ehp->dest, dmac);
    MAC_CPY(ehp->srce, smac);
    ehp->ptype = htons(pcol);
    return(sizeof(ETHERHDR));
}

// Create ICMP request
int Event::ip_make_icmp(uint8_t *buff, MACADDR mac, IPADDR dip, uint8_t type, uint8_t code, void *pdata) {
    PING_DATA* const pd = static_cast<PING_DATA*>(pdata);
    int n = local.ip_add_eth(buff, mac, my_mac, PCOL_IP);
    n += local.ip_add_hdr(&buff[n], dip, PICMP, sizeof(ICMPHDR)+pd->dataSize);
    n += local.ip_add_icmp(&buff[n], type, code, pdata);
    return(n);
}

// Transmit ICMP request
int Event::ip_tx_icmp(MACADDR mac, IPADDR dip, uint8_t type, uint8_t code, void *pdata) {
    PING_DATA* const pd = static_cast<PING_DATA*>(pdata);
    int n=ip_make_icmp(txbuff, mac, dip, type, code, pd);
    if(display_mode & DISP_ICMP)
      ip_print_icmp((IPHDR *)&txbuff[sizeof(ETHERHDR)]);
    return(ip_tx_eth(txbuff, n));
}

// Run event handlers, until one returns non-zero
int Event::event_handle(EVENT_INFO *eip) {
    int i, ret=0;
    for(i=0; i<num_handlers && !ret; i++) {
       eip->server_port = event_ports[i];
       ret = event_handlers[i](eip);
    }
    return(ret);
}

// Receive incoming ICMP data
int Event::ip_rx_icmp(uint8_t *data, int dlen) {
    uint8_t *p = data;
    const uint8_t *rdata = (const uint8_t *)&p[sizeof(ETHERHDR)+sizeof(IPHDR)+sizeof(ICMPHDR)];
    ETHERHDR *ehp=(ETHERHDR *)p;
    IPHDR *ip = (IPHDR *)&p[sizeof(ETHERHDR)];
    ICMPHDR *icmp = (ICMPHDR *)&p[sizeof(ETHERHDR)+sizeof(IPHDR)];

    int n;
    if(display_mode & DISP_ICMP) ip_print_icmp(ip);
    if(icmp->type == ICREQ) { // We are being pinged. Respond to request.
      ip_add_eth(data, ehp->srce, my_mac, PCOL_IP);
      ip_cpy(ip->dip, ip->sip);
      ip_cpy(ip->sip, my_ip);
      icmp->check = add_csum(icmp->check, &icmp->type, 1);
      icmp->type = ICREP;
      n = htons(ip->len);
      // Was a ping request. Return response.
      return(local.ip_tx_eth(data, sizeof(ETHERHDR)+n+sizeof(ICMPHDR)));
    } else if (icmp->type == ICREP) { // Do we have a response to our ping?
        const PING_DATA reply{.dip   = {ip->sip[0], ip->sip[1], ip->sip[2], ip->sip[3]},
                              .ttl      = ip->ttl,
                              .ident    = icmp->ident,
                              .seq      = icmp->seq,
                              .data     = rdata,
                              .dataSize = pingDataSize};
        myping.replyf_(reply);
    }
    return(0);
}

// Enable events
int Event::ioctl_enable_evts(EVT_STR *evtp) {
  currentE_evts = evtp;
  memset(event_mask, 0, sizeof(event_mask));
  while (evtp->num >= 0) {
    if(evtp->num / 8 < (int32_t)sizeof(event_mask))
      SET_EVENT(event_mask, evtp->num);
    evtp++;
  }
  return wifiCard.ioctl_set_data("event_msgs", 0, event_mask, sizeof(event_mask));
}

// Poll events
int Event::pollEvents() {
  int n, ret = 0;
  EVENT_INFO *eip = &event_info;
  
  //Check for an event response
  if((n=ioctl_get_event(&iehh, eventbuf, sizeof(eventbuf))) > 0) {
    eip->chan = iehh.sw_header.chan; // chan = ctrl, evt or data.
    eip->flags = SWAP16(eep->event.msg.flags);
    eip->event_type = SWAP32(eep->event.msg.event_type);
    eip->status = SWAP32(eep->event.msg.status);
    eip->reason = SWAP32(eep->event.msg.reason);
    eip->data = eventbuf+10; //NOTE: Need to move eventbuf ahead by 10 bytes.
                             //      ioctl_get_event() has a 10 byte prefix that
                             //      is not used. Not sure what the 10 bytes are yet.
    eip->dlen = n; // Size of received data in bytes.
    eip->sock = -1;
    ret = event_handle(eip); // Distribute to proper event handler.
  }
  return ret;
}

// Get event data, return data length excluding header
uint32_t Event::ioctl_get_event(sdpcm_header_t *hp, uint8_t *data, int maxlen) {
    int n=0, dlen=0, blklen;
    bool res = false;
    hp->len = 0;
    res = wifiCard.cardCMD53_read(SD_FUNC_RAD, SB_32BIT_WIN, (uint8_t *)hp, sizeof(sdpcm_header_t), false);
    if(res == true && hp->len > sizeof(sdpcm_header_t) && hp->notlen > 0 && hp->len == (hp->notlen^0xffff)) {
      dlen = hp->len - sizeof(sdpcm_header_t);  //Strip off sdpcm_header_t.
      while (n < dlen && n < maxlen) {
        blklen = MIN(MIN(maxlen - n, hp->len - n), IOCTL_MAX_BLKLEN_T4);
        wifiCard.cardCMD53_read(SD_FUNC_RAD, SB_32BIT_WIN, (uint8_t *)(&data[n]), blklen, false);
        n += blklen;
      }
      //Read and discard remaining bytes over maxlen
      while (n < dlen) {
        blklen = MIN(hp->len - n, IOCTL_MAX_BLKLEN_T4);
        wifiCard.cardCMD53_read(SD_FUNC_RAD, SB_32BIT_WIN, 0, blklen, false);
        n += blklen;
      }
    }
    return dlen > maxlen ? maxlen : dlen;
}

// Return string corresponding to SDPCM channel number
char *Event::sdpcm_chan_str(int chan)
{
    return(chan==SDPCM_CHAN_CTRL ? (char *)"CTRL" : chan==SDPCM_CHAN_EVT ? (char *)"EVT ": 
           chan==SDPCM_CHAN_DATA ? (char *)"DATA" : (char *)"?");
}

// Return string corresponding to event number, without "WLC_E_" prefix
char *Event::event_str(int event)
{
    EVT_STR *evtp=currentE_evts;

    while (evtp && evtp->num>=0 && evtp->num!=event)
        evtp++;
    return(evtp && evtp->num>=0 && strlen((char *)evtp->str)>6 ? (char *)&evtp->str[6] : (char *)"?");
}

// Transmit network data
int Event::event_net_tx(void *data, int len) {
    TX_MSG *txp = &tx_msg;
    uint8_t *dp = (uint8_t *)txp;
    int txlen = sizeof(SDPCM_HDR)+2+sizeof(BDC_HDR_T4)+len;
    if(display_mode & DISP_DATA) {
      wifiCard.disp_bytes((uint8_t *)data, len);
      Serial.printf("\n");
    }
    txp->sdpcm.len = txlen;
    txp->sdpcm.notlen = ~txp->sdpcm.len;
    txp->sdpcm.seq = sd_tx_seq++;
    memcpy(txp->data, (uint8_t *)data, len);
    while (txlen & 3) dp[txlen++] = 0;
    return (wifiCard.cardCMD53_write(SD_FUNC_RAD, 0, (uint8_t *)dp, txlen, false));
}

//----------------------------------------------------------------------
// Return string corresponding to event status Added 02-21-25
//----------------------------------------------------------------------
const char *Event::ioctl_evt_status_str(int status) {
    return (status>=0 && status<MAX_EVENT_STATUS ? event_status[status] : "?");
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Return string corresponding to event number, without "WLC_E_" prefix Added 02-21-25
//----------------------------------------------------------------------
const char *Event::ioctl_evt_str(int event) {
    EVT_STR *evtp=currentE_evts;
    while (evtp && evtp->num>=0 && evtp->num!=event) evtp++;
    return(evtp && evtp->num>=0 && strlen(evtp->str)>6 ? &evtp->str[6] : "?");
}
//----------------------------------------------------------------------

// Convert byte-order in a 'short' variable (local version)
uint16_t Event::htons(uint16_t w) {
    return(w<<8 | w>>8);
}

// Copy IP address (byte-by-byte, in case misaligned)
void Event::ip_cpy(uint8_t *dest, uint8_t *src) {
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
}

// Display MAC addresses in Ethernet frame
void Event::ip_print_eth(uint8_t *buff) {
    ETHERHDR *ehp = (ETHERHDR *)buff;

    print_mac_addr(ehp->srce);
    printf("->");
    print_mac_addr(ehp->dest);
    printf("\n");
    printf("ehp->ptype = %4.4x\n",htons(ehp->ptype));
    printf("\n");
}

// Display IP address
void Event::print_ip_addr(IPADDR a) {
    printf("%u.%u.%u.%u", a[0],a[1],a[2],a[3]);
}

// Display IP addresses in IP header
void Event::print_ip_addrs(IPHDR *ip) {
    print_ip_addr(ip->sip);
    Serial.printf("->");
    print_ip_addr(ip->dip);
}

// Display MAC address
void Event::print_mac_addr(MACADDR mac) {
    printf("%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}

// Display ARP
void Event::ip_print_arp(ARPKT *arp) {
    uint16_t op=htons(arp->op);

    print_ip_addr(arp->sip);
    printf("->");
    print_ip_addr(arp->dip);
    printf(" ARP %s\n", op==ARPREQ ? "request" : op==ARPRESP ? "response" : "");
}

// Display ICMP
void Event::ip_print_icmp(IPHDR *ip) {
    ICMPHDR *icmp = (ICMPHDR *)((uint8_t *)ip + sizeof(IPHDR));
    
    print_ip_addrs(ip);
    printf(" ICMP %s\n", icmp->type==ICREQ     ? "request" : 
                        icmp->type==ICREP     ? "response" : 
                        icmp->type==ICUNREACH ? "dest unreachable" : 
                        icmp->type==ICQUENCH  ? "srce quench" : "?");
}
