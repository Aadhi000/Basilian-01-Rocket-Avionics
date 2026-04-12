# Web Dashboard Interface Gallery

The Basilian-01 Mission Control Dashboard (Telemetry PRO) is a Next.js and Flask-based dark-themed UI that visualizes high-frequency telemetry in real time. Below is a complete gallery of the system interface capturing different telemetry modules, analytics modes, and control consoles.

## 1. Dashboard Overview
![Dashboard Overview](../images/website/Screenshot%202026-04-07%20023358.png)
*The main view aggregating Flight State, IMU, GPS mapping, Control Console, and the Attitude Indicator.*

## 2. Primary Telemetry
![Primary Telemetry](../images/website/Screenshot%202026-04-07%20023409.png)
*Real-time feed showing the active Flight State, Barometric Altitude, and calculated Velocity.*

## 3. Full Systems Telemetry
![Full Systems Telemetry](../images/website/Screenshot%202026-04-07%20023422.png)
*Live graphical plotting of Accelerometer history, Gyroscope axes, Positioning, and Environment (Temp/Pressure/Humidity).*

## 4. System Health
![System Health](../images/website/Screenshot%202026-04-07%20023436.png)
*Monitoring of internal Flight Controller Battery levels and LoRa signal strength (RSSI).*

## 5. Control & Feed
![Control feed](../images/website/Screenshot%202026-04-07%20023450.png)
*Execution commands (Ping, Hold, Release, Send) alongside the raw, parsed telemetry CSV feed directly from the LoRa stream.*

## 6. Map Tracking
![Local Map](../images/website/Screenshot%202026-04-07%20023534.png)
*Live trajectory and active tracking of the flight vehicle using Leaflet.*

## 7. Attitude Indicator
![Attitude Indicator](../images/website/Screenshot%202026-04-07%20023544.png)
*2D Pitch, Roll, and Yaw visualization using MPU6050 quaternion/euler angle mapping.*

## 8. Analytics Overview
![Analytics Top](../images/website/Screenshot%202026-04-07%20023603.png)
*The dedicated analytics tab showcasing detailed, interactive line charts for Apogee detection, Velocity, Temperature, and Pressure.*

## 9. Recent Packet Logs
![Analytics Bottom](../images/website/Screenshot%202026-04-07%20023621.png)
*A consolidated tabular view of the most recent telemetry packets processed by the backend.*

## 10. Launch Sequence Console
![Launch Sequence](../images/website/Screenshot%202026-04-07%20024838.png)
*The secure modal overlay used to send the authenticated **Initiate Launch** or **Manual Eject** commands to the SCUB ignition module.*
