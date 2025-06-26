/*
 * Mihashi Simple USB Device MIDI
 * RP2350A USB MIDI Device implementation
 * 
 * Simple version for testing USB Device functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "tusb.h"

// Configuration
#define MIHASHI_CPU_FREQ_KHZ      240000  // 240MHz
#define MIHASHI_PIO_USB_DP_PIN    13      // GPIO 13 (for future host)
#define MIHASHI_PIO_USB_DM_PIN    12      // GPIO 12 (for future host)

// Status tracking
static bool device_ready = false;
static uint32_t messages_received = 0;
static uint32_t messages_sent = 0;

void system_clock_init() {
    // Set CPU clock to 240MHz
    if (!set_sys_clock_khz(MIHASHI_CPU_FREQ_KHZ, true)) {
        printf("Mihashi: Failed to set 240MHz clock, using default\n");
    } else {
        printf("Mihashi: CPU clock set to %d MHz\n", MIHASHI_CPU_FREQ_KHZ / 1000);
    }
}

void gpio_init_mihashi() {
    // Initialize future PIO-USB pins
    gpio_init(MIHASHI_PIO_USB_DP_PIN);
    gpio_init(MIHASHI_PIO_USB_DM_PIN);
    
    printf("Mihashi: GPIO pins initialized (Future PIO-USB: %d,%d)\n", 
           MIHASHI_PIO_USB_DP_PIN, MIHASHI_PIO_USB_DM_PIN);
}

void send_test_midi() {
    static uint32_t last_test = 0;
    static uint8_t note = 60;  // C4
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // Send test MIDI every 3 seconds
    if (device_ready && (now - last_test > 3000)) {
        uint8_t packet[4] = {
            0x09,        // Cable 0, Note On
            0x90,        // Note On, Channel 1
            note,        // Note number
            0x7F         // Velocity 127
        };
        
        if (tud_midi_packet_write(packet)) {
            printf("Mihashi: Sent test MIDI Note On %d\n", note);
            messages_sent++;
            
            // Cycle through notes
            note++;
            if (note > 72) note = 60;  // C4 to C5
        }
        
        last_test = now;
    }
}

void mihashi_status_task() {
    static uint32_t last_status = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    if (now - last_status > 10000) {  // Every 10 seconds
        printf("=== Mihashi Device Status ===\n");
        printf("Device Ready: %s\n", device_ready ? "YES" : "NO");
        printf("Messages RX: %lu\n", messages_received);
        printf("Messages TX: %lu\n", messages_sent);
        printf("Uptime: %lu seconds\n", now / 1000);
        printf("============================\n");
        last_status = now;
    }
}

int main() {
    // Initialize standard I/O
    stdio_init_all();
    
    printf("\n=== Mihashi USB Device MIDI v1.0 ===\n");
    printf("Hardware: RP2350A\n");
    printf("Mode: USB MIDI Device Only\n");
    printf("===================================\n");
    
    // System initialization
    system_clock_init();
    gpio_init_mihashi();
    
    // Initialize USB Device stack
    tud_init(BOARD_TUD_RHPORT);
    device_ready = true;
    
    printf("Mihashi: USB MIDI Device initialized\n");
    printf("Mihashi: Ready to receive/send MIDI data\n");
    
    // Main loop
    while (1) {
        // Process USB Device events
        tud_task();
        
        // Send test MIDI periodically
        send_test_midi();
        
        // Status monitoring
        mihashi_status_task();
        
        sleep_ms(1);
    }
    
    return 0;
}

//--------------------------------------------------------------------
// USB Device MIDI Callbacks
//--------------------------------------------------------------------
void tud_midi_rx_cb(uint8_t itf) {
    (void)itf;
    uint8_t packet[4];
    
    while (tud_midi_packet_read(packet)) {
        printf("Mihashi RX: [%02X %02X %02X %02X]\n", 
               packet[0], packet[1], packet[2], packet[3]);
        
        messages_received++;
        
        // Echo back the MIDI message (for testing)
        if (tud_midi_packet_write(packet)) {
            printf("Mihashi: Echoed MIDI message\n");
            messages_sent++;
        }
    }
}