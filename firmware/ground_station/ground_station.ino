// ===================== GROUND ESP32 =====================
// LoRa Receiver + Telemetry Printer + PING/ACK + Servo CMD
// Fully stable final version for Aadhi

#include <SPI.h>
#include <LoRa.h>

// ----- LoRa Pins -----
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  2
#define LORA_FREQ  433E6

// ----- Buzzer -----
#define BUZZER_PIN 27

// ----- Expected telemetry field count from onboard -----
#define TELEMETRY_FIELDS 22   // without RSSI (it will be added as 23rd field)

// --------------------- BUZZER -------------------------
void beepTX() { digitalWrite(BUZZER_PIN, HIGH); delay(40); digitalWrite(BUZZER_PIN, LOW); }
void beepRX() { digitalWrite(BUZZER_PIN, HIGH); delay(40); digitalWrite(BUZZER_PIN, LOW); }

// -------------- Validate servo angle commands ----------
bool isValidServoAngles(const String &cmd) {
  int c = cmd.indexOf(',');
  if (c < 0) return false;
  String a = cmd.substring(0, c);
  String b = cmd.substring(c + 1);
  a.trim(); b.trim();
  // Check if both parts exist (length > 0)
  return (a.length() > 0 && b.length() > 0);
}

// ---------------------- CSV Splitter --------------------
// Splits the data string into the fields array, up to maxFields.
// If the data string has more than maxFields, the extras are ignored but not counted.
int splitCSV(const String &data, String *fields, int maxFields) {
  int count = 0;
  int start = 0;

  for (int i = 0; i <= data.length(); i++) {
    if (i == data.length() || data[i] == ',') {
      if (count < maxFields)
        fields[count++] = data.substring(start, i);
      start = i + 1;
    }
  }
  return count;
}

// ====================== SETUP ===========================
void setup() {
  Serial.begin(115200);
  delay(700);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // LoRa SPI pins (SCK=18, MISO=19, MOSI=23)
  SPI.begin(18, 19, 23, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  Serial.println("GROUND ESP32 STARTED");

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LORA INIT FAILED — CHECK MODULE");
    while (1);
  }
  Serial.println("LORA INIT OK");

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x34);
  
  // NOTE: Header order corrected to match the On-Board telemetry output order
  Serial.println("STATE,temp,hum,press,altCm,battPct,pitch,roll,yaw,ax,ay,az,gx,gy,gz,lat,lon,gpsFix,sats,hdop,speed_kmph,timestamp,rssi");
}

// ====================== LOOP ============================
void loop() {

  // -------------------- SEND COMMANDS --------------------
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) return;

    String up = cmd; up.toUpperCase();
    bool sendIt = false;

    // PING - send a simple PING or PING,token
    if (up.startsWith("PING")) {
      sendIt = true;
    }
    // Standard commands
    else if (up == "RELEASE" || up == "HOLD" || isValidServoAngles(cmd)) {
      sendIt = true;
    }

    if (sendIt) {
      LoRa.beginPacket();
      LoRa.print(cmd);
      LoRa.endPacket();
      Serial.print("SENT,"); Serial.println(cmd);
      beepTX();
    }
  }

  // -------------------- RECEIVE PACKETS --------------------
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return;

  String incoming = "";
  while (LoRa.available()) incoming += (char)LoRa.read();
  incoming.trim();
  int rssi = LoRa.packetRssi();

  // ---------- Handle ACK responses (packets that are ONLY ACK) ----------
  if (incoming == "TXRX_OK") {
    Serial.print("TXRX_OK,"); Serial.println(rssi);
    beepRX();
    return;
  }
  if (incoming.startsWith("ACK,")) {
    Serial.print(incoming); Serial.print(","); Serial.println(rssi);
    beepRX();
    return;
  }

  // ---------- Handle Telemetry ----------
  // We use TELEMETRY_FIELDS + 1 for the array size to store the 22 fields + RSSI.
  String fields[TELEMETRY_FIELDS + 1];
  
  // splitCSV only fills up to TELEMETRY_FIELDS (22)
  int count = splitCSV(incoming, fields, TELEMETRY_FIELDS);

  // CRITICAL FIX: Ensure we got at least the expected number of fields.
  // If the On-Board appends ACK data, count will still be 22, and we proceed.
  if (count < TELEMETRY_FIELDS) {
    // corrupted / incomplete packet, ignore
    return;
  }

  // Attach RSSI as last field (at index 22, since fields[0] to fields[21] are the data)
  fields[TELEMETRY_FIELDS] = String(rssi);

  // Print full CSV line (23 fields: 22 data + 1 rssi)
  for (int i = 0; i <= TELEMETRY_FIELDS; i++) {
    Serial.print(fields[i]);
    if (i < TELEMETRY_FIELDS) Serial.print(",");
  }
  Serial.println();

  beepRX();
}
