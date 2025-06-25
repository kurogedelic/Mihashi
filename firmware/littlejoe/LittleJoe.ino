/*
 * LittleJoe - TinyUSB MIDI Monitor
 * XIAO SAMD21 TinyUSB MIDI → UART ASCII Monitor
 * 
 * Hardware: Seeed XIAO SAMD21
 * Function: Receive USB MIDI from Mihashi, output ASCII to UartMonitor via UART
 * 
 * Connections:
 * - USB-C: Mihashi USB MIDI input
 * - D6 (PA04): UART TX → UartMonitor RX
 * - D7 (PA05): UART RX ← UartMonitor TX (future use)
 * - GND/3V3: Power/Ground
 */

#include <TinyUSB.h>
#include <MIDI.h>
#include <ArduinoJson.h>

// TinyUSB MIDI instance
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// Configuration
#define UART_BAUD 115200
#define MIDI_CHANNEL_ALL MIDI_CHANNEL_OMNI

void setup() {
    // UART communication initialization (to UartMonitor)
    Serial1.begin(UART_BAUD);
    
    // TinyUSB MIDI initialization
    TinyUSB_Device_Init(0);
    
    // MIDI configuration
    MIDI.begin(MIDI_CHANNEL_ALL);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleControlChange(handleControlChange);
    MIDI.setHandleProgramChange(handleProgramChange);
    MIDI.setHandlePitchBend(handlePitchBend);
    MIDI.setHandleSystemExclusive(handleSystemExclusive);
    
    // Startup message
    Serial1.println("LittleJoe TinyUSB MIDI Monitor Ready");
    delay(100);
}

void loop() {
    // TinyUSB device task
    TinyUSB_Device_Task();
    
    // MIDI message processing
    if (MIDI.read()) {
        // Message processed by handlers
    }
    
    delay(1);
}

// MIDI Event Handlers
void handleNoteOn(byte channel, byte note, byte velocity) {
    StaticJsonDocument<200> json;
    json["type"] = "note_on";
    json["channel"] = channel;
    json["note"] = note;
    json["velocity"] = velocity;
    json["timestamp"] = millis();
    
    serializeJson(json, Serial1);
    Serial1.println();
}

void handleNoteOff(byte channel, byte note, byte velocity) {
    StaticJsonDocument<200> json;
    json["type"] = "note_off";
    json["channel"] = channel;
    json["note"] = note;
    json["velocity"] = velocity;
    json["timestamp"] = millis();
    
    serializeJson(json, Serial1);
    Serial1.println();
}

void handleControlChange(byte channel, byte controller, byte value) {
    StaticJsonDocument<200> json;
    json["type"] = "control_change";
    json["channel"] = channel;
    json["controller"] = controller;
    json["value"] = value;
    json["timestamp"] = millis();
    
    serializeJson(json, Serial1);
    Serial1.println();
}

void handleProgramChange(byte channel, byte program) {
    StaticJsonDocument<200> json;
    json["type"] = "program_change";
    json["channel"] = channel;
    json["program"] = program;
    json["timestamp"] = millis();
    
    serializeJson(json, Serial1);
    Serial1.println();
}

void handlePitchBend(byte channel, int bend) {
    StaticJsonDocument<200> json;
    json["type"] = "pitch_bend";
    json["channel"] = channel;
    json["value"] = bend;
    json["timestamp"] = millis();
    
    serializeJson(json, Serial1);
    Serial1.println();
}

void handleSystemExclusive(byte* array, unsigned size) {
    StaticJsonDocument<400> json;
    json["type"] = "sysex";
    json["size"] = size;
    json["timestamp"] = millis();
    
    JsonArray data = json.createNestedArray("data");
    for (unsigned i = 0; i < size && i < 50; i++) {
        data.add(array[i]);
    }
    
    if (size > 50) {
        json["truncated"] = true;
    }
    
    serializeJson(json, Serial1);
    Serial1.println();
}