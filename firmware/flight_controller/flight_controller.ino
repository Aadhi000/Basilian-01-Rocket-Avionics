// Combined final ON-BOARD ESP32 firmware
// LoRa + BME280 + MPU6050 + GPS (Serial2) + SD logging + Servos + Battery + PING/ACK
// Timestamp field appended as IST (India Standard Time, UTC+5:30)
// *** ADDED: ESP-NOW for redundant data link to ESP32-B Logger ***

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <ESP32Servo.h>
#include <TinyGPSPlus.h>
#include <SD.h>
#include <WiFi.h>      // Required for ESP-NOW
#include <esp_now.h>   // Required for ESP-NOW

// ---------- Hardware pin config ----------
static const uint8_t LORA_SS    = 5;     // LoRa CS
static const uint8_t LORA_RST   = 14;    // LoRa RST
static const uint8_t LORA_DIO0  = 2;     // LoRa DIO0
static const long    LORA_FREQ  = 433E6;

static const int SPI_SCK    = 18;
static const int SPI_MISO   = 19;
static const int SPI_MOSI   = 23;

static const uint8_t SD_CS = 13;        // SD card CS

static const uint8_t BUZZER_PIN = 27;
static const uint8_t SERVO1_PIN = 25;
static const uint8_t SERVO2_PIN = 26;

static const uint8_t I2C_SDA = 21;
static const uint8_t I2C_SCL = 22;
static const uint8_t BME_ADDR = 0x77;
static const uint8_t MPU6050_ADDR = 0x68;

static const uint8_t BATT_ADC_PIN = 34;

// GPS (Serial2)
static const int GPS_RX_PIN = 16; // connect GPS TX -> GPIO16
static const int GPS_TX_PIN = 17; // optional: connect GPS RX -> GPIO17
static const unsigned long GPS_BAUD = 9600;

// ---------- Globals / objects ----------
Adafruit_BME280 bme;
Servo servo1, servo2;
TinyGPSPlus gps;

bool bmeOK = false;
bool imuOK = false;
bool sdOK = false;

File sdFile;

// ----- IMU calibration & orientation -----
float ax_off = 0, ay_off = 0, az_off = 0;
float gx_off = 0, gy_off = 0, gz_off = 0;
double ax_sum = 0, ay_sum = 0, az_sum = 0;
double gx_sum = 0, gy_sum = 0, gz_sum = 0;
long imu_samples = 0;
bool imuCalibrated = false;
float pitch = 0, roll = 0, yaw = 0;
unsigned long lastOrientationMs = 0;

// ----- Battery divider/calibration -----
static const float ADC_REF_V = 3.3f;
static const int ADC_MAX = 4095;
static const float R1 = 220000.0f;
static const float R2 = 100000.0f;
static const float CAL = 1.055f;
static const float BATT_FULL_V = 8.51f;
static const float BATT_EMPTY_V = 6.60f;

// ----- Flight timing, calibration, state machine -----
unsigned long lastTelemetry = 0;
static const unsigned long telemetryInterval = 1000;

static const unsigned long calibDuration = 20000; // 20s
unsigned long calibStartTime = 0;
bool altitudeCalibrated = false;
double altSum = 0.0;
long altSamples = 0;
float altBaselineMeters = 0.0f;

enum RocketState { STATE_CAL, STATE_ARMED, STATE_FLIGHT, STATE_APOGEE, STATE_LAND, STATE_ABORT };
RocketState currentState = STATE_CAL;
long lastAltCm = 0;
bool haveLastAlt = false;
long maxAltCm = 0;

static const long LIFTOFF_THRESH_CM     = 500;
static const long APOGEE_DROP_CM        = 50;
static const long LAND_TOLERANCE_CM     = 500;
static const long ABORT_ALT_THRESH_CM = 12000;

// ----- ACK behavior -----
static const int ACK_REPEATS = 2;
static const int ACK_DELAY_MS = 180;
static bool appendAckToNextTelemetry = false;
static String pendingAckToken = "";


// ******************************************************
// ---------- ESP-NOW specific configuration ----------

// ---------- ESP32-B MAC (SD LOGGER) ----------
uint8_t ESP32_B_MAC[] = {0xC0, 0xCD, 0xD6, 0x85, 0x6E, 0xCC};  //C0:CD:D6:85:6E:CC

// Packet structure for ESP-NOW (must be compatible with receiver)
typedef struct {
    char csv[420];    // full telemetry CSV
} ESPNOW_Packet;

