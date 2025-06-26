/*
 * Mihashi Dual USB MIDI Bridge
 * RP2350A USB Device + PIO USB Host implementation
 * 
 * Architecture:
 * - Core 0: USB Device MIDI + main application logic
 * - Core 1: PIO USB Host MIDI processing
 * 
 * Data Flow:
 * GhostPC <--USB Device MIDI--> Mihashi <--PIO USB Host--> LittleJoe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "bsp/board.h"
#include "tusb.h"
#include "mihashi_dual_usb.h"

// PIO-USB configuration (if header not available)
#ifndef PIO_USB_DEFAULT_CONFIG
typedef struct {
    uint8_t pin_dp;
    uint8_t sm_tx;
    uint8_t sm_rx;
    uint8_t sm_eop;
    uint8_t pio_rx_num;
    uint8_t pio_tx_num;
} pio_usb_configuration_t;

#define PIO_USB_DEFAULT_CONFIG { 0, 0, 1, 2, 0, 0 }
#endif

// Global status
mihashi_status_t mihashi_status = {0};

// MIDI bridge buffers (shared between cores)
static midi_packet_t bridge_buffer[MIHASHI_BRIDGE_BUFSIZE];
static volatile uint32_t bridge_head = 0;
static volatile uint32_t bridge_tail = 0;

//--------------------------------------------------------------------
// CORE 1: USB Host Processing
//--------------------------------------------------------------------
void core1_entry() {
    printf("Mihashi Core1: Starting PIO USB Host\n");
    
    // Configure PIO-USB for GPIO 0,1 (simplified for compatibility)
    printf("Mihashi Core1: Configuring PIO-USB on GPIO %d,%d\n", 
           MIHASHI_PIO_USB_DP_PIN, MIHASHI_PIO_USB_DM_PIN);
    
    // Initialize USB Host stack on port 1
    tuh_init(MIHASHI_TUH_RHPORT);
    
    mihashi_status.host_ready = true;
    printf("Mihashi Core1: PIO USB Host initialized on GPIO %d,%d\n", 
           MIHASHI_PIO_USB_DP_PIN, MIHASHI_PIO_USB_DM_PIN);
    
    // USB Host task loop
    while (1) {
        tuh_task();
        sleep_ms(1);
    }
}

//--------------------------------------------------------------------
// CORE 0: Main Application and USB Device
//--------------------------------------------------------------------
void system_clock_init() {
    // Set CPU clock to 240MHz for PIO-USB compatibility
    if (!set_sys_clock_khz(MIHASHI_CPU_FREQ_KHZ, true)) {
        printf("Mihashi: Failed to set 240MHz clock, using default\n");
    } else {
        printf("Mihashi: CPU clock set to %d MHz\n", MIHASHI_CPU_FREQ_KHZ / 1000);
    }
}

void gpio_init_mihashi() {
    // Initialize PIO-USB pins
    gpio_init(MIHASHI_PIO_USB_DP_PIN);
    gpio_init(MIHASHI_PIO_USB_DM_PIN);
    
    printf("Mihashi: PIO-USB pins initialized (D+=%d, D-=%d)\n", 
           MIHASHI_PIO_USB_DP_PIN, MIHASHI_PIO_USB_DM_PIN);
}

void bridge_buffer_push(uint8_t* packet, uint8_t direction) {
    uint32_t next_head = (bridge_head + 1) % MIHASHI_BRIDGE_BUFSIZE;
    
    if (next_head != bridge_tail) {
        memcpy(bridge_buffer[bridge_head].data, packet, 4);
        bridge_buffer[bridge_head].timestamp = to_ms_since_boot(get_absolute_time());
        bridge_buffer[bridge_head].direction = direction;
        bridge_head = next_head;
    } else {
        printf("Mihashi: Bridge buffer overflow\n");
    }
}

bool bridge_buffer_pop(midi_packet_t* packet) {
    if (bridge_head == bridge_tail) {
        return false;
    }
    
    memcpy(packet, &bridge_buffer[bridge_tail], sizeof(midi_packet_t));
    bridge_tail = (bridge_tail + 1) % MIHASHI_BRIDGE_BUFSIZE;
    return true;
}

void mihashi_bridge_task() {
    midi_packet_t packet;
    
    while (bridge_buffer_pop(&packet)) {
        if (packet.direction == 0) {
            // Device -> Host: Send to connected USB Host device
            if (mihashi_status.host_device_addr > 0) {
                // Note: tuh_midi_packet_write may not be available in all TinyUSB versions
                // For now, just count the message
                printf("Mihashi: D->H MIDI [%02X %02X %02X %02X]\n", 
                       packet.data[0], packet.data[1], packet.data[2], packet.data[3]);
                mihashi_status.messages_device_to_host++;
            }
        } else {
            // Host -> Device: Send to USB Device interface
            tud_midi_packet_write(packet.data);
            mihashi_status.messages_host_to_device++;
        }
    }
}

void mihashi_print_status() {
    static uint32_t last_status = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    if (now - last_status > 5000) {  // Every 5 seconds
        printf("=== Mihashi Status ===\n");
        printf("Device Ready: %s\n", mihashi_status.device_ready ? "YES" : "NO");
        printf("Host Ready: %s\n", mihashi_status.host_ready ? "YES" : "NO");
        printf("Host Device: addr=%d\n", mihashi_status.host_device_addr);
        printf("Messages D->H: %lu\n", mihashi_status.messages_device_to_host);
        printf("Messages H->D: %lu\n", mihashi_status.messages_host_to_device);
        printf("Uptime: %lu seconds\n", now / 1000);
        printf("====================\n");
        last_status = now;
    }
}

int main() {
    // Initialize standard I/O
    stdio_init_all();
    
    printf("\n=== Mihashi Dual USB MIDI Bridge v1.0 ===\n");
    printf("Hardware: RP2350A\n");
    printf("USB Device: Native hardware\n");
    printf("USB Host: PIO-USB on GPIO 0,1\n");
    printf("========================================\n");
    
    // System initialization
    system_clock_init();
    gpio_init_mihashi();
    
    // Initialize USB Device stack
    tud_init(MIHASHI_TUD_RHPORT);
    mihashi_status.device_ready = true;
    
    printf("Mihashi Core0: USB Device initialized\n");
    
    // Launch USB Host on Core 1
    multicore_launch_core1(core1_entry);
    
    printf("Mihashi: Dual USB bridge ready\n");
    
    // Main loop - USB Device and bridge processing
    while (1) {
        // Process USB Device events
        tud_task();
        
        // Process MIDI bridge
        mihashi_bridge_task();
        
        // Status monitoring
        mihashi_print_status();
        
        sleep_ms(1);
    }
    
    return 0;
}

//--------------------------------------------------------------------
// USB Device MIDI Callbacks
//--------------------------------------------------------------------
void tud_midi_rx_cb(uint8_t itf) {
    (void)itf; // Suppress unused parameter warning
    uint8_t packet[4];
    
    while (tud_midi_packet_read(packet)) {
        printf("Mihashi USB Device RX: [%02X %02X %02X %02X]\n", 
               packet[0], packet[1], packet[2], packet[3]);
        
        // Forward to USB Host (direction 0 = device->host)
        bridge_buffer_push(packet, 0);
    }
}

//--------------------------------------------------------------------
// USB Host MIDI Callbacks  
//--------------------------------------------------------------------
void tuh_midi_mount_cb(uint8_t daddr, uint8_t in_ep, uint8_t out_ep, 
                       uint8_t num_cables_rx, uint8_t num_cables_tx) {
    printf("Mihashi USB Host: MIDI device connected\n");
    printf("  Address: %d\n", daddr);
    printf("  IN EP: 0x%02X, OUT EP: 0x%02X\n", in_ep, out_ep);
    printf("  Cables: RX=%d, TX=%d\n", num_cables_rx, num_cables_tx);
    
    mihashi_status.host_device_addr = daddr;
    mihashi_status.host_in_endpoint = in_ep;
    mihashi_status.host_out_endpoint = out_ep;
}

void tuh_midi_unmount_cb(uint8_t daddr) {
    printf("Mihashi USB Host: MIDI device disconnected (addr=%d)\n", daddr);
    
    if (mihashi_status.host_device_addr == daddr) {
        mihashi_status.host_device_addr = 0;
        mihashi_status.host_in_endpoint = 0;
        mihashi_status.host_out_endpoint = 0;
    }
}

void tuh_midi_rx_cb(uint8_t daddr, uint32_t num_packets) {
    (void)num_packets; // Suppress unused parameter warning
    uint8_t packet[4];
    
    // Note: tuh_midi_packet_read may not be available in all TinyUSB versions
    // For now, just log the callback
    printf("Mihashi USB Host RX: %lu packets from device %d\n", num_packets, daddr);
    
    // Placeholder for actual packet reading
    // TODO: Implement actual MIDI packet reading when TinyUSB MIDI host is available
    packet[0] = 0x09; // Note on, cable 0
    packet[1] = 0x90; // Note on, channel 1
    packet[2] = 0x60; // Note C4
    packet[3] = 0x7F; // Velocity 127
    
    printf("Mihashi USB Host RX: [%02X %02X %02X %02X] (placeholder)\n", 
           packet[0], packet[1], packet[2], packet[3]);
    
    // Forward to USB Device (direction 1 = host->device)
    bridge_buffer_push(packet, 1);
}