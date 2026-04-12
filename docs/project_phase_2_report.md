# Basilian-01: Development of a Fault-Tolerant Rocket Avionics Hardware System
**Project Phase II Report**

Submitted to the APJ Abdul Kalam Technological University in partial fulfillment of requirements for the award of degree Bachelor of Technology in Electronics and Communication Engineering.

**By:**
- AADIL MUHAMMED (MBC22EC001)
- BEVAL PHILIP MATHEW (MBC22EC018)
- RAHUL B (MBC22EC045)
- SIBI B JOHN (MBC22EC054)

**Institution:**  
Mar Baselios Christian College of Engineering and Technology  
Department of Electronics and Communication Engineering  
*2025-2026*

---

## Abstract
The hardware design and integration of Basilian-01 present a model rocket avionics system developed for reliable flight monitoring, effective recovery control, and atmospheric aerosol data acquisition. The system is centered on a dual-redundant embedded hardware architecture to enhance mission reliability and eliminate single-point failures during critical flight phases.

Two independent ESP32-based flight control units are employed, each interfaced with dedicated barometric sensors for altitude estimation, ensuring fault tolerance through redundant sensing and control paths. The avionics hardware integrates multiple subsystems including barometric altitude sensors, an IMU, GPS module, LoRa-based telemetry transceiver, SD card data logger, servo-based parachute deployment mechanism, and an aerosol sensing module for in-flight atmospheric particulate measurement. 

All recovery-critical operations, such as apogee detection and parachute deployment, are designed to operate autonomously on-board, independent of ground systems. Emphasis is placed on robust power distribution, signal interfacing, sensor integration, and redundancy-oriented hardware design. 

---

## Chapter 1: Introduction
Sounding rockets and model rockets have become increasingly important platforms for low-cost aerospace experimentation, atmospheric studies, and avionics system validation. Unlike large-scale launch vehicles, model rockets provide an accessible testbed for evaluating embedded hardware performance, sensor integration, telemetry systems, and recovery mechanisms under dynamic flight conditions. 

The Basilian-01 project focuses on the hardware design and integration of a dual-redundant rocket avionics system with aerosol sensing capability. The system employs redundant embedded controllers, multiple sensors for altitude and motion estimation, telemetry modules for real-time data transmission, onboard data logging, and a servo-based parachute deployment mechanism for safe recovery.

---

## Chapter 2: Literature Survey
1. **Servo-Based Model Rocket and Flight Behavior:** Outlines how IMU data can be used to identify critical flight events (launch detection, peak acceleration, apogee). 
2. **Space-Capable Sounding Rockets:** Advocates for a modular avionics design philosophy separating critical subsystems and allowing independent testing.
3. **Fault-Tolerant Embedded Systems:** Identifies single-point failure as a critical weakness in conventional rocketry. Proposes dual-redundant setups.
4. **Atmospheric Data Acquisition:** Validates the feasibility of low-cost rocket-based aerosol monitoring systems and highlights potential for atmospheric research.
5. **High-Reliability Telemetry:** Advocates hybrid data handling combining real-time telemetry with onboard SD data logging as a resilient backup.

---

## Chapter 3: System Specifications and Design Justification

### 3.1 & 3.2 Flight Controller (ESP32)
The ESP32 microcontroller is selected due to its high processing capability (dual-core 240 MHz), integrated communication features, and extensive peripheral support. The dual-core architecture enables task separation, where one core manages sensor fusion while the other handles communication and storage operations.

### 3.3 Communication System (LoRa 433 MHz)
Long-range telemetry communication relies on LoRa technology operating at 433 MHz. LoRa provides long-distance communication extending several kilometers under line-of-sight conditions.

### 3.4 & 3.5 Aerosol Detection Payload & Machine Intelligence
The payload captures vertical atmospheric data via optical scattering principles. A Machine Intelligence (MI) algorithm is proposed for onboard data processing to extract meaningful features (mean particulate concentration, rate of change) and stabilize raw noise caused by vibration.

### 3.6 to 3.8 Sensor and Data Logging Modules
- **MPU6050 (IMU):** Tracks motion dynamics, acceleration, and angular velocity.
- **BME280:** High-accuracy measurements of temperature, pressure, and humidity for barometric altitude.
- **SD Card Module:** SPI-based storage for preserving critical measurements locally in CSV/binary formats.

### 3.9 & 3.10 Power Distribution & Recovery
- **UBEC Power:** Steps down Li-Po battery voltage (7.4V) to stable 5V and 3.3V lines for safe logic operation, isolating high-current servo spikes.
- **Servo-based Recovery:** Uses mechanical deployment over pyrotechnics for safety and reusability. Triggered algorithmically at apogee detection.

### 3.11 & 3.12 Imaging & GPS Navigation
- **Mini DVR & Camera:** Real-time video capturing powered by the regulated UBEC to avoid voltage fluctuations.
- **NEO-6M GPS:** Provides NMEA sentences processed over UART for real-time tracking.

---

## Chapter 4: Block Diagram Architecture

### 4.1 On-Board System
Powered by a 2S/3S Li-Po battery via two UBECs. Employs two ESP32 microcontrollers (ESP32-A and ESP32-B) functioning as active-active redundant primary flight controller units. 

