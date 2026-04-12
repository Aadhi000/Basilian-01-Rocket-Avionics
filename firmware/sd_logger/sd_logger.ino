#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <SD.h>
#include <HardwareSerial.h>

// ================= SD PIN CONFIG =================
#define SD_CS   5
#define SD_SCK  18
#define SD_MISO 19
#define SD_MOSI 23

SPIClass sdSPI(VSPI);

// ================= SDS011 =================
#define SDS_RX 16
#define SDS_TX 17
HardwareSerial sdsSerial(2);

// ================= ESP32-A MAC =================
uint8_t ESP32_A_MAC[] = {0xC0,0xCD,0xD6,0x85,0x6E,0xCC};

// ================= STRUCT =================
typedef struct {
  char csv[420];
} ESPNOW_Packet;

volatile bool dataReady = false;
ESPNOW_Packet rxPacket;

// ================= SDS011 DATA =================
float pm25 = -1;
float pm10 = -1;

// ================= FILE =================
String logFile;

// ================= SDS011 READ =================
void readSDS011() {
  static uint8_t buf[10], idx = 0;

  while (sdsSerial.available()) {
    uint8_t b = sdsSerial.read();
    if (idx == 0 && b != 0xAA) continue;
    buf[idx++] = b;

    if (idx == 10) {
      idx = 0;
      if (buf[0] == 0xAA && buf[1] == 0xC0) {
        pm25 = (buf[3]*256 + buf[2]) / 10.0;
        pm10 = (buf[5]*256 + buf[4]) / 10.0;
      }
    }
  }
}

// ================= ESPNOW CALLBACK =================
void onReceive(const esp_now_recv_info *info,
               const uint8_t *data,
               int len) {

  if (memcmp(info->src_addr, ESP32_A_MAC, 6) != 0) return;
  if (len != sizeof(ESPNOW_Packet)) return;

  memcpy(&rxPacket, data, sizeof(rxPacket));
  dataReady = true;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1500);

  // -------- WIFI --------
  WiFi.mode(WIFI_STA);
  delay(500);

  Serial.print("ESP32-B MAC: ");
  Serial.println(WiFi.macAddress());

  // -------- SD --------
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, sdSPI)) {
    Serial.println("SD INIT FAILED");
    while (1);
  }

  logFile = "/log_" + String(millis()) + ".csv";
  Serial.println("LOG FILE: " + logFile);

  File f = SD.open(logFile, FILE_WRITE);
  f.println(
    "STATE,temp,hum,press,altCm,battPct,"
    "pitch,roll,yaw,ax,ay,az,gx,gy,gz,"
    "lat,lon,gpsFix,sats,hdop,speed_kmph,"
    "timestamp,timestamp_ms,pm2.5,pm10"
  );
  f.close();

  // -------- SDS011 --------
  sdsSerial.begin(9600, SERIAL_8N1, SDS_RX, SDS_TX);
  Serial.println("SDS011 READY");

  // -------- ESPNOW --------
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW INIT FAILED");
    while (1);
  }
  esp_now_register_recv_cb(onReceive);

  Serial.println("ESP32-B READY (LOGGER ACTIVE)");
}

// ================= LOOP =================
void loop() {
  readSDS011();

  if (dataReady) {
    dataReady = false;

    String line;
    line.reserve(520);
    line += rxPacket.csv;
    line += ",";
    line += String(millis());
    line += ",";
    line += String(pm25,1);
    line += ",";
    line += String(pm10,1);

    File f = SD.open(logFile, FILE_APPEND);
    if (f) {
      f.println(line);
      f.close();
      Serial.println("LOGGED: " + line);
    } else {
      Serial.println("SD WRITE FAILED");
    }
  }
}
