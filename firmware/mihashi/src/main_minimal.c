/*
 * Mihashi Minimal Test Firmware
 * RP2350A Basic functionality test without USB Host
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "mihashi_config.h"

// System status
static bool system_initialized = false;

void system_clock_init() {
    // Set CPU clock to 240MHz for optimal performance
    if (!set_sys_clock_khz(MIHASHI_CPU_FREQ_KHZ, true)) {
        printf("Mihashi: Failed to set 240MHz clock, using default\n");
    } else {
        printf("Mihashi: CPU clock set to %d MHz\n", MIHASHI_CPU_FREQ_KHZ / 1000);
    }
}

void gpio_init_mihashi() {
    // Initialize USB pins for future use
    gpio_init(MIHASHI_USB_DP_PIN);
    gpio_init(MIHASHI_USB_DM_PIN);
    
    // Set USB pins to input for now
    gpio_set_dir(MIHASHI_USB_DP_PIN, GPIO_IN);
    gpio_set_dir(MIHASHI_USB_DM_PIN, GPIO_IN);
    
    printf("Mihashi: USB pins initialized (D+=%d, D-=%d)\n", 
           MIHASHI_USB_DP_PIN, MIHASHI_USB_DM_PIN);
}

void system_status_task() {
    static uint32_t last_heartbeat = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // Heartbeat every 3 seconds
    if (now - last_heartbeat > 3000) {
        printf("Mihashi: Minimal firmware running, uptime=%d seconds\n", now / 1000);
        printf("Mihashi: Ready for USB Host integration\n");
        last_heartbeat = now;
    }
}

int main() {
    // Initialize standard I/O
    stdio_init_all();
    
    printf("\n=== Mihashi Minimal Test v1.0 ===\n");
    printf("Hardware: RP2350A\n");
    printf("Purpose: Basic system test\n");
    printf("================================\n");
    
    // System initialization
    system_clock_init();
    gpio_init_mihashi();
    
    printf("Mihashi: Minimal initialization complete\n");
    
    system_initialized = true;
    printf("Mihashi: System ready - minimal mode\n");
    
    // Main loop
    while (1) {
        // System status and monitoring
        system_status_task();
        
        // Basic processing
        sleep_ms(100);
    }
    
    return 0;
}