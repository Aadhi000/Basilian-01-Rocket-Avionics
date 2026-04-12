# Ground Control - Rocket Telemetry & Mission Control System (Frontend)

A real-time rocket telemetry and mission control system built with Next.js, PostgreSQL, WebSockets, and ESP32-based hardware.

## 📋 Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Hardware Components](#hardware-components)
- [Getting Started](#getting-started)
- [Configuration](#configuration)
- [Deployment](#deployment)
- [API Reference](#api-reference)
- [Firmware](#firmware)

---

## 🛰️ Overview

Ground Control is a comprehensive mission control system that receives live sensor data from a rocket via **LoRa SX1278 433MHz radio link** between:

| Device | Role |
|--------|------|
| ESP32 + Sensors (Rocket) | Telemetry Sender |
| ESP32 + LoRa (Ground Station) | Receiver → WebSocket Bridge |
| Next.js Web App | Dashboard & Command Center |
| PostgreSQL | Historical Data Storage |

### Features

- ✅ **Real-time telemetry** at ~100ms update rate
- ✅ **Dark-themed dashboard** with live charts and map
- ✅ **Admin controls** for Launch, Emergency Eject, Ping
- ✅ **Secure authentication** with role-based access
- ✅ **Historical data** logging and CSV export
- ✅ **DigitalOcean deployment** ready

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         ROCKET                                   │
│  ┌─────────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐        │
│  │  MPU-6050   │  │ Neo-6M  │  │ BME280  │  │  Servo  │        │
│  │   (IMU)     │  │  (GPS)  │  │ (Baro)  │  │(Chute)  │        │
│  └──────┬──────┘  └────┬────┘  └────┬────┘  └────┬────┘        │
│         └───────────────┴───────────┴────────────┘              │
│                         │                                        │
│                  ┌──────┴──────┐                                │
│                  │ ESP32 WROOM │                                │
│                  │   + LoRa    │                                │
│                  └──────┬──────┘                                │
└─────────────────────────┼───────────────────────────────────────┘
                          │ 433MHz
                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                    GROUND STATION                                │
│                  ┌──────┴──────┐                                │
│                  │ ESP32 + LoRa│──────► Serial USB              │
│                  │  (Receiver) │                                │
│                  └─────────────┘                                │
└─────────────────────────┼───────────────────────────────────────┘
                          │
                  ┌───────▼───────┐
                  │ Python Bridge │
                  │ serial_bridge │
                  └───────┬───────┘
                          │ HTTP/WebSocket
                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                      CLOUD/SERVER                                │
│  ┌─────────────────┐    ┌───────────────┐    ┌──────────────┐  │
│  │    Next.js      │◄──►│   PostgreSQL  │    │  WebSocket   │  │
│  │   Dashboard     │    │    Database   │    │   Clients    │  │
│  └─────────────────┘    └───────────────┘    └──────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔧 Hardware Components

Based on the **Flight Controller Schematic**:

| Component | Function | Interface |
|-----------|----------|-----------|
| ESP32 WROOM-32 | Main MCU | - |
| SX1278 LoRa RA-02 | 433MHz Radio | SPI |
| MPU-6050 | IMU (Accel + Gyro) | I2C |
| Neo-6M GPS | Position & Altitude | UART |
| BME280 | Barometer, Temp, Humidity | I2C |
| Servo Motor | Parachute Deployment | PWM |
| UBEC Regulator | 5V/3.3V Power | - |
| 7.4V LiPo | Flight Power | - |

### Pin Assignments

```
LoRa SX1278:  NSS=5, RST=14, DIO0=2, SCK=18, MISO=19, MOSI=23
I2C (Sensors): SDA=21, SCL=22
GPS (Serial2): RX=16, TX=17
Servo:        GPIO 13
Battery ADC:  GPIO 34
```

---

##  Getting Started

### Prerequisites

- Node.js 18+
- PostgreSQL 14+
- Python 3.8+ (for serial bridge)
- PlatformIO (for firmware)

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/arroward/basilian-01.git
   cd basilian-01/ground-control
   ```

2. **Install dependencies**
   ```bash
   npm install
   ```

3. **Configure environment**
   ```bash
   cp .env.example .env.local
   # Edit .env.local with your settings
   ```

4. **Run database migrations**
   ```bash
   npm run db:migrate
   npm run db:seed  # Optional: add sample data
   ```

5. **Start development server**
   ```bash
   npm run dev
   ```

6. **Open dashboard**
   Visit [http://localhost:3000](http://localhost:3000)

### Default Credentials

- **Admin:** `admin` / `admin123`
- **Operator:** `operator` / `operator123`
- **Viewer:** `viewer` / `viewer123`

---

## ⚙️ Configuration

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `DATABASE_URL` | PostgreSQL connection string | Required |
| `NEXTAUTH_URL` | App URL | http://localhost:3000 |
| `NEXTAUTH_SECRET` | Auth secret (generate with `openssl rand -base64 32`) | Required |
| `WEBSOCKET_PORT` | WebSocket server port | 3001 |

Additional environment variables for built-in credential auth:

| Variable | Description | Default |
|----------|-------------|---------|
| `ADMIN_PASSWORD` | Password for `admin` role | `admin123` |
| `OPERATOR_PASSWORD` | Password for `operator` role | `operator123` |
| `VIEWER_PASSWORD` | Password for `viewer` role | `viewer123` |
| `GC_USERS_JSON` | JSON array of `{ username, password, role, displayName }` entries to override defaults | _Optional_ |

### Serial Bridge Configuration

```bash
cd bridge
pip install -r requirements.txt
python serial_bridge.py --port /dev/ttyUSB0 --baud 115200 --server ws://localhost:3000
```

---

## ☁️ Deployment

### DigitalOcean App Platform

1. **Create a DigitalOcean account** and install `doctl`

2. **Create managed PostgreSQL database**
   ```bash
   doctl databases create ground-control-db --engine pg --size db-s-1vcpu-1gb --region nyc1
   ```

3. **Deploy the app**
   ```bash
   doctl apps create --spec .do/app.yaml
   ```

4. **Set secrets**
   ```bash
   doctl apps update <app-id> --spec .do/app.yaml
   ```

### GitHub Actions

The included workflow (`.github/workflows/deploy.yml`) automatically:
1. Runs linting and type checks on PR
2. Builds the application
3. Deploys to DigitalOcean on merge to main

Required secrets:
- `DIGITALOCEAN_ACCESS_TOKEN`
- `DIGITALOCEAN_APP_ID`

---

## 📡 API Reference

### Telemetry

```http
POST /api/telemetry
Content-Type: application/json

{
  "ts": 1733222300,
  "lat": 34.05223,
  "lon": -118.24374,
  "alt": 150.5,
  "vz": 25.3,
  "r": 5.2, "p": -2.1, "y": 180.0,
  "ax": 0.02, "ay": -0.01, "az": 9.80,
  "bv": 7.85,
  "state": "FLT",
  "rssi": -62
}
```

```http
GET /api/telemetry?limit=100&chart=true&minutes=10
```

### Commands

```http
POST /api/commands
Authorization: Bearer <token>
Content-Type: application/json

{
  "command": "LAUNCH",
  "payload": {}
}
```

Available commands: `ARM`, `DISARM`, `LAUNCH`, `EJECT`, `ABORT`, `PING`, `CALIBRATE`

### Events

```http
GET /api/events?limit=100&type=COMMAND&severity=INFO
```

### Export

```http
GET /api/export/csv?startTime=2024-01-01T00:00:00Z&endTime=2024-01-02T00:00:00Z
```

---

## 🔌 Firmware

### Rocket Flight Controller

Location: `firmware/rocket/rocket_flight_controller.ino`

Features:
- 10Hz telemetry transmission
- Automatic launch detection (accelerometer)
- Automatic apogee detection & parachute deployment
- Command reception and acknowledgment
- Mission state machine (CAL → RDY → FLT → LND)

### Ground Station Receiver

Location: `firmware/ground_station/ground_station_rx.ino`

Features:
- Continuous LoRa receive
- Serial JSON output for Python bridge
- Command relay to rocket
- Signal strength reporting

### Building Firmware

```bash
cd firmware
# For rocket
pio run -e rocket -t upload

# For ground station
pio run -e ground_station -t upload
```

---

## 📊 Mission States

| Code | State | Color | Description |
|------|-------|-------|-------------|
| CAL | Calibration | 🔵 Blue | Initial calibration |
| RDY | Ready/Armed | 🟡 Yellow | Armed for launch |
| FLT | In-Flight | 🟢 Green | Active flight |
| ABT | Abort | 🔴 Red | Abort/Emergency |
| LND | Landed | 🟣 Purple | Post-landing recovery |

---

## 📐 Telemetry Packet Format

Compact JSON for LoRa bandwidth efficiency:

```json
{
  "ts": 1733222300,     // Unix timestamp
  "lat": 34.05223,      // GPS latitude
  "lon": -118.24374,    // GPS longitude
  "alt": 150.5,         // Altitude (m)
  "vz": 25.3,           // Vertical velocity (m/s)
  "r": 5.2,             // Roll (°)
  "p": -2.1,            // Pitch (°)
  "y": 180.0,           // Yaw (°)
  "ax": 0.02,           // Accel X (m/s²)
  "ay": -0.01,          // Accel Y (m/s²)
  "az": 9.80,           // Accel Z (m/s²)
  "bv": 7.85,           // Battery (V)
  "state": "FLT",       // Mission state
  "rssi": -62,          // Signal strength
  "temp": 25.3,         // Temperature (°C)
  "hum": 45,            // Humidity (%)
  "sat": 8,             // GPS satellites
  "pd": false,          // Parachute deployed
  "pid": 12345          // Packet ID
}
```

---

## 🛡️ Security

- **Authentication:** NextAuth.js with JWT
- **Role-based access:**
  - `admin` - Full control
  - `operator` - Issue commands
  - `viewer` - View telemetry only
- **Command authentication:** Critical commands (EJECT, ABORT) require confirmation
- **HTTPS:** Automatic via DigitalOcean

---

## 🧪 Testing

### Local Testing with Mock Data

```bash
npm run db:seed
npm run dev
```

### Simulating Telemetry

```bash
# Send mock telemetry
curl -X POST http://localhost:3000/api/telemetry \
  -H "Content-Type: application/json" \
  -d '{"ts":1733222300,"lat":34.05223,"lon":-118.24374,"alt":100,"vz":10,"r":0,"p":0,"y":0,"ax":0,"ay":0,"az":9.8,"bv":7.8,"state":"FLT","rssi":-65}'
```

---

## 📝 License

MIT License - See [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgments

- Hardware design based on Flight Controller Schematic
- LoRa library by Sandeep Mistry
- Sensor libraries by Adafruit
- Icons and styling inspired by SpaceX mission control

---

**Built with  by Ground Control Team**
