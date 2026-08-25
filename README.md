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

*(See the attached simulation screenshot in the repository for the circuit layout and real-time logic pulse waveform).*
