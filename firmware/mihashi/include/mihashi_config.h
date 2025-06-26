/*
 * Mihashi Configuration
 * RP2350A USB MIDI Host Device Configuration
 */

#ifndef MIHASHI_CONFIG_H
#define MIHASHI_CONFIG_H

// Hardware Configuration
#define MIHASHI_USB_DP_PIN      12    // USB D+ pin (GPIO 12)
#define MIHASHI_USB_DM_PIN      13    // USB D- pin (GPIO 13)
#define MIHASHI_CPU_FREQ_KHZ    240000 // 240MHz for optimal PIO USB performance

// USB Host Configuration
// Note: CFG_TUH_* defines are in tusb_config.h to avoid conflicts

// MIDI Configuration
#define MIHASHI_MIDI_MAX_DEVICES  4   // Maximum MIDI devices
#define MIHASHI_MIDI_BUFFER_SIZE  64  // MIDI message buffer size

// Debug Configuration
#define MIHASHI_DEBUG_ENABLED   1     // Enable debug output
#define MIHASHI_DEBUG_USB_ENUM  1     // Debug USB enumeration
#define MIHASHI_DEBUG_MIDI_DATA 1     // Debug MIDI data flow

// Port Configuration
typedef enum {
    MIHASHI_PORT_A = 0,  // Primary USB Host port (connected to LittleJoe)
    MIHASHI_PORT_B = 1,  // Secondary USB Host port (future expansion)
    MIHASHI_PORT_MAX
} mihashi_port_t;

#endif // MIHASHI_CONFIG_H