/*
 * TinyUSB Configuration for Mihashi Device Only
 * USB MIDI Device configuration for RP2350A
 */

#ifndef _TUSB_CONFIG_DEVICE_H_
#define _TUSB_CONFIG_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

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

// Enable Device stack only
#define CFG_TUD_ENABLED       1  // USB Device stack
#define CFG_TUH_ENABLED       0  // USB Host stack disabled

// Speed configuration
#define CFG_TUD_MAX_SPEED     OPT_MODE_FULL_SPEED

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
#define CFG_TUD_MIDI_RX_BUFSIZE   128
#define CFG_TUD_MIDI_TX_BUFSIZE   128

//--------------------------------------------------------------------
// PORT CONFIGURATION
//--------------------------------------------------------------------
#define BOARD_TUD_RHPORT         0  // Device on port 0

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_DEVICE_H_ */