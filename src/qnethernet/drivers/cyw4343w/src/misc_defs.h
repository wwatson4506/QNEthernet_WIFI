// CYW4343W MISC Defines.
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
// = and Sparkfun CYW43439 shield.                                     =
// =====================================================================
#ifndef MISC_DEFS_H
#define MISC_DEFS_H

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

//==============================================================================
// Defines for auto selected WIFI device.
// Used for identifying device type using the card ID.
//==============================================================================
#define CYW43439 43439 // For the Sparkfun CYW43439 shield.
#define CYW4343W 43430 // For the dogbone06 CYW4343W board.

//==============================================================================
// Begin parameters defines:
// SDIO1 (false), SDIO2 (true) which uSDHC IF to use.
// WL_REG_ON pin
// WL_IRQ pin (-1 to ignore)
// EXT_LPO pin (optional, -1 to ignore)
// WIFIcard.begin(true, 30, 29, -1) for CYW4343x boards. 
//==============================================================================
#define REG_ON 30
#define WL_IRQ 29
#define EXT_LPO -1

//==============================================================================
// Choose SDHC speed. 33'000 or 50'000. Default is a safe 33'000.
//==============================================================================
#define KHZ_WIFI_CLK  50000; // Max SDHC speed for CYW4343x may work at 50000 KHz
                             // with short connections to T4.1.
//==============================================================================
// Force reduce SDHC speed to 33'000. true == reduce, false == not reduce. 
//==============================================================================
#define REDUCE_CYW4343W_SPEED false 

//==============================================================================
// Un-comment to enable multicast.
//==============================================================================
#define USE_MCAST false

//==============================================================================
// Init Debug Mode. Displays initialization processes.
//==============================================================================
#define INIT_DEBUG_MODE false

//==============================================================================
// Warnings
//==============================================================================
#define DEBUG_WARNINGS false

//==============================================================================
// More verbose
//==============================================================================
#define USE_DEBUG_MODE false

//==============================================================================
// Show hidden sites during scan.
//==============================================================================
#define SHOW_HIDDEN false

//==============================================================================
// SD function numbers
//==============================================================================
#define SD_FUNC_BUS         0
#define SD_FUNC_BAK         1
#define SD_FUNC_RAD         2

//==============================================================================
// Fake function number, used on startup when bus data is swapped
//==============================================================================
#define SD_FUNC_SWAP        4
#define SD_FUNC_MASK        (SD_FUNC_SWAP - 1)
#define SD_FUNC_BUS_SWAP    (SD_FUNC_BUS | SD_FUNC_SWAP)

//==============================================================================
// SDIO bus config registers
//==============================================================================
#define BUS_CONTROL             0x000   // SPI_BUS_CONTROL
#define BUS_IOEN_REG            0x002   // SDIOD_CCCR_IOEN          I/O enable
#define BUS_IORDY_REG           0x003   // SDIOD_CCCR_IORDY         Ready indication
#define BUS_INTEN_REG           0x004   // SDIOD_CCCR_INTEN
#define BUS_INTPEND_REG         0x005   // SDIOD_CCCR_INTPEND
#define BUS_BI_CTRL_REG         0x007   // SDIOD_CCCR_BICTRL        Bus interface control
#define BUS_SPI_STATUS_REG      0x008   // SPI_STATUS_REGISTER
#define BUS_SPEED_CTRL_REG      0x013   // SDIOD_CCCR_SPEED_CONTROL Bus speed control  
#define BUS_BRCM_CARDCAP_REG    0x0f0   // SDIOD_CCCR_BRCM_CARDCAP
#define BUS_BAK_BLKSIZE_REG     0x110   // SDIOD_CCCR_F1BLKSIZE_0   Backplane blocksize 
#define BUS_RAD_BLKSIZE_REG     0x210   // SDIOD_CCCR_F2BLKSIZE_0   WiFi radio blocksize

//==============================================================================
// Misc defines.
//==============================================================================
#define ALIGN_UINT(val, align) (((val) + (align) - 1) & ~((align) - 1))
#define CYW43_WRITE_BYTES_PAD(len) ALIGN_UINT((len), 64)
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define SWAP16(x) ((x&0xff)<<8 | (x&0xff00)>>8)
#define SWAP32(x) ((x&0xff)<<24 | (x&0xff00)<<8 | (x&0xff0000)>>8 | (x&0xff000000)>>24)
//==============================================================================
// Swap bytes in two 16-bit values
//==============================================================================
#define SWAP16_2(x) ((((x) & 0xff000000) >> 8) | (((x) & 0xff0000) << 8) | \
                    (((x) & 0xff00) >> 8)      | (((x) & 0xff) << 8))

