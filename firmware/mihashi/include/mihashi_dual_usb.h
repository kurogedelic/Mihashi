/*
 * Mihashi Dual USB Configuration
 * RP2350A USB Device + PIO USB Host implementation
 */

#ifndef MIHASHI_DUAL_USB_H
#define MIHASHI_DUAL_USB_H

#include <stdint.h>
#include <stdbool.h>

// Hardware Configuration for Mihashi
#define MIHASHI_PIO_USB_DP_PIN    13   // GPIO 13 for PIO USB D+ (N)
#define MIHASHI_PIO_USB_DM_PIN    12   // GPIO 12 for PIO USB D- (P)
#define MIHASHI_CPU_FREQ_KHZ      240000  // 240MHz required for PIO-USB

// USB Port Assignments
#define MIHASHI_TUD_RHPORT        0    // USB Device on native hardware (port 0)
#define MIHASHI_TUH_RHPORT        1    // USB Host on PIO-USB (port 1)

// USB Device MIDI Configuration
#define MIHASHI_USB_VID           0x1209  // pid.codes VID
#define MIHASHI_USB_PID           0x0001  // Mihashi PID
#define MIHASHI_USB_DEVICE_VER    0x0100  // Version 1.0

// MIDI Buffer Sizes
#define MIHASHI_MIDI_RX_BUFSIZE   128
#define MIHASHI_MIDI_TX_BUFSIZE   128
#define MIHASHI_BRIDGE_BUFSIZE    256

// Device Status
typedef struct {
    bool device_ready;
    bool host_ready;
    uint8_t host_device_addr;
    uint8_t host_in_endpoint;
    uint8_t host_out_endpoint;
    uint32_t messages_device_to_host;
    uint32_t messages_host_to_device;
} mihashi_status_t;

// MIDI Bridge Buffer
typedef struct {
    uint8_t data[4];
    uint32_t timestamp;
    uint8_t direction; // 0=device->host, 1=host->device
} midi_packet_t;

// External status access
extern mihashi_status_t mihashi_status;

// Function declarations
void mihashi_dual_usb_init(void);
void mihashi_bridge_task(void);
void mihashi_print_status(void);

// TinyUSB callbacks (defined in implementation)
void tud_midi_rx_cb(uint8_t itf);
void tuh_midi_mount_cb(uint8_t daddr, uint8_t in_ep, uint8_t out_ep, uint8_t num_cables_rx, uint8_t num_cables_tx);
void tuh_midi_unmount_cb(uint8_t daddr);
void tuh_midi_rx_cb(uint8_t daddr, uint32_t num_packets);

#endif // MIHASHI_DUAL_USB_H