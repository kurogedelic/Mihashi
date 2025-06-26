/*
 * TinyUSB Configuration for Mihashi Dual USB Stack
 * Device + Host configuration for RP2350A
 */

#ifndef _TUSB_CONFIG_DUAL_H_
#define _TUSB_CONFIG_DUAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mihashi_dual_usb.h"

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS           OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG        0
#endif

// Enable both Device and Host stacks
#define CFG_TUD_ENABLED       1  // USB Device stack
#define CFG_TUH_ENABLED       1  // USB Host stack

// Speed configuration
#define CFG_TUD_MAX_SPEED     OPT_MODE_FULL_SPEED
#define CFG_TUH_MAX_SPEED     OPT_MODE_FULL_SPEED

//--------------------------------------------------------------------
// DEVICE CONFIGURATION  
//--------------------------------------------------------------------
#define CFG_TUD_ENDPOINT0_SIZE    64

// Device classes
#define CFG_TUD_CDC               0
#define CFG_TUD_MSC               0  
#define CFG_TUD_HID               0
#define CFG_TUD_MIDI              1  // Enable MIDI Device
#define CFG_TUD_VENDOR            0

// MIDI Device buffers
#define CFG_TUD_MIDI_RX_BUFSIZE   MIHASHI_MIDI_RX_BUFSIZE
#define CFG_TUD_MIDI_TX_BUFSIZE   MIHASHI_MIDI_TX_BUFSIZE

//--------------------------------------------------------------------
// HOST CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUH_ENUMERATION_BUFSIZE 512
#define CFG_TUH_ENDPOINT_MAX        16

// Host classes
#define CFG_TUH_HUB               1
#define CFG_TUH_CDC               0
#define CFG_TUH_HID               0
#define CFG_TUH_MSC               0
#define CFG_TUH_MIDI              1  // Enable MIDI Host
#define CFG_TUH_VENDOR            0

// Device support
#define CFG_TUH_DEVICE_MAX        (CFG_TUH_HUB ? 4 : 1)

// PIO-USB Host configuration
#define CFG_TUH_RPI_PIO_USB       1

// MIDI Host configuration
#define CFG_TUH_MIDI_EP_BUFSIZE   64
#define CFG_TUH_CABLE_MAX         16

//--------------------------------------------------------------------
// PORT CONFIGURATION
//--------------------------------------------------------------------
#define BOARD_TUD_RHPORT         MIHASHI_TUD_RHPORT  // Device on port 0
#define BOARD_TUH_RHPORT         MIHASHI_TUH_RHPORT  // Host on port 1

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_DUAL_H_ */