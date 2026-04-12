# Basilian-01 Rocket Avionics

![Rocket Avionics System](images/obp.jpg)

## Overview
**DEVELOPMENT OF A FAULT-TOLERANT ROCKET AVIONICS HARDWARE SYSTEM FOR FLIGHT CONTROL AND AEROSOL MONITORING**

The hardware design and integration of Basilian-01 present a model rocket avionics system developed for reliable flight monitoring, effective recovery control, and atmospheric aerosol data acquisition. This project is centered on a dual-redundant embedded hardware architecture to enhance mission reliability and eliminate single-point failures during critical flight phases.

### Team Members
* AADIL MUHAMMED
* BEVAL PHILIP MATHEW
* RAHUL B
* SIBI B JOHN

*Mar Baselios Christian College of Engineering and Technology*

## Abstract
The avionics hardware integrates multiple subsystems including barometric altitude sensors, an inertial measurement unit (IMU), GPS module, LoRa-based telemetry transceiver, SD card data logger, servo-based parachute deployment mechanism, and an aerosol sensing module for in-flight atmospheric particulate measurement. Aerosol data is collected during ascent and descent phases to enable analysis of particulate concentration variations with altitude. All recovery-critical operations operate autonomously on-board, independent of ground systems. Emphasis is placed on robust power distribution, signal interfacing, sensor integration, and redundancy-oriented hardware design.

## Features
- **Fault-Tolerant Architecture**: Dual-redundant ESP32-based flight controllers.
- **Flight Event Detection**: Finite State Machine for Launch, Apogee, and Landing detection based on dual-sensor voting.
- **Autonomous Recovery**: Servo-controlled parachute deployment at apogee, with backup logic triggers.
- **Payload Module**: Aerosol sensing capability using optical scattering for environmental data logging.
- **Long-Range Telemetry**: 433 MHz LoRa transceiver for high-reliability data links.
- **Machine Intelligence**: Edge-processing capabilities for aerosol feature extraction (moving average filtering and decision tree classification).
- **Redundant Power Distribution**: Multiple UBECs on dedicated Li-Po batteries with isolated logic/actuator rails.
- **Ground Control Dashboard**: A full real-time Next.js and Flask/WebSocket web dashboard to visualize telemetry, map tracking, and command execution.

## Repository Structure

- 📖 **[`docs/project_phase_2_report.md`](docs/project_phase_2_report.md)** : **The Complete Project Phase II Academic Report**
- `docs/` : Detailed documentation (Hardware, Software Architecture, etc.)
- `firmware/flight_controller/` : Dual ESP32 On-board flight firmware.
- `firmware/ground_station/` : ESP32 telemetry receiver firmware bridging LoRa and the Web Dashboard.
- `Web_Dashboard/` : Front-end (Next.js) and Back-end (Flask) sources.
- `images/`: Placeholders for block diagrams and photos during flight testing.
- `extras/`: Previous project reports and references.

## Getting Started
See the [Hardware Design Document](docs/hardware_design.md) for pinouts and schematic references.

1. **Flight Controller Firmware**: Open `firmware/flight_controller/flight_controller.ino` using the Arduino IDE. Flash to ESP32-A and ESP32-B. Ensure the required libraries (ESP32Servo, Adafruit_BME280, TinyGPSPlus, LoRa) are installed. 
2. **Ground Station Firmware**: Flash `firmware/ground_station/ground_station.ino` to the Ground Control ESP32.
3. **Dashboard Setup**: Proceed to the `Web_Dashboard` directory to deploy the React front-end and Flask back-end for tracking.

## Authors
Developed as part of a Bachelor of Technology curriculum requirement in Electronics and Communication Engineering (2025-2026).