ESPNOW_Packet espNowPacket;


void initESPNOW() {
    // ESP-NOW requires WiFi in STA mode to operate.
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("[ESP-NOW] Init FAILED");
        return;
    }

    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, ESP32_B_MAC, 6);
    peer.channel = 0; // Default channel
    peer.encrypt = false; // No encryption

    // Add peer to the list
    if (esp_now_add_peer(&peer) != ESP_OK) {
        Serial.println("[ESP-NOW] Peer add FAILED");
        return;
    }

    Serial.println("[ESP-NOW] Ready -> ESP32-B");
}

// ******************************************************


// ---------- Helpers: buzzer, servos ----------
void beepTX(){ digitalWrite(BUZZER_PIN, HIGH); delay(50); digitalWrite(BUZZER_PIN, LOW); }
void beepRX(){ for(int i=0;i<2;i++){ digitalWrite(BUZZER_PIN,HIGH); delay(40); digitalWrite(BUZZER_PIN,LOW); delay(40);} }

void setServos(int a1,int a2){ a1 = constrain(a1,0,180); a2 = constrain(a2,0,180); servo1.write(a1); servo2.write(a2); }
bool applyServoAngles(const String &cmd){ int c=cmd.indexOf(','); if(c<0) return false; String s1=cmd.substring(0,c), s2=cmd.substring(c+1); s1.trim(); s2.trim(); if(s1.isEmpty()||s2.isEmpty()) return false; setServos(s1.toInt(), s2.toInt()); return true; }
bool handleServoMessage(const String &msgRaw){ String m=msgRaw; m.trim(); String up=m; up.toUpperCase(); if(up=="RELEASE"){ setServos(150,150); return true; } if(up=="HOLD"){ setServos(75,75); return true; } return applyServoAngles(m); }

// ---------- Battery ----------
float readBatteryVoltage(){ int raw = analogRead(BATT_ADC_PIN); float vAdc = raw * (ADC_REF_V / (float)ADC_MAX); float vBatt = vAdc * ((R1 + R2) / R2); vBatt *= CAL; return vBatt; }
int readBatteryPercent(){ float v = readBatteryVoltage(); v = constrain(v, BATT_EMPTY_V, BATT_FULL_V); float pct = (v - BATT_EMPTY_V) * 100.0f / (BATT_FULL_V - BATT_EMPTY_V); return (int)(constrain(pct,0.0f,100.0f)+0.5f); }

// ---------- Altitude calibration ----------
void updateAltitudeCalibration(float altMeters){
    if(altitudeCalibrated || !bmeOK) return;
    unsigned long now = millis();
    altSum += altMeters; altSamples++;
    if(now - calibStartTime >= calibDuration){
        if(altSamples>0) altBaselineMeters = (float)(altSum / (double)altSamples);
        else altBaselineMeters = altMeters;
        altitudeCalibrated = true;
        Serial.print("[CALIB] baseline m: "); Serial.println(altBaselineMeters,3);
    }
}

// ---------- IMU low-level ----------
void imuWriteByte(uint8_t reg,uint8_t data){ Wire.beginTransmission(MPU6050_ADDR); Wire.write(reg); Wire.write(data); Wire.endTransmission(); }
void imuReadBytes(uint8_t reg,uint8_t *buf,uint8_t len){ Wire.beginTransmission(MPU6050_ADDR); Wire.write(reg); Wire.endTransmission(false); Wire.requestFrom(MPU6050_ADDR,len); uint8_t i=0; while(Wire.available() && i<len) buf[i++]=Wire.read(); }

bool initIMU(){
    imuWriteByte(0x6B,0x00); // wake
    delay(50);
    imuWriteByte(0x1C,0x00); // accel ±2g
    imuWriteByte(0x1B,0x00); // gyro ±250 dps
    return true;
}

void readIMU(float &ax,float &ay,float &az,float &gx,float &gy,float &gz){
    uint8_t b[14]; imuReadBytes(0x3B,b,14);
    int16_t rawAx = (b[0]<<8)|b[1]; int16_t rawAy=(b[2]<<8)|b[3]; int16_t rawAz=(b[4]<<8)|b[5];
    int16_t rawGx = (b[8]<<8)|b[9]; int16_t rawGy=(b[10]<<8)|b[11]; int16_t rawGz=(b[12]<<8)|b[13];
    ax = rawAx / 16384.0f; ay = rawAy / 16384.0f; az = rawAz / 16384.0f;
    gx = rawGx / 131.0f; gy = rawGy / 131.0f; gz = rawGz / 131.0f;
}

