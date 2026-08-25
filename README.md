# Analog FPV RC Racer - Hardware Vision System

## Overview
This repository contains the hardware design and Gerber files for a 1:32 scale FPV RC racing car. The core objective is to achieve ultra-fast line detection on a black/white track by shifting the vision processing from software to a dedicated analog hardware front-end.

## Hardware Implementation
* **Power Distribution (Stage 1):** Steps down a 3S LiPo battery to safely power an ESP32-C3 SuperMini, dual drive motors, a motor driver, and the analog camera.
* **Sync Separation (Stage 2):** Utilizes an LM1881 video sync separator to extract vertical and horizontal timing signals directly from the camera's raw NTSC CVBS feed.
* **Analog Filtering (Stage 3):** Hardware-level filtering networks distinguish high-contrast white boundaries from the black track surface to eliminate environmental noise.
* **Data Extraction (Stage 4):** The conditioned sync signals and line-detection triggers are routed to the ESP32-C3, allowing the microcontroller to calculate X/Y coordinates with minimal computational overhead.

## Design Assumptions
* **Lighting:** The track environment will have relatively consistent lighting to ensure the analog contrast thresholds remain stable.
* **Battery:** The system assumes a standard 3S LiPo voltage curve (12.6V fully charged down to ~11.1V nominal).
* **Camera Protocol:** The analog camera strictly operates on the NTSC standard for proper timing extraction.
## Stage 2 & 3: Hardware Sync & Filtering Simulation
To validate the analog front-end without a physical board, the threshold and filtering logic was simulated.

**Stage 2: Camera Output & Sync Extraction**
Simulating the complex timing intervals of a raw NTSC CVBS signal is prone to software inaccuracies, so Stage 2 is handled entirely at the hardware level. The custom PCB utilizes a dedicated **LM1881 Video Sync Separator IC**. This chip ingests the raw 1Vpp composite video feed and cleanly strips out the Composite Sync (CSYNC) and Vertical Sync (VSYNC) pulses. These logic-level timing signals are routed directly to the ESP32-C3 hardware interrupt pins, fulfilling the synchronization extraction requirement with zero microcontroller overhead.

**Stage 3: Analog Threshold & Noise Filtering**
To detect the white track boundaries and eliminate false positives from environmental noise, a discrete analog front-end was designed and simulated:
1. **RC Low-Pass Filter (1kΩ / 100nF):** Strips away high-frequency camera static and track noise from the raw video signal.
2. **Threshold Comparator:** A comparator thresholded at 600mV distinguishes high-contrast white lines from the dark track (simulated as a 1V AC source with a 0.5V DC offset).
3. **Schmitt Trigger (Hysteresis):** A 100kΩ feedback loop ensures that even if shadows cause the voltage to hover near the threshold, the digital output remains cleanly latched without high-frequency fluttering.

## Stage 4: Filter to Microcontroller (X/Y Extraction Firmware)
To translate the analog hardware signals into actionable steering data, custom C++ firmware was developed for the **ESP32-C3 SuperMini** using the Arduino IDE. 

Instead of relying on heavy, slow image processing, the system extracts coordinates in real-time using lightweight Hardware Interrupt Service Routines (ISRs):

* **Y-Coordinate (Vertical):** The `VSYNC` interrupt resets the frame line counter to zero. The `CSYNC` interrupt triggers at the start of every new scan line, incrementing the Y-coordinate.
* **X-Coordinate (Horizontal):** Every `CSYNC` pulse also starts a microsecond hardware timer. When the comparator fires a `LINE_DETECTED` pulse (indicating the white track boundary), the timer is paused. This microsecond offset directly correlates to the X-coordinate on the screen.
* **Result:** The microcontroller derives the precise (X, Y) track boundaries instantly, leaving maximum CPU resources available for the high-speed PID motor control loop.

## Bonus Stage 1: Hardware Obstacle Detection
To achieve obstacle and collision detection without adding processing overhead to the ESP32, a **Window Comparator** architecture was integrated into the analog front-end. 

A second op-amp was placed in parallel with the line-detection circuit. The filtered video signal is routed to its inverting (-) input, with a 300mV reference voltage on the non-inverting (+) input. 
* If the camera scans an object that is darker than the track (e.g., a physical blockade casting a shadow), the voltage dips below 300mV.
* The comparator instantly fires a dedicated `OBSTACLE_DETECTED` hardware interrupt to the ESP32, triggering an immediate braking routine before the microcontroller even processes the next frame.

## Bonus Stage 2: Microcontroller-Free Direct Analog Control
To prove system resilience and explore pure hardware computation, a completely microcontroller-free analog control loop was designed:
* **Analog Position Averaging:** The high-frequency digital pulses from the hardware filter are passed through an analog RC integrator network to convert pulse-width variations into a smooth, proportional steering DC voltage.
* **Potentiometer Tuning Network:** Adjustable trim potentiometers establish baseline reference voltages for center-steering calibration.
* **Differential Motor Mixing:** The processed analog signals drive an operational amplifier differential network, adjusting left and right motor speeds in real-time based strictly on analog voltage comparison—achieving closed-loop track following with zero software latency.*(See the updated dual-scope simulation screenshot in the repository).**(The compiled `.ino` firmware file is available in this repository).**(See the attached simulation screenshot in the repository for the circuit layout and real-time logic pulse waveform).*
