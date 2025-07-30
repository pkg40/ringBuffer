/*
 * ringBuffer Library - Basic Usage Example
 * 
 * This example demonstrates basic ring buffer operations:
 * - Creating ring buffers for different data types
 * - Push/pop operations
 * - Buffer state checking
 * - Peek functionality
 * 
 * Compatible with ESP32, ESP8266, and Arduino platforms
 */

#include <Arduino.h>
#include "ringBuffer.hpp"

// Create ring buffers for different data types
ringBuffer<int, 8> intBuffer;
ringBuffer<float, 16> floatBuffer;

// Custom struct example
struct SensorReading {
    float temperature;
    float humidity;
    uint32_t timestamp;
};
ringBuffer<SensorReading, 32> sensorBuffer;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ringBuffer Library - Basic Usage Example");
    Serial.println("========================================");
    
    // Demonstrate integer buffer
    demonstrateIntBuffer();
    
    // Demonstrate float buffer
    demonstrateFloatBuffer();
    
    // Demonstrate struct buffer
    demonstrateSensorBuffer();
    
    Serial.println("\nExample completed!");
}

void loop() {
    // Nothing to do in loop for this example
    delay(1000);
}

void demonstrateIntBuffer() {
    Serial.println("\n1. Integer Buffer Demo:");
    Serial.println("-----------------------");
    
    // Push some integers
    for (int i = 1; i <= 5; i++) {
        bool success = intBuffer.push(i * 10);
        Serial.printf("Push %d: %s\n", i * 10, success ? "OK" : "FAILED");
    }
    
    // Check buffer state
    Serial.printf("Buffer size: %d/%d\n", intBuffer.size(), intBuffer.capacity());
    Serial.printf("Is empty: %s\n", intBuffer.isEmpty() ? "Yes" : "No");
    Serial.printf("Is full: %s\n", intBuffer.isFull() ? "Yes" : "No");
    
    // Peek at next value
    int peeked;
    if (intBuffer.peek(peeked)) {
        Serial.printf("Next value (peek): %d\n", peeked);
    }
    
    // Pop values
    Serial.println("Popping values:");
    int value;
    while (intBuffer.pop(value)) {
        Serial.printf("  Popped: %d\n", value);
    }
    
    Serial.printf("Buffer now empty: %s\n", intBuffer.isEmpty() ? "Yes" : "No");
}

void demonstrateFloatBuffer() {
    Serial.println("\n2. Float Buffer Demo:");
    Serial.println("---------------------");
    
    // Push some float values
    float values[] = {3.14, 2.718, 1.414, 0.577, 1.618};
    for (int i = 0; i < 5; i++) {
        floatBuffer.push(values[i]);
        Serial.printf("Pushed: %.3f\n", values[i]);
    }
    
    Serial.printf("Buffer contains %d values\n", floatBuffer.size());
    
    // Pop half the values
    Serial.println("Popping 2 values:");
    for (int i = 0; i < 2; i++) {
        float val;
        if (floatBuffer.pop(val)) {
            Serial.printf("  Popped: %.3f\n", val);
        }
    }
    
    // Add more values
    Serial.println("Adding more values:");
    floatBuffer.push(9.876);
    floatBuffer.push(5.432);
    Serial.printf("Buffer now contains %d values\n", floatBuffer.size());
    
    // Clear the buffer
    floatBuffer.clear();
    Serial.println("Buffer cleared");
    Serial.printf("Buffer now empty: %s\n", floatBuffer.isEmpty() ? "Yes" : "No");
}

void demonstrateSensorBuffer() {
    Serial.println("\n3. Sensor Data Buffer Demo:");
    Serial.println("---------------------------");
    
    // Create some sensor readings
    SensorReading readings[] = {
        {23.5, 65.2, millis()},
        {24.1, 63.8, millis() + 1000},
        {23.9, 64.5, millis() + 2000}
    };
    
    // Push sensor readings
    for (int i = 0; i < 3; i++) {
        bool success = sensorBuffer.push(readings[i]);
        Serial.printf("Sensor reading %d: T=%.1f°C, H=%.1f%%, Time=%lu - %s\n",
                     i + 1,
                     readings[i].temperature,
                     readings[i].humidity,
                     readings[i].timestamp,
                     success ? "Stored" : "Failed");
    }
    
    // Process readings
    Serial.println("Processing stored readings:");
    SensorReading reading;
    int count = 1;
    while (sensorBuffer.pop(reading)) {
        Serial.printf("  Reading %d: %.1f°C, %.1f%% @ %lu\n",
                     count++,
                     reading.temperature,
                     reading.humidity,
                     reading.timestamp);
    }
    
    Serial.printf("Sensor buffer capacity: %d readings\n", sensorBuffer.capacity());
}