// ---------- IMU calibration & orientation ----------
void updateIMUCalibration(float ax,float ay,float az,float gx,float gy,float gz){
    if(imuCalibrated) return;
    unsigned long now = millis();
    ax_sum += ax; ay_sum += ay; az_sum += (az - 1.0f);
    gx_sum += gx; gy_sum += gy; gz_sum += gz;
    imu_samples++;
    if(now - calibStartTime >= calibDuration){
        if(imu_samples>0){
            ax_off = ax_sum / imu_samples; ay_off = ay_sum / imu_samples; az_off = az_sum / imu_samples;
            gx_off = gx_sum / imu_samples; gy_off = gy_sum / imu_samples; gz_off = gz_sum / imu_samples;
        }
        imuCalibrated = true;
        lastOrientationMs = now;
        Serial.println("[IMU] calibrated");
    }
}

void updateOrientation(float ax,float ay,float az,float gx,float gy,float gz,float dt){
    const float alpha = 0.98f;
    float pitchAcc = atan2f(ay, az) * 57.2957795f;
    float rollAcc  = atan2f(-ax, sqrtf(ay*ay + az*az)) * 57.2957795f;
    pitch = alpha * (pitch + gy * dt) + (1 - alpha) * pitchAcc;
    roll  = alpha * (roll  + gx * dt) + (1 - alpha) * rollAcc;
    yaw  += gz * dt;
}

// ---------- Flight state ----------
void updateFlightState(long altCm){
    if(!haveLastAlt){ lastAltCm=altCm; maxAltCm=altCm; haveLastAlt=true; }
    if(altCm>maxAltCm) maxAltCm = altCm;
    switch(currentState){
        case STATE_CAL: if(altitudeCalibrated && imuCalibrated) { currentState=STATE_ARMED; Serial.println("[STATE] -> ARM"); } break;
        case STATE_ARMED: if(altCm >= LIFTOFF_THRESH_CM){ currentState=STATE_FLIGHT; Serial.println("[STATE] -> FLT"); } break;
        case STATE_FLIGHT:
            if(altCm >= ABORT_ALT_THRESH_CM){ currentState=STATE_ABORT; Serial.println("[STATE] -> ABT"); }
            else if(altCm < maxAltCm - APOGEE_DROP_CM){ currentState=STATE_APOGEE; Serial.println("[STATE] -> APG"); setServos(150,150); }
            break;
        case STATE_APOGEE: if(abs(altCm) <= LAND_TOLERANCE_CM){ currentState=STATE_LAND; Serial.println("[STATE] -> LND"); } break;
        case STATE_ABORT: if(abs(altCm) <= LAND_TOLERANCE_CM){ currentState=STATE_LAND; Serial.println("[STATE] ABT->LND"); } break;
        case STATE_LAND: break;
    }
    lastAltCm = altCm;
}

const char* stateTag(RocketState s){
    switch(s){ case STATE_CAL: return "CAL"; case STATE_ARMED: return "ARM"; case STATE_FLIGHT: return "FLT"; case STATE_APOGEE: return "APG"; case STATE_LAND: return "LND"; case STATE_ABORT: return "ABT"; default: return "UNK"; }
}

// ---------- Utility: produce IST timestamp from GPS (YYYY-MM-DD HH:MM:SS) ----------
String getISTTimestampFromGPS(){
    if(!gps.time.isValid() || !gps.date.isValid()) return String("N/A");

    // GPS time/date are UTC. Convert to IST (UTC+5:30)
    int hh = gps.time.hour();
    int mm = gps.time.minute();
    int ss = gps.time.second();
    int dd = gps.date.day();
    int mo = gps.date.month();
    int yy = gps.date.year();

    mm += 30;
    if(mm >= 60){ mm -= 60; hh += 1; }
    hh += 5;
    if(hh >= 24){
        hh -= 24;
        dd += 1;
        // NOTE: we do not implement full calendar rollover (month/year) here.
    }

    char buf[32];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", yy, mo, dd, hh, mm, ss);
    return String(buf);
}

// ---------- SD helper ----------
void sdAppendLine(const String &line){
    if(!sdOK) return;
    File f = SD.open("/flight_log.csv", FILE_APPEND);
    if(f){
        f.println(line);
        f.close();
    } else {
        sdOK = false;
        Serial.println("[SD] write failed, disabling SD writes");
    }
}

