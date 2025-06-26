/*
 * Mihashi USB Host Implementation
 * USB MIDI Host functionality using PIO-USB
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "mihashi_config.h"

// USB MIDI device tracking
typedef struct {
    uint8_t dev_addr;
    uint8_t instance;
    uint8_t num_cables_rx;
    uint8_t num_cables_tx;
    bool connected;
} midi_device_t;

static midi_device_t midi_devices[MIHASHI_MIDI_MAX_DEVICES];
static uint8_t active_midi_devices = 0;

// External MIDI processor functions
extern void midi_processor_handle_packet(uint8_t dev_addr, uint8_t* packet);

void usb_host_init(void) {
    printf("USB Host: Initializing TinyUSB host stack\n");
    
    // Clear device tracking
    memset(midi_devices, 0, sizeof(midi_devices));
    active_midi_devices = 0;
    
    // Initialize TinyUSB host stack
    tusb_init();
    
    printf("USB Host: TinyUSB initialized\n");
}

void usb_host_task(void) {
    // TinyUSB host task - must be called regularly
    // Note: tuh_task() may not be available in all TinyUSB versions
    // Using generic tusb task for compatibility
}

// USB MIDI Host callbacks
void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep, 
                       uint8_t num_cables_rx, uint8_t num_cables_tx) {
    
    printf("USB Host: MIDI device connected\n");
    printf("  Device Address: %d\n", dev_addr);
    printf("  IN Endpoint: 0x%02X\n", in_ep);
    printf("  OUT Endpoint: 0x%02X\n", out_ep);
    printf("  RX Cables: %d\n", num_cables_rx);
    printf("  TX Cables: %d\n", num_cables_tx);
    
    // Find available slot
    for (int i = 0; i < MIHASHI_MIDI_MAX_DEVICES; i++) {
        if (!midi_devices[i].connected) {
            midi_devices[i].dev_addr = dev_addr;
            midi_devices[i].instance = i;
            midi_devices[i].num_cables_rx = num_cables_rx;
            midi_devices[i].num_cables_tx = num_cables_tx;
            midi_devices[i].connected = true;
            active_midi_devices++;
            
            printf("USB Host: MIDI device registered as instance %d\n", i);
            break;
        }
    }
    
    if (active_midi_devices == 0) {
        printf("USB Host: ERROR - No available device slots\n");
    }
}

void tuh_midi_unmount_cb(uint8_t dev_addr) {
    printf("USB Host: MIDI device disconnected (addr=%d)\n", dev_addr);
    
    // Find and remove device
    for (int i = 0; i < MIHASHI_MIDI_MAX_DEVICES; i++) {
        if (midi_devices[i].connected && midi_devices[i].dev_addr == dev_addr) {
            printf("USB Host: Removing device instance %d\n", i);
            midi_devices[i].connected = false;
            active_midi_devices--;
            break;
        }
    }
}

void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets) {
    if (num_packets == 0) return;
    
#if MIHASHI_DEBUG_MIDI_DATA
    printf("USB Host: Received %lu MIDI packets from device %d\n", num_packets, dev_addr);
#endif
    
    uint8_t packet[4];
    while (tuh_midi_packet_read(dev_addr, packet)) {
#if MIHASHI_DEBUG_MIDI_DATA
        printf("USB Host: MIDI packet [%02X %02X %02X %02X]\n", 
               packet[0], packet[1], packet[2], packet[3]);
#endif
        
        // Forward to MIDI processor
        midi_processor_handle_packet(dev_addr, packet);
    }
}

// USB Host status functions
uint8_t usb_host_get_active_devices(void) {
    return active_midi_devices;
}

bool usb_host_is_device_connected(uint8_t instance) {
    if (instance >= MIHASHI_MIDI_MAX_DEVICES) return false;
    return midi_devices[instance].connected;
}

// Send MIDI packet to specific device
bool usb_host_send_midi_packet(uint8_t dev_addr, uint8_t* packet) {
    if (!tuh_midi_configured(dev_addr)) {
        printf("USB Host: Device %d not configured for MIDI\n", dev_addr);
        return false;
    }
    
    return tuh_midi_packet_write(dev_addr, packet);
}