### 4.2 Ground Control System
Powered by Li-Po and UBEC. An ESP32 interfaces with an SX1278 LoRa module listening at 433 MHz to receive telemetry. A passive buzzer provides audible feedback during RX/TX events.

### 4.3 Wireless Ignition
Uses an ESP32 processing an authenticated launch command via LoRa to trigger a relay module completing the circuit from a battery array directly into the ignitor. 

---

## Chapter 5: Core Software Architecture
Follows a modular and layered architecture prioritizing real-time task scheduling. High-priority tasks (sensor acquisition) run at faster loops, while lower-priority tasks (data logging) schedule less frequently. 

---

## Chapter 6: Circuit Diagrams
- **Flight Unit:** Features the ESP32 array, BME280, MPU6050, LoRa SX1278, and SDS011 payload.
- **Ground Plane:** Minimal control interface bridging the LoRa link to PC/Serial, equipped with buzzer indicators.
- **Ignition:** Standalone secure trigger relay isolating heavy load currents from logic rails.

*(Refer to `docs/hardware_design.md` for attached schematic diagrams).*

---

## Chapter 7: Flight State Machine Design
The FSM ensures deterministic execution by separating the flight into defined phases:
1. **Calibration State:** Sensor baseline initialization.
2. **Armed State:** Ready for launch. Wait state.
3. **Launch Detection:** Liftoff verified via sustained vertical acceleration.
4. **Powered Ascent:** Motor active, rapid altitude gain.
5. **Coasting Phase:** Motor burnout, inertial climb.
6. **Apogee State:** Velocity hits zero, altitude begins decreasing. 
7. **Descent Phase:** Parachutes deployed.
8. **Landing & Abort States:** Terminal resting conditions.

---

## Chapter 8 & 9: Apogee Detection & Redundant Estimation
Altitude is computed from barometric pressure via exponential atmospheric models. A moving average filter removes rapid pressure spikes. 

**Redundancy Implementation:**
Both ESP32-A and ESP32-B independently compute altitude using individual BME280s. A sliding window confirms a continuous decreasing altitude trend across multiple cycles to trigger apogee. A servo deploys the parachute. Backup deployment strategies (time-based fallback and secondary thresholds) exist to trigger if primary detection anomalies occur.

---

## Chapter 10: Derated Analysis

### Component Tolerances Analysis

| Component | V_op | V_max | Margin | Current (Typ) | Current limit |
|:---|:---:|:---:|:---:|:---:|:---:|
| BME280 | 3.3V | 3.6V  | 8.3% | 0.0036 mA | - |
| MPU6050 | 3.3V | 3.46V | 4.6% | 3.8 mA | - |
| LoRa (SX1278) | 3.3V | 3.6V  | 8.3% | 100 mA (TX) | - |
| NEO-6M GPS | 3.3V | 3.6V  | 8.3% | 50 mA | - |
| Servo | 5.0V | 6.0V  | 16.6%| 100 mA (Stall 1A)| - |

**Conclusions:** Treat 3.3V as ~90–95% of absolute max. Regulator handles 2A peak but is strictly sized to an 80% continuous rating margin (~1.6A). Based on applied derating considerations, all components operate with sufficient safety margins.

---

## Chapter 11: Telemetry and Data Logging
CSV-formatted telemetry packets broadcast at regular intervals (50–100ms).
**Format:** `STATE,temp,hum,press,altCm,battPct,pitch,roll,yaw,ax,ay,az,gx,gy,gz,lat,lon,gpsFix,sats,hdop,speed_kmph,timestamp,RSSI`

A GSM-based emergency messaging fallback ensures disaster thresholds immediately trigger SMS alerts to pre-registered authorities if cloud/internet access breaks.

---

## Chapter 12 & 14: Ground Station & Website Backend 
The telemetry is funneled into a Python Flask backend bridging physical serial to WebSockets.

- **Frontend:** Next.js-based dark-themed UI implementing real-time data visualization via line charts, 3D attitude visuals, and command consoles. Employs strong role-based authentication.
- **Backend:** Flask and Flask-SocketIO handle low-latency broadcasting. PostgreSQL stores historical flights.

*(Refer to `docs/dashboard_frontend.md` and `docs/dashboard_backend.md` for deep software implementations).*

---

## Chapter 13: Software Safety & Redundancy

| Mechanism | Description |
|:---|:---|
| Dual Microcontrollers | Active-active redundant ESP32 handling |
| Watchdog Timers | Reboots the system if loops freeze |
| Sensor Validation | Pre-flight constraints on reading boundaries |
| Dual Servos | Primary and Backup parachute releases |

---

## Chapter 15 & 18: Results & Conclusion

### Circuit Board Hardware
![Wireless Ignition Hardware](../images/ignition_photo.jpg)
*(Fig 15.1: Assembled Wireless Ignition Circuit with ESP32, SX1278 LoRa, and Relay Module)*

The Basilian-01 rocket avionics system successfully demonstrated an effective hardware architecture capable of monitoring flight parameters, maintaining long-range communication, logging data on-board, and executing robust recovery via a multi-trigger FSM without external intervention. Cost-effective, modular, and fault-tolerant, this architecture serves as a superior test bench for experimental payload integration and reliable model rocketry research.

---

## Achievements
The team successfully validated these parameters over multiple flight trials, earning media coverage and certifications from regional Engineering & Scientific authorities for robust technological advancements.

*(End of Report)*