// ---------- Telemetry builder & sender (adds IST timestamp) ----------
void sendTelemetry(){
    if(!bmeOK) return;

    float temp = bme.readTemperature();
    float hum  = bme.readHumidity();
    float pres = bme.readPressure() / 100.0f;
    float altMeters = bme.readAltitude(1013.25f);

    if(!altitudeCalibrated) updateAltitudeCalibration(altMeters);

    long altCm = altitudeCalibrated ? (long)((altMeters - altBaselineMeters) * 100.0f) : 0;
    updateFlightState(altCm);
    int battPct = readBatteryPercent();

    float axv=0,ayv=0,azv=0,gxv=0,gyv=0,gzv=0;
    if(imuOK){
        readIMU(axv,ayv,azv,gxv,gyv,gzv);
        if(!imuCalibrated){
            updateIMUCalibration(axv,ayv,azv,gxv,gyv,gzv);
            axv=ayv=azv=gxv=gyv=gzv=0;
        } else {
            axv -= ax_off; ayv -= ay_off; azv -= az_off;
            gxv -= gx_off; gyv -= gy_off; gzv -= gz_off;
            unsigned long now = millis();
            float dt = (lastOrientationMs==0) ? 0.01f : ((now - lastOrientationMs) / 1000.0f);
            if(dt <= 0) dt = 0.01f;
            updateOrientation(axv,ayv,azv,gxv,gyv,gzv,dt);
            lastOrientationMs = now;
        }
    }

    // GPS fields
    bool gpsFix = gps.location.isValid();
    double gpsLat = gpsFix ? gps.location.lat() : 0.0;
    double gpsLon = gpsFix ? gps.location.lng() : 0.0;
    int gpsSats = gps.satellites.isValid() ? gps.satellites.value() : 0;
    float gpsHdop = gps.hdop.isValid() ? gps.hdop.hdop() : 0.0f;
    float gpsSpeedKmph = gps.speed.isValid() ? gps.speed.kmph() : 0.0f;

    // IST timestamp (from GPS); fallback = "N/A"
    String istTs = getISTTimestampFromGPS();

    // Build telemetry CSV payload (no spaces), fields (22 total):
    // 0:STATE, 1:temp, 2:hum, 3:press, 4:altCm, 5:battPct,
    // 6:pitch, 7:roll, 8:yaw,
    // 9:ax, 10:ay, 11:az, 12:gx, 13:gy, 14:gz,
    // 15:lat, 16:lon, 17:gpsFix, 18:sats, 19:hdop, 20:speed_kmph, 21:timestamp
    String pkt; pkt.reserve(400);
    pkt  = stateTag(currentState);
    pkt += ',' + String(temp,2);
    pkt += ',' + String(hum,2);
    pkt += ',' + String(pres,2);
    pkt += ',' + String(altCm);
    pkt += ',' + String(battPct);
    pkt += ',' + String(pitch,2);
    pkt += ',' + String(roll,2);
    pkt += ',' + String(yaw,2);
    pkt += ',' + String(axv,3);
    pkt += ',' + String(ayv,3);
    pkt += ',' + String(azv,3);
    pkt += ',' + String(gxv,3);
    pkt += ',' + String(gyv,3);
    pkt += ',' + String(gzv,3);
    pkt += ',' + String(gpsLat,7);
    pkt += ',' + String(gpsLon,7);
    pkt += ',' + String(gpsFix ? 1 : 0);
    pkt += ',' + String(gpsSats);
    pkt += ',' + String(gpsHdop,2);
    pkt += ',' + String(gpsSpeedKmph,2);

    // Append IST timestamp as last CSV field (22nd field)
    pkt += ',' + istTs;

    // Append ACK if pending (this adds 2 extra fields, making 24 total)
    if(appendAckToNextTelemetry && pendingAckToken.length() > 0){
        pkt += ",ACK," + pendingAckToken;
        appendAckToNextTelemetry = false;
        pendingAckToken = "";
    }

    // 1. LoRa send
    LoRa.beginPacket();
    LoRa.print(pkt);
    LoRa.endPacket();
    beepTX();

    // 2. ESP-NOW send (redundant link)
    // Copy String to the fixed-size char array for ESP-NOW transmission
    pkt.toCharArray(espNowPacket.csv, sizeof(espNowPacket.csv));
    esp_now_send(ESP32_B_MAC, (uint8_t*)&espNowPacket, sizeof(espNowPacket));


    // 3. SD log: prepend timestamp as millis() and then the full pkt
    String logline;
    logline.reserve(pkt.length() + 20);
    logline += String(millis());
    logline += ',';
    logline += pkt;
    sdAppendLine(logline);

    // Serial debug (brief)
    Serial.print("[TX TEL] ");
    Serial.println(pkt);
}