//==============================================================================
// MAC address
//==============================================================================
#define MACLEN    6           /* Ethernet (MAC) address length */
typedef uint8_t MACADDR[MACLEN];
//==============================================================================
// Check if MAC address is non-zero
//==============================================================================
#define IS_MAC_NONZERO(a) (a[0] || a[1] || a[2] || a[3] || a[4] || a[5])
//==============================================================================
// Copy a MAC address
//==============================================================================
#define MAC_CPY(a, b) memcpy(a, b, MACLEN)
//==============================================================================
// Compare two MAC addresses
//==============================================================================
#define MAC_CMP(a, b) (a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2]&&a[3]==b[3]&&a[4]==b[4]&&a[5]==b[5])
//==============================================================================
// Compare MAC address to broadcast
//==============================================================================
#define MAC_IS_BCAST(a) ((a[0]&a[1]&a[2]&a[3]&a[4]&a[5])==0xff)
//==============================================================================
// Set broadcast MAC address
//==============================================================================
#define MAC_BCAST(a) {a[0]=a[1]=a[2]=a[3]=a[4]=a[5]=0xff;}
//==============================================================================
// Check if MAC address is non-zero
//==============================================================================
#define MAC_IS_NONZERO(a) (a[0] || a[1] || a[2] || a[3] || a[4] || a[5])
//==============================================================================
// Copy a MAC address
//==============================================================================
#define MAC_CPY(a, b) memcpy(a, b, MACLEN)
//==============================================================================

//==============================================================================
// Initialiser for address variable
//==============================================================================
#define IPADDR_VAL(a, b, c, d) {a, b, c, d}
//==============================================================================
// Compare two IP addresses
//==============================================================================
#define IP_CMP(a, b)    (a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && a[3]==b[3])
//==============================================================================
// Compare IP address to broadcast
//==============================================================================
#define IP_IS_BCAST(a)  ((a[0] & a[1] & a[2] & a[3]) == 0xff)
//==============================================================================
// Copy an IP address
//==============================================================================
#define IP_CPY(a, b)    ip_cpy(a, b) // memcpy((a), (b), IPLEN) // NOT WORKING!!!!!
//==============================================================================
// Set an IP address to zero
//==============================================================================
#define IP_ZERO(a)      (a[0] = a[1] = a[2] = a[3] = 0)
//==============================================================================
// Check if IP address is zero
//==============================================================================
#define IP_IS_ZERO(a)   ((a[0] || a[1] || a[2] || a[3]) == 0)
//==============================================================================

//==============================================================================
//#define USE_DEBUG_COLORS
//==============================================================================
#if defined (USE_DEBUG_COLORS)
//Foreground: reset = 0, black = 30, red = 31, green = 32, yellow = 33, blue = 34, magenta = 35, cyan = 36, and white = 37
//Background: reset = 0, black = 40, red = 41, green = 42, yellow = 43, blue = 44, magenta = 45, cyan = 46, and white = 47
#define SER_RED "\033[1;31m"
#define SER_GREEN "\033[1;32m"
#define SER_YELLOW "\033[1;33m"
#define SER_MAGENTA "\033[1;35m"
#define SER_CYAN "\033[1;36m"
#define SER_WHITE "\033[1;37m"
#define SER_RESET "\033[1;0m"

#define SER_TRACE "\033[38;2;182;222;215m"
#define SER_INFO "\033[38;2;200;200;200m"
#define SER_WARN "\033[38;2;221;230;112m"
#define SER_ERROR "\033[38;2;255;105;82m"
#define SER_USER "\033[38;2;55;255;28m"
#define SER_GREY "\033[38;2;128;128;128m"

#else
#define SER_RED ""
#define SER_GREEN ""
#define SER_YELLOW ""
#define SER_MAGENTA ""
#define SER_CYAN ""
#define SER_WHITE ""
#define SER_RESET ""

#define SER_TRACE ""
#define SER_INFO ""
#define SER_WARN ""
#define SER_ERROR ""
#define SER_USER ""
#define SER_GREY ""
#endif //USE_DEBUG_COLORS

#endif
// EOF
