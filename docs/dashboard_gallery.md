# Web Dashboard Interface Gallery

The Basilian-01 Mission Control Dashboard (Telemetry PRO) is a Next.js and Flask-based dark-themed UI that visualizes high-frequency telemetry in real time. Below is a complete gallery of the system interface capturing different telemetry modules, analytics modes, and control consoles.

## 1. Dashboard Overview
![Dashboard Overview](../images/website/Screenshot%202026-04-07%20023358.png)
*The **Dashboard Overview** serves as the unified command center for the Ground Control Officer. It aggregates all critical mission parameters onto a single pane of glass—combining flight state, inertials, GPS mapping, command consoles, and attitude visualization. This ensures situational awareness is maintained instantly without navigating through multiple tabs during a fast-paced launch sequence.*

## 2. Primary Telemetry
![Primary Telemetry](../images/website/Screenshot%202026-04-07%20023409.png)
*The **Primary Telemetry** module isolates the most critical flight data required to confirm a successful launch. It displays the active Finite State Machine (FSM) status (e.g., Armed, Powered Ascent, Apogee, Descent), the real-time Barometric Altitude, and the calculated descent or ascent Velocity. This gives immediate visual confirmation of the rocket's current trajectory phase.*

## 3. Full Systems Telemetry
![Full Systems Telemetry](../images/website/Screenshot%202026-04-07%20023422.png)
*The **Full Systems Telemetry** view provides deep diagnostic insight into the hardware's performance. It features live, dynamically scaling line charts plotting the raw Accelerometer and Gyroscope axes to monitor vibration and stability. It also tracks historical environmental conditions (Temperature, Pressure, Humidity) and active satellite positioning coordinates in real-time.*

## 4. System Health
![System Health](../images/website/Screenshot%202026-04-07%20023436.png)
*The **System Health** module is vital for preventing mid-flight logic failures. It actively monitors the internal battery percentage of the onboard ESP32 flight controller to ensure sufficient voltage remains to fire the parachute servos. Additionally, it charts the LoRa RSSI (Received Signal Strength Indicator), allowing operators to predict line-of-sight dropouts or telemetry blackouts.*

## 5. Control & Feed
![Control feed](../images/website/Screenshot%202026-04-07%20023450.png)
*The **Control & Feed** panel bridges remote manual operations and raw data debugging. It provides hardware command buttons (Ping, Hold, Release, Send) that broadcast over the 433MHz frequency to the physical rocket. Below the controls, a scrolling terminal displays the raw, unprocessed CSV packets exactly as they are received by the LoRa ground station, aiding in real-time troubleshooting.*

## 6. Map Tracking
![Local Map](../images/website/Screenshot%202026-04-07%20023534.png)
*The **Map Tracking** interface integrates Leaflet mapping layers to transpose the NEO-6M GPS coordinates directly onto 2D topography. This provides a live trajectory of the flight vehicle and establishes a dynamic recovery target radius, enabling ground teams to visually track and physically recover the payload after a parachute descent.*

## 7. Attitude Indicator
![Attitude Indicator](../images/website/Screenshot%202026-04-07%20023544.png)
*The **Attitude Indicator** abstracts complex quaternion and Euler angle math from the MPU6050 into an easy-to-read aircraft-style digital horizon. It instantly visually communicates whether the rocket has pitched over, induced a sudden roll, or deviated off its parallel flight path without requiring operators to interpret raw numbers.*

## 8. Analytics Overview
![Analytics Top](../images/website/Screenshot%202026-04-07%20023603.png)
*The **Analytics Overview** tab is dedicated to post-flight and in-flight technical review. It showcases interactive line charts tracking Altitude, Velocity, Temperature, and Pressure against the mission clock. It clearly visualizes the peak of the arc, proving mathematically exactly when and where Apogee was detected and the parachutes were deployed.*

## 9. Recent Packet Logs
![Analytics Bottom](../images/website/Screenshot%202026-04-07%20023621.png)
*The **Recent Packet Logs** section provides a structured, tabular audit trail of the PostgreSQL database. It consolidates the high-frequency telemetry into human-readable rows, freezing exact timestamps and parameter states for meticulous post-mission analysis and safety auditing.*

## 10. Launch Sequence Console
![Launch Sequence](../images/website/Screenshot%202026-04-07%20024838.png)
*The **Launch Sequence Console** is a high-stakes, secure modal overlay. It specifically prevents accidental ground firings by explicitly requiring the operator to authorize and transmit the "Initiate Launch" command. Once authorized, this digitally triggers the distant SCUB Ignition Module's relay, latching the 7.4V battery array directly to the ignitor for liftoff.*
