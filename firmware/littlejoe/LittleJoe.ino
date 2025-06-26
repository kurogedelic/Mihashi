/*
 * LittleJoe - Arduino USB MIDI Monitor (Stable Version)
 * XIAO SAMD21 Arduino USB MIDI → UART ASCII Monitor
 * 
 * Hardware: Seeed XIAO SAMD21
 * Function: Receive USB MIDI from Mihashi, output ASCII to UartMonitor via UART
 * USB Stack: Arduino (not TinyUSB) - for stability on XIAO SAMD21
 * 
 * Connections:
 * - USB-C: Mihashi USB MIDI input
 * - D6 (PA04): UART TX → UartMonitor RX
 * - D7 (PA05): UART RX ← UartMonitor TX (future use)
 * - GND/3V3: Power/Ground
 * 
 * Arduino IDE Settings:
 * - Board: Seeeduino XIAO
 * - USB Stack: Arduino (NOT TinyUSB)
 */

#include <USB-MIDI.h>
#include <MIDI.h>
#include <ArduinoJson.h>

// Arduino USB-MIDI instance (stable on XIAO SAMD21)
USBMIDI_CREATE_DEFAULT_INSTANCE();

// Configuration
#define UART_BAUD 115200
#define MIDI_CHANNEL_ALL MIDI_CHANNEL_OMNI

// Pending MIDI events (safe processing)
struct MidiEvent {
    uint8_t type;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;
    int data3;  // for pitch bend
    unsigned long timestamp;
};

volatile bool midi_event_pending = false;
MidiEvent pending_event;

void setup() {
    // UART communication initialization (to UartMonitor)
    Serial.begin(UART_BAUD);
    Serial1.begin(115200);
    
    // Arduino USB-MIDI initialization
    MIDI.begin(MIDI_CHANNEL_ALL);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleControlChange(handleControlChange);
    MIDI.setHandleProgramChange(handleProgramChange);
    MIDI.setHandlePitchBend(handlePitchBend);
    
    // Startup message via UART (not USB to avoid conflicts)
    Serial1.println("LittleJoe Arduino USB MIDI Monitor Ready");
    delay(100);
}

void loop() {
    // Process MIDI input
    MIDI.read();
    
    // Process pending events safely in main loop
    if (midi_event_pending) {
        processPendingEvent();
        midi_event_pending = false;
    }
    
    // Debug heartbeat every 3 seconds
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 3000) {
        Serial1.println("[DEBUG] LittleJoe heartbeat");
        lastHeartbeat = millis();
    }
    delay(1);
}

// Safe event processing in main loop (not in interrupt context)
void processPendingEvent() {
    StaticJsonDocument<200> json;
    json["timestamp"] = pending_event.timestamp;
    json["channel"] = pending_event.channel;
    
    switch (pending_event.type) {
        case 1: // Note On
            json["type"] = "note_on";
            json["note"] = pending_event.data1;
            json["velocity"] = pending_event.data2;
            break;
            
        case 2: // Note Off
            json["type"] = "note_off";
            json["note"] = pending_event.data1;
            json["velocity"] = pending_event.data2;
            break;
            
        case 3: // Control Change
            json["type"] = "control_change";
            json["controller"] = pending_event.data1;
            json["value"] = pending_event.data2;
            break;
            
        case 4: // Program Change
            json["type"] = "program_change";
            json["program"] = pending_event.data1;
            break;
            
        case 5: // Pitch Bend
            json["type"] = "pitch_bend";
            json["value"] = pending_event.data3;
            break;
    }
    
    serializeJson(json, Serial1);
    Serial1.println();
}

// MIDI Event Handlers (keep minimal - no USB operations!)
void handleNoteOn(byte channel, byte note, byte velocity) {
    if (!midi_event_pending) {  // Avoid overwrite
        pending_event.type = 1;
        pending_event.channel = channel;
        pending_event.data1 = note;
        pending_event.data2 = velocity;
        pending_event.timestamp = millis();
        midi_event_pending = true;
    }
}

void handleNoteOff(byte channel, byte note, byte velocity) {
    if (!midi_event_pending) {
        pending_event.type = 2;
        pending_event.channel = channel;
        pending_event.data1 = note;
        pending_event.data2 = velocity;
        pending_event.timestamp = millis();
        midi_event_pending = true;
    }
}

void handleControlChange(byte channel, byte controller, byte value) {
    if (!midi_event_pending) {
        pending_event.type = 3;
        pending_event.channel = channel;
        pending_event.data1 = controller;
        pending_event.data2 = value;
        pending_event.timestamp = millis();
        midi_event_pending = true;
    }
}

void handleProgramChange(byte channel, byte program) {
    if (!midi_event_pending) {
        pending_event.type = 4;
        pending_event.channel = channel;
        pending_event.data1 = program;
        pending_event.timestamp = millis();
        midi_event_pending = true;
    }
}

void handlePitchBend(byte channel, int bend) {
    if (!midi_event_pending) {
        pending_event.type = 5;
        pending_event.channel = channel;
        pending_event.data3 = bend;
        pending_event.timestamp = millis();
        midi_event_pending = true;
    }
}