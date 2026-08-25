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
