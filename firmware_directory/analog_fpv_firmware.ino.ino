/*
 * ANALOG FPV RACER - MAIN FIRMWARE (REFINED)
 * Architecture: NTSC Sync Extraction + Hardware Interrupt (ISR) Timing
 * MCU: ESP32-C3 SuperMini
 */

#include "esp_timer.h" // For high-precision hardware timer

// --- PIN DEFINITIONS ---
#define PIN_CSYNC 9       
#define PIN_VSYNC 8       
#define PIN_LINE_DETECT 2 

// Motor Driver Pins (TB6612FNG)
#define PIN_PWMA 1        
#define PIN_AIN2 0        
#define PIN_AIN1 3        
#define PIN_BIN1 4        
#define PIN_BIN2 10       
#define PIN_PWMB 20       

// --- VOLATILE TIMING & COORDINATE VARIABLES ---
volatile uint64_t line_start_time = 0;
volatile uint32_t left_x_time = 0;
volatile uint32_t right_x_time = 0;
volatile uint16_t current_y_line = 0; 
volatile int lines_detected = 0;
volatile bool valid_track_data = false;

// --- TUNING PARAMETERS ---
float Kp = 3.0;           
int base_speed = 120;     
const uint16_t TARGET_Y_MIN = 150; // Look-ahead vertical window start
const uint16_t TARGET_Y_MAX = 180; // Look-ahead vertical window end

// --- INTERRUPT SERVICE ROUTINES (ISRs) ---
void IRAM_ATTR onVsync() {
    current_y_line = 0; // Reset vertical line counter at new frame
}

void IRAM_ATTR onHsync() {
    current_y_line++; 
    line_start_time = esp_timer_get_time(); // Sub-microsecond precision
    lines_detected = 0; 
    valid_track_data = false;
}

void IRAM_ATTR onLineDetect() {
    // Only process if within our vertical look-ahead window
    if (current_y_line < TARGET_Y_MIN || current_y_line > TARGET_Y_MAX) return;

    uint64_t elapsed = esp_timer_get_time() - line_start_time;
    
    // Tightened NTSC active video window (ignore edge blanking artifacts)
    if (elapsed > 3 && elapsed < 50) { 
        if (lines_detected == 0) {
            left_x_time = elapsed;   
            lines_detected++;
        } else if (lines_detected == 1) {
            right_x_time = elapsed;  
            lines_detected++;
            valid_track_data = true; 
        }
    }
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);

    pinMode(PIN_CSYNC, INPUT);
    pinMode(PIN_VSYNC, INPUT);
    pinMode(PIN_LINE_DETECT, INPUT);

    pinMode(PIN_PWMA, OUTPUT);
    pinMode(PIN_AIN1, OUTPUT);
    pinMode(PIN_AIN2, OUTPUT);
    pinMode(PIN_PWMB, OUTPUT);
    pinMode(PIN_BIN1, OUTPUT);
    pinMode(PIN_BIN2, OUTPUT);

    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_AIN2, LOW);
    digitalWrite(PIN_BIN1, HIGH);
    digitalWrite(PIN_BIN2, LOW);
    analogWrite(PIN_PWMA, 0);
    analogWrite(PIN_PWMB, 0);

    attachInterrupt(digitalPinToInterrupt(PIN_VSYNC), onVsync, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_CSYNC), onHsync, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_LINE_DETECT), onLineDetect, FALLING);
}

// --- MAIN LOOP ---
void loop() {
    if (valid_track_data) {
        float track_center_time = (left_x_time + right_x_time) / 2.0;
        float camera_center_time = 26.35; 
        
        float error = camera_center_time - track_center_time;
        float steering_correction = Kp * error;
        
        int left_pwm = base_speed - steering_correction;
        int right_pwm = base_speed + steering_correction;
        
        left_pwm = constrain(left_pwm, 0, 255);
        right_pwm = constrain(right_pwm, 0, 255);
        
        analogWrite(PIN_PWMA, left_pwm); 
        analogWrite(PIN_PWMB, right_pwm);
        
        valid_track_data = false; 
    }
}