// ---------- Setup ----------
void setup(){
    Serial.begin(115200);
    delay(200);
    Serial.println("\n[ONBOARD] init");

    pinMode(BUZZER_PIN, OUTPUT); digitalWrite(BUZZER_PIN, LOW);
    pinMode(BATT_ADC_PIN, INPUT);

    // I2C
    Wire.begin(I2C_SDA, I2C_SCL);

    // BME
    bmeOK = bme.begin(BME_ADDR);
    if(bmeOK) Serial.println("BME280 OK");
    else Serial.println("BME280 NOT FOUND");

    // IMU
    imuOK = initIMU();
    if(imuOK) Serial.println("MPU init (basic)");
    else Serial.println("MPU init failed");

    // Servos
    servo1.attach(SERVO1_PIN);
    servo2.attach(SERVO2_PIN);
    setServos(90,90);

    // SPI + LoRa
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if(!LoRa.begin(LORA_FREQ)){
        Serial.println("LoRa init FAILED - check wiring and 3.3V");
        while(1){ delay(500); } // halt if LoRa essential
    }
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setSyncWord(0x34);
    Serial.println("LoRa OK");
    
    // ** Initialize ESP-NOW **
    initESPNOW();

    // SD init (nonfatal)
    if(SD.begin(SD_CS, SPI)){
        sdOK = true;
        Serial.println("SD initialized");
        if(!SD.exists("/flight_log.csv")){
            File h = SD.open("/flight_log.csv", FILE_WRITE);
            if(h){
                // header columns
                h.println("ts_ms,STATE,temp,hum,press,altCm,battPct,pitch,roll,yaw,ax,ay,az,gx,gy,gz,lat,lon,gpsFix,sats,hdop,speed_kmph,IST");
                h.close();
            }
        }
    } else {
        sdOK = false;
        Serial.println("SD init failed (continuing without SD)");
    }

    // GPS Serial2
    Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("GPS serial2 started");

    // calibration start
    calibStartTime = millis();
    altitudeCalibrated = false;
    imuCalibrated = false;
    appendAckToNextTelemetry = false;
    pendingAckToken = "";

    Serial.println("Setup complete");
}

// ---------- Main loop ----------
void loop(){
    // read GPS bytes continuously
    while(Serial2.available()) gps.encode(Serial2.read());

    unsigned long now = millis();
    if(now - lastTelemetry >= telemetryInterval){
        lastTelemetry = now;
        sendTelemetry();
    }

    // LoRa command handling
    int packetSize = LoRa.parsePacket();
    if(packetSize){
        String incoming;
        while(LoRa.available()) incoming += (char)LoRa.read();
        incoming.trim();
        String upper = incoming; upper.toUpperCase();

        // Handle PING without token
        if(upper == "PING"){
            LoRa.beginPacket(); LoRa.print("TXRX_OK"); LoRa.endPacket(); beepRX();
            Serial.println("[RX CMD] PING -> TXRX_OK");
            return;
        }
        
        // Handle PING with token (sends ACK repeatedly)
        if(upper.startsWith("PING,")){
            int pos = incoming.indexOf(',');
            if(pos >= 0){
                String token = incoming.substring(pos + 1); token.trim();
                if(token.length() > 0){
                    for(int i=0;i<ACK_REPEATS;i++){ 
                        delay(ACK_DELAY_MS); 
                        LoRa.beginPacket(); 
                        LoRa.print("ACK," + token); 
                        LoRa.endPacket(); 
                    }
                    // Set flag to append ACK to the next telemetry packet
                    appendAckToNextTelemetry = true;
                    pendingAckToken = token;
                    beepRX();
                    Serial.print("[RX CMD] PING, token -> ACK sent x"); Serial.println(ACK_REPEATS);
                    return;
                }
            }
        }

        // Handle servo/release commands
        if(handleServoMessage(incoming)){
            beepRX();
            Serial.print("[RX CMD] servo -> "); Serial.println(incoming);
        } else {
            Serial.print("[RX CMD] ignored -> "); Serial.println(incoming);
        }
    }
}
