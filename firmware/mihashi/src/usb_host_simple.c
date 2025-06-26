/*
 * Mihashi Simple USB Host Implementation
 * Basic USB Host functionality without advanced MIDI features
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "mihashi_config.h"

// Simple device tracking
static bool usb_host_initialized = false;
static uint32_t device_check_timer = 0;

void usb_host_init(void) {
    printf("USB Host: Initializing simple USB host\n");
    
    // Initialize TinyUSB
    tusb_init();
    usb_host_initialized = true;
    device_check_timer = to_ms_since_boot(get_absolute_time());
    
    printf("USB Host: Simple host initialized\n");
}

void usb_host_task(void) {
    if (!usb_host_initialized) return;
    
    // Basic host task - check for device connections
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // Check devices every 1 second
    if (now - device_check_timer > 1000) {
        // Basic device enumeration check would go here
        // For now, just maintain the timer
        device_check_timer = now;
    }
}

// Placeholder MIDI callbacks for compatibility
void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep, 
                       uint8_t num_cables_rx, uint8_t num_cables_tx) {
    printf("USB Host: MIDI device connected (simple mode)\n");
}

void tuh_midi_unmount_cb(uint8_t dev_addr) {
    printf("USB Host: MIDI device disconnected (simple mode)\n");
}

void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets) {
    printf("USB Host: MIDI data received (simple mode)\n");
}

// Status functions
uint8_t usb_host_get_active_devices(void) {
    return 0; // Simplified - no device tracking yet
}

bool usb_host_is_device_connected(uint8_t instance) {
    return false; // Simplified
}

bool usb_host_send_midi_packet(uint8_t dev_addr, uint8_t* packet) {
    printf("USB Host: MIDI send request (simple mode)\n");
    return false; // Not implemented yet
}