# 🚀 Ground Control - Rocket Telemetry System (Python Backend)

**A complete Python-based real-time telemetry system for high-altitude rocket tracking**

![Version](https://img.shields.io/badge/version-3.0-blue)
![Status](https://img.shields.io/badge/status-production--ready-green)
![Python](https://img.shields.io/badge/python-3.8%2B-brightgreen)
![License](https://img.shields.io/badge/license-MIT-blue)

---

## 📋 Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Key Features](#key-features)
- [Hardware Components](#hardware-components)
- [Software Stack](#software-stack)
- [Database Schema](#database-schema)
- [Telemetry Specifications](#telemetry-specifications)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Component Details](#component-details)
- [API Reference](#api-reference)
- [Configuration](#configuration)
- [Flight State Machine](#flight-state-machine)
- [Dashboard Features](#dashboard-features)
- [Performance Specifications](#performance-specifications)
- [Deployment](#deployment)
- [Troubleshooting](#troubleshooting)
- [Development](#development)
- [FAQ](#faq)

---

## Overview

Ground Control is a **full-stack Python telemetry system** designed for high-altitude rocket missions. The system provides real-time data acquisition, processing, storage, and visualization for rocket telemetry data transmitted via LoRa radio.

### What It Does

- **Captures** sensor readings from rocket-mounted sensors (IMU, GPS, barometer, air quality)
- **Transmits** data via long-range LoRa SX1278 radio (10+ km range)
- **Processes** telemetry through Python serial bridge and Flask backend
- **Stores** all data in NeonDB (serverless PostgreSQL) with 30-day retention
- **Displays** live telemetry on interactive web dashboard with charts and GPS tracking
- **Controls** rocket operations via admin command interface (Launch, Eject, Ping, )

### Key Features

✅ **Real-time Telemetry**

- sensor fields (IMU, GPS, barometer, air quality, servos, power)
- Live WebSocket streaming to dashboard
- Configurable packet rate (10- Hz)

✅ **Advanced Data Visualization**

- Altitude profiles with GPS overlay
- 3D orientation (roll/pitch/yaw)
- Live GPS tracking on interactive map
- Environmental monitoring (temp, humidity, AQI)
- Link quality indicators (RSSI/SNR)

✅ **Persistent Storage**

- NeonDB (PostgreSQL) backend
- 30-day retention with auto-archival
- CSV backup for safety
- Full event logging

✅ **Mission Control**

- Flight state machine (IDLE → RDY → FLT → APOGEE → DESCENT → LANDED)
- Admin command interface (Launch, Eject, Ping, )
- Real-time event log with source tracking

✅ **Robust Architecture**

- Binary CBOR serialization (≤70 bytes per packet)
- Store-and-forward for dropouts
- CRC validation on all packets
- Automatic reconnection

---

## Hardware Components

### Rocket-Mounted Sensors

| Module              | Model               | Purpose                            | Interface | Specifications            |
| ------------------- | ------------------- | ---------------------------------- | --------- | ------------------------- |
| **Microcontroller** | ESP32 WROOM-32      | Central processor & data packaging | -         | Dual-core 240MHz, WiFi/BT |
| **Barometer**       | BME280              | Pressure, temperature, humidity    | I2C       | ±1 hPa, ±1°C, ±3% RH      |
| **IMU**             | MPU-6050            | 6-axis accel + gyro                | I2C       | ±2g/±250°/s, 16-bit       |
| **GPS**             | NEO-6M              | Position, velocity, time           | UART      | 2.5m CEP, 5 Hz update     |
| **Air Quality**     | SDS011              | PM2.5 & PM10 particles             | UART      | 0-999 µg/m³               |
| **Radio**           | LoRa SX1278 (RA-02) | Long-range telemetry               | SPI       | 915 MHz, 10+ km range     |
| **Servos**          | Standard 9g         | Fin control & parachute            | PWM       | 0-180°                    |
| **Power**           | LiPo Battery        | System power                       | -         | 3.7-4.2V monitoring       |

### Ground Station

| Component       | Purpose        | Specifications             |
| --------------- | -------------- | -------------------------- |
| **ESP32**       | LoRa receiver  | Receives telemetry packets |
| **LoRa SX1278** | Radio receiver | 915 MHz, matched to rocket |
| **USB-Serial**  | PC interface   | 115200 baud UART           |

---

## Software Stack

### Backend Technologies

| Technology          | Version | Purpose                   |
| ------------------- | ------- | ------------------------- |
| **Python**          | 3.8+    | Core language             |
| **Flask**           | 2.3.3   | Web framework             |
| **Flask-SocketIO**  | 5.3.4   | WebSocket server          |
| **SQLAlchemy**      | 2.0.44  | ORM for database          |
| **psycopg2-binary** | 2.9.11  | PostgreSQL driver         |
| **pyserial**        | 3.5     | Serial communication      |
| **cbor2**           | 5.4.6   | Binary serialization      |
| **python-dotenv**   | 1.0.0   | Environment configuration |

### Frontend Technologies

| Technology             | Purpose                 |
| ---------------------- | ----------------------- |
| **Bootstrap 5**        | Responsive UI framework |
| **Chart.js**           | Real-time charting      |
| **Leaflet.js**         | GPS map visualization   |
| **Socket.IO Client**   | WebSocket connection    |
| **Font Awesome**       | Icons                   |
| **Vanilla JavaScript** | Dashboard logic         |

### Database

- **NeonDB** (Serverless PostgreSQL 15+)
- **Alternative**: Local PostgreSQL 12+

---

## Database Schema

*(Truncated details here for brevity, see code structure)*

---

## Telemetry Specifications

This section details the structure and content of the telemetry packets transmitted from the rocket and processed by the ground station.

### Packet Structure

Telemetry data is serialized using **CBOR (Concise Binary Object Representation)** for efficiency over the LoRa radio link. Each packet is a dictionary (map) containing various sensor readings and system states.

**Maximum Packet Size**: ~70 bytes (excluding LoRa headers)
**Transmission Rate**: Configurable, typically 10-50 Hz

### Ground Station Processing

Upon reception, the `serial_bridge` validates the packet (CRC check) and forwards it to the Flask backend via WebSocket. The backend then:
1. **Deserializes** the CBOR data.
2. **Enriches** the data with server-side timestamps (`timestamp_utc`).
3. **Translates** enum codes (e.g., `phase`, `rst_rsn`) into human-readable strings.
4. **Calculates** derived metrics (e.g., AQI from PM2.5/PM10).
5. **Stores** the comprehensive record in the `telemetry_full` table in NeonDB.
6. **Broadcasts** the processed data to connected dashboard clients via WebSocket.

---

## Quick Start

### 1. Setup (5 minutes)

```bash
# Clone repo
cd basilian-01

# Create virtual environment
python3 -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Configure
cp config/.env.example config/.env
```

### 3. Database
**Option B: NeonDB (Cloud)**
1. Sign up: https://neon.tech
2. Set DATABASE_URL in config/.env

### 4. Run It

**Terminal 1: Backend**
```bash
./run_backend.sh
```

**Terminal 2: Serial Bridge** (when receiver connected)
```bash
./run_serial_bridge.sh
```

---

## System Architecture

### High-Level Overview

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          ROCKET (ESP32)                                  │
│                         LoRa SX1278 TX                                   │
└──────────────────────────────┬──────────────────────────────────────────┘
                              │ LoRa Radio (915 MHz, 10+ km range)
                              ↓
┌──────────────────────────────────────────────────────────────────────────┐
│                      GROUND STATION (ESP32)                              │
│                         UART Serial TX                                   │
└──────────────────────────────┬──────────────────────────────────────────┘
                              │ Serial (/dev/ttyUSB0, 115200 baud)
                              ↓
┌──────────────────────────────────────────────────────────────────────────┐
│                    PYTHON SERIAL BRIDGE                                  │
└──────────────────────────────┬──────────────────────────────────────────┘
                              │ WebSocket Connection
                              ↓
┌──────────────────────────────────────────────────────────────────────────┐
│                       FLASK BACKEND SERVER                               │
└──────────────────────────────┬──────────────────────────────────────────┘
                              │ WebSocket + REST API
                              ↓
┌──────────────────────────────────────────────────────────────────────────┐
│                      WEB DASHBOARD (Browser)                             │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## API Reference

### REST Endpoints
*(See full README code block for complete HTML APIs and standard queries)*

### WebSocket Events
Available Events: `connect`, `telemetry`, `flight_phase`, `link_status`, `event`

---

## Admin Commands

| Command    | Parameters | Effect            | Safety        |
| ---------- | ---------- | ----------------- | ------------- |
| **LAUNCH** | {}         | Transition to RDY | Requires IDLE |
| **EJECT**  | {}         | Deploy chute      | Requires FLT+ |
| **PING**   | {}         | Test link         | Anytime       |
| **ARM**    | {}         | Arm for launch    | Requires IDLE |
| \*\*\*\*   | {}         | Emergency abort   | Anytime       |

---

## Flight State Machine

```
IDLE → RDY → FLT → APOGEE → DESCENT → LANDED
```

---

**End of Backend README**
