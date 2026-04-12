# Firmware Code Architecture & Details

This document outlines the detailed functionality, methodologies, and logic incorporated within the three primary `.ino` firmware sketches of the Basilian-01 avionics architecture.

## 1. Flight Controller (`firmware/flight_controller/flight_controller.ino`)
This firmware operates as the Central Processing Unit of the rocket. It manages the sensors, calculates metrics, runs the Finite State Machine (FSM), and pushes redundant data to both LoRa and ESP-NOW.

**Key Components:**
* **Sensor Acquisition**: Reads high-frequency data from the MPU6050 (Acceleration/Gyro), BME280 (Temperature/Pressure/Barometric Altitude), and NEO-6M GPS (Positional Tracking + IST Timestamps offset dynamically from UTC timezone tracking).
* **Battery Monitoring**: Reads voltage through a predetermined resistor tracking network to gauge precise operational capacity remaining, enabling brownout mitigation.
* **Finite State Machine (FSM)**:
    * `STATE_CAL`: The rocket establishes sea-level baseline pressure metrics and calibrates the MPU gyroscope's zero offsets.
    * `STATE_ARMED`: Wait block until rapid vertical acceleration triggers flight.
    * `STATE_FLIGHT`: Monitors peak altitude continuous gain (`maxAltCm`).
    * `STATE_APOGEE`: Automatically transitions when current barometric altitude drops significantly (`APOGEE_DROP_CM`) below the prior peak. Triggers dual-servo action to release parachute mechanisms to `150, 150` angles.
* **Dual Telemetry Redundancy**: 
    1. A compiled 22-field CSV string is forwarded to the LoRa SX1278 transceiver and blasted 433MHz down to the ground.
    2. A secondary local packet containing the exact same String is serialized and pushed via **ESP-NOW** at high bandwidth directly to the secondary logger board (`ESP32-B`).

## 2. SD Logger (`firmware/sd_logger/sd_logger.ino`)
This module provides a fail-safe mechanism to ensure zero data loss occurs if LoRa communication signals drop. It acts concurrently as the data acquisition suite for the **Aerosol Payload**.

**Key Components:**
* **Asynchronous Listening**: Binds an `esp_now_recv_cb` event hook to listen exclusively to the MAC address of the primary flight controller (`0xC0, 0xCD, 0xD6, 0x85, 0x6E, 0xCC`).
* **Environmental Payload Integration**: Connects an `SDS011` laser particulate sensor via UART to capture `PM2.5` and `PM10` volumetric aerosol concentrations.
* **Data Fusion**: When an ESP-NOW packet arrives, it extracts the 420-byte payload, merges the live `PM2.5` and `PM10` readings, tacks on a local monotonic timestamp, and flushes it immediately into the SPI SD Card (`/log_{millis}.csv`).

## 3. Ground Station (`firmware/ground_station/ground_station.ino`)
This firmware operates on the ground control rig interfacing the LoRa radio with the Next.js / Python USB backend.

**Key Components:**
* **Packet Validation**: Reads incoming LoRa packets. To avoid decoding corrupted packets, it aggressively parses the CSV commas. If the count matches the `TELEMETRY_FIELDS` definition, it allows the packet through and subsequently appends the `RSSI` (Radio Signal Strength Indicator) dbM reading so the ground team knows network degradation real-time.
* **Command Translation**: Listens for HTTP-originating commands on the USB `Serial` bus (`PING`, `RELEASE`, `HOLD`). Once validated, it pushes these strictly back *up* the LoRa pipeline.
* **ACK Lifecycle**: Upon receiving an `ACK` response token from the rocket (confirming the rocket executed a command like `RELEASE`), the console logs the ACK ensuring 100% operational clarity to the operator.
