/*
 * Mihashi Main Firmware
 * RP2350A USB MIDI Host Device
 * 
 * Main entry point and system initialization
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "mihashi_config.h"

// External function declarations
extern void usb_host_init(void);
extern void usb_host_task(void);
extern void midi_processor_init(void);

// System status
static bool system_initialized = false;

void core1_entry() {
    printf("Mihashi Core1: USB Host stack starting\n");
    
    // Initialize USB host on core 1
    usb_host_init();
    
    printf("Mihashi Core1: USB Host initialized\n");
    
    // USB host task loop
    while (1) {
        usb_host_task();
        sleep_ms(1);
    }
}

void system_clock_init() {
    // Set CPU clock to 240MHz for optimal PIO USB performance
    if (!set_sys_clock_khz(MIHASHI_CPU_FREQ_KHZ, true)) {
        printf("Mihashi: Failed to set 240MHz clock, using default\n");
    } else {
        printf("Mihashi: CPU clock set to %d MHz\n", MIHASHI_CPU_FREQ_KHZ / 1000);
    }
}

void gpio_init_mihashi() {
    // Initialize USB pins
    gpio_init(MIHASHI_USB_DP_PIN);
    gpio_init(MIHASHI_USB_DM_PIN);
    
    // Set USB pins to input (PIO will control them)
    gpio_set_dir(MIHASHI_USB_DP_PIN, GPIO_IN);
    gpio_set_dir(MIHASHI_USB_DM_PIN, GPIO_IN);
    
    printf("Mihashi: USB pins initialized (D+=%d, D-=%d)\n", 
           MIHASHI_USB_DP_PIN, MIHASHI_USB_DM_PIN);
}

void system_status_task() {
    static uint32_t last_heartbeat = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // Heartbeat every 5 seconds
    if (now - last_heartbeat > 5000) {
        printf("Mihashi: System running, uptime=%d seconds\n", now / 1000);
        last_heartbeat = now;
    }
}

int main() {
    // Initialize standard I/O
    stdio_init_all();
    
    printf("\n=== Mihashi USB MIDI Host v1.0 ===\n");
    printf("Hardware: RP2350A USB PIO HOST\n");
    printf("Target: LittleJoe MIDI Monitor\n");
    printf("====================================\n");
    
    // System initialization
    system_clock_init();
    gpio_init_mihashi();
    midi_processor_init();
    
    printf("Mihashi: Core0 initialization complete\n");
    
    // Launch USB host stack on core 1
    multicore_launch_core1(core1_entry);
    
    printf("Mihashi: Core1 launched for USB host stack\n");
    
    system_initialized = true;
    printf("Mihashi: System initialization complete\n");
    
    // Main loop on core 0
    while (1) {
        // System status and monitoring
        system_status_task();
        
        // Main processing tasks
        // (MIDI processing, monitoring, etc.)
        
        sleep_ms(100);
    }
    
    return 0;
}