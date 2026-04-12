# Software Architecture

This document describes the software structure, Finite State Machine (FSM), telemetry transmission protocols, and Web Dashboard implementations for Basilian-01.

## 1. Core Flight Software (Active-Active Redundancy)

The Basilian-01 avionics software utilizes cooperative multitasking on dual ESP32s. To ensure high reliability, both microcontrollers actively parse sensor inputs, compute values, and run logic loops. 

### Finite State Machine (FSM)
To handle the highly dynamic motion parameters of flight, operations are grouped into states:

1. **CAL (Calibration):** Sensors online, base altitude baseline taken. 
2. **ARM (Ready):** System prepared, awaiting specific acceleration limits to detect launch.
3. **FLT (Ascent):** Powered thrust and coasting ascent phase. Continuous logging and LoRa TX.
4. **APG (Apogee):** The highest point; triggered when the effective voted altitude continuously drops past `APOGEE_DROP_CM` over successive checks. Servo mechanisms actuate parachute deployment.
5. **LND (Landing):** Triggered when velocity vector reaches 0 m/s and altitude stabilizes close to ground level for 5+ seconds.
6. **ABT (Abort):** Error recovery mode, locking mechanisms and defaulting to fail-safe parameters.

### Apogee Detection Logic
1. **Redundant Cross-voting:** Both controllers share barometric readings (`Alt_A` vs `Alt_B`). The effective altitude is established as `max(Alt_A, Alt_B)` to prevent premature deployment if one sensor fails low.
2. **Sliding Window Filtering:** Smoothing pressure oscillations to distinguish apogee from local aerodynamic turbulence.
3. **Trigger:** `effectiveAltitude < previousAltitude - APOGEE_THRESHOLD` evaluated true over consecutive samples while `flightDuration > MIN_FLIGHT_TIME`.

## 2. Telemetry and LoRa Link Protocol

The ESP32 flight controller pushes continuous CSV-formatted status packets encoded with specific state bounds over LoRa. Error checking includes checksums / CRCs across transmissions.

* **Frequency:** 433 MHz
* **Bandwidth:** 125 kHz
* **Spreading Factor:** SF9
* **Coding Rate:** 4/5

### CSV Payload Order Format:
`STATE, temp, humidity, press, altCm, battPct, pitch, roll, yaw, ax, ay, az, gx, gy, gz, lat, lon, gpsFix, sats, hdop, speed_kmph, timestamp, RSSI`

Example:
`FLT,25.3,45.2,987.6,1250,87.4,12.5,-8.3,22.1,0.2,0.8,-9.4,0.1,-0.3,0.2,13.0541,80.2384,1,8,1.2,15.3,1704067523,-110`

### Ground Station Interface
The ground unit receiver simply listens, parses string bytes, emits audible buzzer tones upon valid packet checksums, and forwards the packet stream over Serial/WebSocket bridges.

## 3. Web Dashboard (Front-End & Back-End)

The Ground Control web application handles visualization, data logging persistence, and operational remote-commands.

* **Front End (Next.js):**
    * Dark-themed React Dashboard.
    * Real-time web-socket graphs drawing Altitudes, Velocity vectors, and 3D orientation gauges.
    * Map UI processing the `lat,lon` elements against live map tiles.
    * Multi-tier login levels (Admin / Operator / Viewer).
* **Back End (Flask & NeonDB PostgreSQL):**
    * HTTP REST API endpoints and Flask-SocketIO servers communicating out to Next.js clients.
    * A Serial bridge mapping incoming LoRa packets from the receiver COM port into the SQL data-schema.
    * Remote task pipelines returning REST-bound commands (`ARM`, `ABORT`, `RELEASE`) back over the serial bridge to be launched upward by the Ground Unit LoRa TX.
