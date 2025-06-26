/*
 * Mihashi Simple Dual USB Implementation
 * Device: Standard USB MIDI Device
 * Host: GPIO-based MIDI Host (software implementation)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "tusb.h"

// Configuration
#define MIHASHI_CPU_FREQ_KHZ      240000  // 240MHz
#define MIHASHI_HOST_TX_PIN       13      // GPIO 13 for Host TX (PIO USB D+)
#define MIHASHI_HOST_RX_PIN       12      // GPIO 12 for Host RX (PIO USB D-)

// Status tracking
typedef struct {
    bool device_ready;
    bool host_ready;
    uint32_t messages_device_rx;
    uint32_t messages_device_tx;
    uint32_t messages_host_rx;
    uint32_t messages_host_tx;
} mihashi_status_t;

static mihashi_status_t status = {0};

// MIDI bridge buffer
typedef struct {
    uint8_t data[4];
    uint32_t timestamp;
    bool valid;
} midi_packet_t;

static midi_packet_t device_to_host_buffer[16];
static midi_packet_t host_to_device_buffer[16];
static volatile uint8_t d2h_head = 0, d2h_tail = 0;
static volatile uint8_t h2d_head = 0, h2d_tail = 0;

void system_clock_init() {
    if (!set_sys_clock_khz(MIHASHI_CPU_FREQ_KHZ, true)) {
        printf("Mihashi: Failed to set 240MHz clock, using default\n");
    } else {
        printf("Mihashi: CPU clock set to %d MHz\n", MIHASHI_CPU_FREQ_KHZ / 1000);
    }
}

void host_gpio_init() {
    // Initialize host communication pins
    gpio_init(MIHASHI_HOST_TX_PIN);
    gpio_init(MIHASHI_HOST_RX_PIN);
    gpio_set_dir(MIHASHI_HOST_TX_PIN, GPIO_OUT);
    gpio_set_dir(MIHASHI_HOST_RX_PIN, GPIO_IN);
    gpio_pull_up(MIHASHI_HOST_RX_PIN);
    
    printf("Mihashi: Host GPIO initialized (TX:%d, RX:%d)\n", 
           MIHASHI_HOST_TX_PIN, MIHASHI_HOST_RX_PIN);
}

bool d2h_buffer_put(uint8_t *packet) {
    uint8_t next_head = (d2h_head + 1) % 16;
    if (next_head == d2h_tail) return false; // Buffer full
    
    memcpy(device_to_host_buffer[d2h_head].data, packet, 4);
    device_to_host_buffer[d2h_head].timestamp = to_ms_since_boot(get_absolute_time());
    device_to_host_buffer[d2h_head].valid = true;
    d2h_head = next_head;
    return true;
}

bool d2h_buffer_get(uint8_t *packet) {
    if (d2h_tail == d2h_head) return false; // Buffer empty
    
    if (device_to_host_buffer[d2h_tail].valid) {
        memcpy(packet, device_to_host_buffer[d2h_tail].data, 4);
        device_to_host_buffer[d2h_tail].valid = false;
        d2h_tail = (d2h_tail + 1) % 16;
        return true;
    }
    return false;
}

bool h2d_buffer_put(uint8_t *packet) {
    uint8_t next_head = (h2d_head + 1) % 16;
    if (next_head == h2d_tail) return false; // Buffer full
    
    memcpy(host_to_device_buffer[h2d_head].data, packet, 4);
    host_to_device_buffer[h2d_head].timestamp = to_ms_since_boot(get_absolute_time());
    host_to_device_buffer[h2d_head].valid = true;
    h2d_head = next_head;
    return true;
}

bool h2d_buffer_get(uint8_t *packet) {
    if (h2d_tail == h2d_head) return false; // Buffer empty
    
    if (host_to_device_buffer[h2d_tail].valid) {
        memcpy(packet, host_to_device_buffer[h2d_tail].data, 4);
        host_to_device_buffer[h2d_tail].valid = false;
        h2d_tail = (h2d_tail + 1) % 16;
        return true;
    }
    return false;
}

void simple_host_task() {
    static uint32_t last_ping = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // Send test ping to host port every 5 seconds
    if (now - last_ping > 5000) {
        gpio_put(MIHASHI_HOST_TX_PIN, 1);
        sleep_us(100);
        gpio_put(MIHASHI_HOST_TX_PIN, 0);
        
        printf("Mihashi Host: Ping sent\n");
        status.messages_host_tx++;
        last_ping = now;
    }
    
    // Check for host device responses
    if (!gpio_get(MIHASHI_HOST_RX_PIN)) {
        // Host device detected
        if (!status.host_ready) {
            status.host_ready = true;
            printf("Mihashi Host: Device detected\n");
        }
    }
    
    // Process device-to-host bridge
    uint8_t packet[4];
    if (d2h_buffer_get(packet)) {
        // Send to host (simplified - just toggle TX pin)
        for (int i = 0; i < 4; i++) {
            for (int bit = 0; bit < 8; bit++) {
                gpio_put(MIHASHI_HOST_TX_PIN, (packet[i] >> bit) & 1);
                sleep_us(10);
            }
        }
        printf("Mihashi Host: Forwarded device packet [%02X %02X %02X %02X]\n",
               packet[0], packet[1], packet[2], packet[3]);
        status.messages_host_tx++;
    }
}

void mihashi_bridge_task() {
    // Process host-to-device bridge
    uint8_t packet[4];
    if (h2d_buffer_get(packet)) {
        if (tud_midi_packet_write(packet)) {
            printf("Mihashi Bridge: Host->Device [%02X %02X %02X %02X]\n",
                   packet[0], packet[1], packet[2], packet[3]);
            status.messages_device_tx++;
        }
    }
}

void mihashi_status_task() {
    static uint32_t last_status = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    if (now - last_status > 10000) {  // Every 10 seconds
        printf("=== Mihashi Simple Dual Status ===\n");
        printf("Device Ready: %s\n", status.device_ready ? "YES" : "NO");
        printf("Host Ready: %s\n", status.host_ready ? "YES" : "NO");
        printf("Device RX: %lu, TX: %lu\n", status.messages_device_rx, status.messages_device_tx);
        printf("Host RX: %lu, TX: %lu\n", status.messages_host_rx, status.messages_host_tx);
        printf("Uptime: %lu seconds\n", now / 1000);
        printf("==================================\n");
        last_status = now;
    }
}

void core1_entry() {
    printf("Mihashi Core1: Starting simple host task\n");
    status.host_ready = true;
    
    while (1) {
        simple_host_task();
        sleep_ms(10);
    }
}

int main() {
    stdio_init_all();
    
    printf("\n=== Mihashi Simple Dual USB v1.0 ===\n");
    printf("Hardware: RP2350A\n");
    printf("Mode: USB Device + Simple Host Bridge\n");
    printf("====================================\n");
    
    // System initialization
    system_clock_init();
    host_gpio_init();
    
    // Initialize USB Device stack
    tud_init(BOARD_TUD_RHPORT);
    status.device_ready = true;
    
    // Start Core1 for host functionality
    multicore_launch_core1(core1_entry);
    
    printf("Mihashi: Simple dual stack initialized\n");
    printf("Mihashi: Ready for MIDI bridging\n");
    
    // Main loop - Core0 handles USB Device
    while (1) {
        tud_task();
        mihashi_bridge_task();
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
        printf("Mihashi Device RX: [%02X %02X %02X %02X]\n", 
               packet[0], packet[1], packet[2], packet[3]);
        
        status.messages_device_rx++;
        
        // Buffer packet for host forwarding
        if (d2h_buffer_put(packet)) {
            printf("Mihashi Bridge: Device->Host queued\n");
        } else {
            printf("Mihashi Bridge: Device->Host buffer full\n");
        }
    }
}