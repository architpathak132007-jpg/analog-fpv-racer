# Analog FPV RC Racing Car (1:32 Scale)

## Project Overview
This repository contains the hardware and firmware design for a high-speed 1:32 FPV RC racing car. Designed around the ESP32-C3 SuperMini, the system bypasses the microcontroller's processing limitations by utilizing a custom analog frontend to process an NTSC CVBS camera feed in real-time. The analog circuit handles all white-line detection and synchronization extraction, sending only lightweight, time-critical interrupts to the MCU for X/Y coordinate calculation.

## CAD Software
* Designed and simulated using **EasyEDA**.

## Hardware Architecture (Analog Frontend)
### Stage 1: Power Distribution
A 3S LiPo battery provides the main power rail. The raw 11.1V-12.6V is routed directly to the TB6612FNG motor driver to maximize drive motor velocity. An MP1584EN buck converter safely steps the battery voltage down to a stable +5V to power the analog ICs and the ESP32-C3. 

### Stage 2: Synchronization Extraction
An LM1881 Video Sync Separator extracts the composite and vertical sync pulses from the raw NTSC CVBS signal. 
* **Logic Level Protection:** Because the LM1881 runs at 5V, its CSYNC and VSYNC outputs are stepped down to safe ~3.1V logic levels using 2kΩ/3.3kΩ resistor voltage dividers before entering the 3.3V-tolerant ESP32-C3 GPIOs.

### Stage 3: Hardware Filtering & Line Detection
High-speed line detection is handled purely in hardware via an LM393 voltage comparator.
* The CVBS video signal is conditioned and fed into the comparator. 
* A rigid voltage divider sets a **430mV detection threshold**. 
* The LM393 features an open-collector output pulled up to the ESP32's 3.3V rail. When the camera detects a high-contrast white line, the LM393 output drives low, sending a clean, instantaneous 3.3V pulse to the MCU.

## Software Architecture (Minimal Overhead)
### Stage 4: Hardware to Microcontroller Interface
To satisfy the strict "minimal computation" constraint, the ESP32-C3 does not sample video data. Instead, it relies on hardware interrupts (`IRAM_ATTR`). 
* The LM1881 sync pulses trigger the start of a frame and scanline.
* The LM393 pulse triggers a hardware timer capture (`esp_timer_get_time()`). 
* By measuring the microsecond delay between the horizontal sync pulse and the comparator pulse, the ESP32-C3 calculates the exact X/Y coordinates of the track boundaries with virtually zero main-loop overhead.

## Design Assumptions
* **Constant Lighting:** The track environment maintains relatively consistent lighting, allowing the fixed 430mV analog comparator threshold to reliably distinguish the white lines from the black track.
* **NTSC Timing:** The system calculates steering deviation by assuming the physical center of the track aligns perfectly with the center of the active NTSC video scanline (roughly 26.35µs after the horizontal sync pulse).
* **Track Contrast:** The track is strictly a plain black surface with bright white boundaries, eliminating the need for complex object-classification algorithms.
* ## Future Work & Bonus Stage Concepts

While this iteration successfully implements the ESP32-C3 pipeline, the architecture is designed with the following analog expansions in mind for future revisions:

### Bonus Objective: Obstacle & Opponent Detection
To detect physical obstacles or competing vehicles without digital object classification, we can utilize the **second, unused channel of the LM393 dual comparator** (Pins 5, 6, and 7). 
* While Channel 1 is tuned to a high 430mV threshold to detect peak-white track boundaries, Channel 2 can be configured with a lower threshold (e.g., 200mV) using a separate voltage divider. 
* This lower threshold would trigger on the mid-level greys caused by the shadows or chassis of competing vehicles on the black track. By comparing the timing of the white-line interrupt vs. the shadow interrupt, the system can flag anomalies on the track surface.

### Bonus Stage: Microcontroller-Free Direct Control
To achieve absolute zero-latency steering, the ESP32-C3 can be completely bypassed by routing the LM393 output into a pure analog control loop:
* **Analog Integration:** The high-speed 3.3V pulses from the LM393 can be fed into an Operational Amplifier configured as an integrator. The timing of the pulse (relative to the horizontal sync) would generate a proportional DC voltage representing the car's off-center error.
* **Analog PID & PWM:** This error voltage would pass through an analog PID network tuned via physical potentiometers. The resulting control voltage would then feed a 555-timer (or op-amp triangle wave generator) configured as a custom PWM generator, driving the TB6612FNG motor driver directly without a single line of code.
