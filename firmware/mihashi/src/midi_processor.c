/*
 * Mihashi MIDI Processor
 * MIDI message processing and forwarding logic
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "mihashi_config.h"

// MIDI message buffer
typedef struct {
    uint8_t packet[4];
    uint32_t timestamp;
    uint8_t source_device;
} midi_message_t;

static midi_message_t midi_buffer[MIHASHI_MIDI_BUFFER_SIZE];
static uint32_t buffer_head = 0;
static uint32_t buffer_tail = 0;
static uint32_t messages_processed = 0;
static uint32_t messages_forwarded = 0;

// External USB host functions
extern bool usb_host_send_midi_packet(uint8_t dev_addr, uint8_t* packet);

void midi_processor_init(void) {
    printf("MIDI Processor: Initializing\n");
    
    // Clear buffer
    memset(midi_buffer, 0, sizeof(midi_buffer));
    buffer_head = 0;
    buffer_tail = 0;
    messages_processed = 0;
    messages_forwarded = 0;
    
    printf("MIDI Processor: Buffer size = %d messages\n", MIHASHI_MIDI_BUFFER_SIZE);
}

bool midi_buffer_is_full(void) {
    return ((buffer_head + 1) % MIHASHI_MIDI_BUFFER_SIZE) == buffer_tail;
}

bool midi_buffer_is_empty(void) {
    return buffer_head == buffer_tail;
}

bool midi_buffer_push(uint8_t dev_addr, uint8_t* packet) {
    if (midi_buffer_is_full()) {
        printf("MIDI Processor: Buffer overflow, dropping message\n");
        return false;
    }
    
    // Store message
    memcpy(midi_buffer[buffer_head].packet, packet, 4);
    midi_buffer[buffer_head].timestamp = to_ms_since_boot(get_absolute_time());
    midi_buffer[buffer_head].source_device = dev_addr;
    
    // Advance head
    buffer_head = (buffer_head + 1) % MIHASHI_MIDI_BUFFER_SIZE;
    
    return true;
}

bool midi_buffer_pop(midi_message_t* message) {
    if (midi_buffer_is_empty()) {
        return false;
    }
    
    // Copy message
    memcpy(message, &midi_buffer[buffer_tail], sizeof(midi_message_t));
    
    // Advance tail
    buffer_tail = (buffer_tail + 1) % MIHASHI_MIDI_BUFFER_SIZE;
    
    return true;
}

const char* midi_get_message_type(uint8_t status) {
    switch (status & 0xF0) {
        case 0x80: return "Note Off";
        case 0x90: return "Note On";
        case 0xA0: return "Aftertouch";
        case 0xB0: return "Control Change";
        case 0xC0: return "Program Change";
        case 0xD0: return "Channel Pressure";
        case 0xE0: return "Pitch Bend";
        case 0xF0: return "System";
        default: return "Unknown";
    }
}

void midi_process_message(midi_message_t* message) {
    uint8_t* packet = message->packet;
    uint8_t cable_num = (packet[0] >> 4) & 0x0F;
    uint8_t code_index = packet[0] & 0x0F;
    
#if MIHASHI_DEBUG_MIDI_DATA
    printf("MIDI Processor: Processing message\n");
    printf("  Cable: %d, Code: 0x%X\n", cable_num, code_index);
    printf("  Data: [%02X %02X %02X]\n", packet[1], packet[2], packet[3]);
    printf("  Type: %s\n", midi_get_message_type(packet[1]));
    printf("  Timestamp: %lu ms\n", message->timestamp);
#endif
    
    // Message processing logic
    // For now, forward all valid MIDI messages to LittleJoe
    
    if (code_index != 0 && packet[1] != 0) {
        // Valid MIDI message, forward to connected device
        // In our case, this would be LittleJoe on Port A
        
        // Note: For now, we'll just log the message
        // Later, this will forward to the connected MIDI device
        printf("MIDI Processor: Forwarding %s message (Ch %d)\n", 
               midi_get_message_type(packet[1]), 
               (packet[1] & 0x0F) + 1);
        
        messages_forwarded++;
    }
    
    messages_processed++;
}

void midi_processor_handle_packet(uint8_t dev_addr, uint8_t* packet) {
    // Queue message for processing
    if (!midi_buffer_push(dev_addr, packet)) {
        printf("MIDI Processor: Failed to queue message from device %d\n", dev_addr);
        return;
    }
    
    // Process queued messages
    midi_message_t message;
    while (midi_buffer_pop(&message)) {
        midi_process_message(&message);
    }
}

// Status and statistics
void midi_processor_get_stats(uint32_t* processed, uint32_t* forwarded, uint32_t* buffer_usage) {
    *processed = messages_processed;
    *forwarded = messages_forwarded;
    *buffer_usage = (buffer_head >= buffer_tail) ? 
                    (buffer_head - buffer_tail) : 
                    (MIHASHI_MIDI_BUFFER_SIZE - buffer_tail + buffer_head);
}

void midi_processor_print_stats(void) {
    uint32_t processed, forwarded, buffer_usage;
    midi_processor_get_stats(&processed, &forwarded, &buffer_usage);
    
    printf("MIDI Processor Statistics:\n");
    printf("  Messages processed: %lu\n", processed);
    printf("  Messages forwarded: %lu\n", forwarded);
    printf("  Buffer usage: %lu/%d\n", buffer_usage, MIHASHI_MIDI_BUFFER_SIZE);
}