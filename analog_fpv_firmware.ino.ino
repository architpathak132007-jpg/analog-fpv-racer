#include <Arduino.h>

// --- GPIO Pin Definitions ---
const uint8_t PIN_VSYNC  = 4; // Connected to LM1881 VSYNC (Pin 3)
const uint8_t PIN_CSYNC  = 5; // Connected to LM1881 CSYNC (Pin 1)
const uint8_t PIN_LINE_DET = 6; // Connected to Analog Comparator Output

// --- Real-time Coordinate Registers ---
volatile uint16_t current_Y = 0;
volatile uint32_t line_start_time = 0;

// Extracted boundary coordinates
volatile uint16_t detected_X = 0;
volatile uint16_t detected_Y = 0;
volatile bool line_found_flag = false;

// --- Hardware Interrupt Service Routines (ISRs) ---

// 1. Resets Y-coordinate at the start of every frame
void IRAM_ATTR isr_vsync() {
    current_Y = 0;
}

// 2. Increments Y-coordinate and starts X-timer for each horizontal line
void IRAM_ATTR isr_csync() {
    current_Y++;
    line_start_time = micros(); // Capture microsecond timestamp for X-coordinate
}

// 3. Captures X/Y position immediately when white line is detected by comparator
void IRAM_ATTR isr_line_detected() {
    uint32_t current_time = micros();
    detected_X = (uint16_t)(current_time - line_start_time); // Time offset across scan line
    detected_Y = current_Y;
    line_found_flag = true;
}

void setup() {
    Serial.begin(115200);

    // Configure input pins
    pinMode(PIN_VSYNC, INPUT);
    pinMode(PIN_CSYNC, INPUT);
    pinMode(PIN_LINE_DET, INPUT);

    // Attach Hardware Interrupts
    attachInterrupt(digitalPinToInterrupt(PIN_VSYNC), isr_vsync, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_CSYNC), isr_csync, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_LINE_DET), isr_line_detected, RISING);
}

void loop() {
    if (line_found_flag) {
        line_found_flag = false;
        
        // Print extracted track coordinates for steering calculation
        Serial.printf("Boundary Line Detected at X: %u µs, Y: Line %u\n", detected_X, detected_Y);
        
        // TODO: Pass (detected_X, detected_Y) to Motor PID loop for high-speed steering adjustment
    }
}