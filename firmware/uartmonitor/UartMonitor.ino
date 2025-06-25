/*
 * UartMonitor - UART to USB Serial Bridge
 * XIAO SAMD21 UART → USB Serial Bridge
 * 
 * Hardware: Seeed XIAO SAMD21
 * Function: Receive UART ASCII from LittleJoe, forward to GhostPC via USB Serial
 * 
 * Connections:
 * - USB-C: GhostPC USB Serial connection
 * - D6 (PA04): UART RX ← LittleJoe TX
 * - D7 (PA05): UART TX → LittleJoe RX (future use)
 * - GND/3V3: Power/Ground
 */

#include <ArduinoJson.h>

// Configuration
#define UART_BAUD 115200
#define USB_BAUD 115200
#define BUFFER_SIZE 512

// Buffer management
char uartBuffer[BUFFER_SIZE];
int bufferIndex = 0;

// Statistics
unsigned long messageCount = 0;
unsigned long lastStatsTime = 0;
unsigned long bytesReceived = 0;

// Filtering options
bool enableNoteMessages = true;
bool enableControlChange = true;
bool enableProgramChange = true;
bool enablePitchBend = true;
bool enableSysEx = true;
bool enableTimestamp = true;
bool enableStatistics = false;

void setup() {
    // USB Serial communication (to GhostPC)
    Serial.begin(USB_BAUD);
    while (!Serial) delay(10);
    
    // UART communication (from LittleJoe)
    Serial1.begin(UART_BAUD);
    
    // Startup messages
    Serial.println("=== UartMonitor USB-Serial Bridge ===");
    Serial.println("Ready to receive MIDI data from LittleJoe");
    Serial.println("Format: JSON MIDI Monitor Data");
    Serial.println("=====================================");
    
    lastStatsTime = millis();
}

void loop() {
    // UART → USB Serial transfer (main function)
    while (Serial1.available()) {
        char c = Serial1.read();
        bytesReceived++;
        
        if (c == '\n') {
            // Line complete - process and forward
            uartBuffer[bufferIndex] = '\0';
            processLine(uartBuffer);
            bufferIndex = 0;
        } else if (bufferIndex < BUFFER_SIZE - 1) {
            uartBuffer[bufferIndex++] = c;
        } else {
            // Buffer overflow - reset
            Serial.println("[ERROR] Buffer overflow, resetting");
            bufferIndex = 0;
        }
    }
    
    // USB Serial → UART transfer (future bidirectional support)
    while (Serial.available()) {
        char c = Serial.read();
        Serial1.write(c);
    }
    
    // Periodic statistics
    if (enableStatistics && millis() - lastStatsTime > 10000) {
        printStatistics();
        lastStatsTime = millis();
    }
    
    delay(1);
}

void processLine(const char* line) {
    // Skip empty lines
    if (strlen(line) == 0) return;
    
    // Parse JSON for filtering
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, line);
    
    if (!error) {
        // Valid JSON - apply filters
        const char* type = doc["type"];
        
        if (!shouldForwardMessage(type)) {
            return; // Filtered out
        }
        
        // Add enhanced timestamp if enabled
        if (enableTimestamp) {
            Serial.print("[");
            Serial.print(millis());
            Serial.print("] ");
        }
        
        messageCount++;
    } else {
        // Invalid JSON - forward as raw data with warning
        Serial.print("[WARN] JSON Parse Error: ");
    }
    
    // Forward the line
    Serial.println(line);
}

bool shouldForwardMessage(const char* type) {
    if (type == nullptr) return true; // Forward unknown types
    
    if ((strcmp(type, "note_on") == 0 || strcmp(type, "note_off") == 0) && !enableNoteMessages) {
        return false;
    }
    if (strcmp(type, "control_change") == 0 && !enableControlChange) {
        return false;
    }
    if (strcmp(type, "program_change") == 0 && !enableProgramChange) {
        return false;
    }
    if (strcmp(type, "pitch_bend") == 0 && !enablePitchBend) {
        return false;
    }
    if (strcmp(type, "sysex") == 0 && !enableSysEx) {
        return false;
    }
    
    return true;
}

void printStatistics() {
    Serial.println("=== UartMonitor Statistics ===");
    Serial.print("Messages processed: ");
    Serial.println(messageCount);
    Serial.print("Bytes received: ");
    Serial.println(bytesReceived);
    Serial.print("Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    Serial.print("Free memory: ");
    Serial.println(freeMemory());
    Serial.println("==============================");
}

// Simple memory check (approximate)
int freeMemory() {
    char stack_dummy = 0;
    return &stack_dummy - sbrk(0);
}

// Enhanced line processing with error recovery
void processLineEnhanced(const char* line) {
    StaticJsonDocument<400> enhancedDoc;
    
    // Try to parse original JSON
    StaticJsonDocument<300> originalDoc;
    DeserializationError error = deserializeJson(originalDoc, line);
    
    if (!error) {
        // Successfully parsed - enhance with metadata
        enhancedDoc["original"] = originalDoc;
        enhancedDoc["bridge_timestamp"] = millis();
        enhancedDoc["message_id"] = messageCount;
        enhancedDoc["bridge_status"] = "ok";
        
        serializeJson(enhancedDoc, Serial);
        Serial.println();
    } else {
        // Parse error - create error report
        enhancedDoc["bridge_status"] = "parse_error";
        enhancedDoc["error_code"] = error.c_str();
        enhancedDoc["raw_data"] = line;
        enhancedDoc["bridge_timestamp"] = millis();
        enhancedDoc["message_id"] = messageCount;
        
        serializeJson(enhancedDoc, Serial);
        Serial.println();
    }
}