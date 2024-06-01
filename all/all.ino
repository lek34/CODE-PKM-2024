#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

// WiFi credentials
const char* ssid = "Belo";
const char* password = "Stevenjovin";

// Sensor and endpoint configurations
#define TdsSensorPin 27
#define VREF 3.3
#define SCOUNT 30
#define DHTPIN 5
#define DHTTYPE DHT11

const char* URL = "http://202.10.36.154:8888/insertdata";
const char* relayStatusUrl = "http://192.168.238.190:8000/relaystatus/1";
const char* configUrl = "http://192.168.238.190:8000/getconfig";
const char* updateRelayUrl = "http://192.168.238.190:8000/updaterelay";

// Pins for relays
const int relayPins[] = {14, 27, 26, 25, 33, 32};

// TDS variables
int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;

// pH variables
const int ph_Pin = 35;
int phBuffer[SCOUNT];
int phBufferTemp[SCOUNT];
int phBufferIndex = 0;
float Po = 0;
float PH_step;
int nilai_analog_ph;
double TeganganPh;
float PH4 = 3.1;
float PH7 = 2.4;

// DHT sensor
DHT dht(DHTPIN, DHTTYPE);
float t = 0;
float h = 0;

// Relay control variables
unsigned long lastOnTime[] = {0, 0, 0, 0, 0, 0};
int timeouts[] = {0, 0, 0, 0, 0, 0};
bool relayStatus[] = {false, false, false, false, false, false};

void setup() {
  Serial.begin(9600);
  connectWiFi();
  pinMode(ph_Pin, INPUT);
  pinMode(TdsSensorPin, INPUT);
  dht.begin();

  for (int i = 0; i < 6; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }

  fetchConfig();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  readTDS();
  readPH();
  readSuhu();
  sendSensorData();
  controlRelays();

  delay(1000);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++) {
    bTab[i] = bArray[i];
  }

  for (int j = 0; j < iFilterLen - 1; j++) {
    for (int i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        int bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }

  if ((iFilterLen & 1) > 0) {
    return bTab[(iFilterLen - 1) / 2];
  } else {
    return (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
}

void readTDS() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex = (analogBufferIndex + 1) % SCOUNT;
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    memcpy(analogBufferTemp, analogBuffer, sizeof(analogBuffer));
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * VREF / 4096.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVoltage = averageVoltage / compensationCoefficient;
    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
  }
}

void readPH() {
  static unsigned long phSampleTimepoint = millis();
  if (millis() - phSampleTimepoint > 40U) {
    phSampleTimepoint = millis();
    phBuffer[phBufferIndex] = analogRead(ph_Pin);
    phBufferIndex = (phBufferIndex + 1) % SCOUNT;
  }

  static unsigned long phPrintTimepoint = millis();
  if (millis() - phPrintTimepoint > 800U) {
    phPrintTimepoint = millis();
    memcpy(phBufferTemp, phBuffer, sizeof(phBuffer));
    nilai_analog_ph = getMedianNum(phBufferTemp, SCOUNT);
    TeganganPh = 3.3 / 4096 * nilai_analog_ph;
    PH_step = (PH4 - PH7) / 3;
    Po = 7.00 + ((PH7 - TeganganPh) / PH_step);
  }
}

void readSuhu() {
  t = dht.readTemperature();
  h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {
    Serial.println("Sensor tidak terbaca!");
  }
}

void sendSensorData() {
  if (phBufferIndex == 0 && analogBufferIndex == 0) {
    String postData = "ph=" + String(Po) + "&tds=" + String(tdsValue) + "&temperature=" + String(t) + "&humidity=" + String(h);
    HTTPClient http;
    http.begin(URL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST(postData);
    String payload = http.getString();

    Serial.print("URL: ");
    Serial.println(URL);
    Serial.print("Data: ");
    Serial.println(postData);
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);
    Serial.print("Payload: ");
    Serial.println(payload);
    Serial.println("--------------------------------------------------");

    delay(10000); // Delay for 10 seconds before sending the next data
  }
}

void fetchConfig() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(configUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);

      const size_t capacity = JSON_OBJECT_SIZE(8) + 200;
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      timeouts[0] = doc["ph_up"];
      timeouts[1] = doc["ph_down"];
      timeouts[2] = doc["nut_a"];
      timeouts[3] = doc["nut_b"];
      timeouts[4] = doc["fan"];
      timeouts[5] = doc["light"];
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

void controlRelays() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(relayStatusUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);

      const size_t capacity = JSON_OBJECT_SIZE(7) + 200;
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      const char* relayStatuses[] = {doc["Relay1_is"], doc["Relay2_is"], doc["Relay3_is"], doc["Relay4_is"], doc["Relay5_is"], doc["Relay6_is"]};
      int is_sync = doc["is_sync"];

      if (is_sync == 0) {
        fetchConfig();
      }

      for (int i = 0; i < 6; i++) {
        updateRelay(relayPins[i], relayStatuses[i], lastOnTime[i], timeouts[i], relayStatus[i]);
      }
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

void updateRelay(int relayPin, const char* status, unsigned long& lastOnTime, int timeout, bool& relayStatus) {
  if (strcmp(status, "ON") == 0) {
    if (!relayStatus) {
      lastOnTime = millis();
      relayStatus = true;
      digitalWrite(relayPin, HIGH);
    }
  } else if (strcmp(status, "OFF") == 0) {
    relayStatus = false;
    digitalWrite(relayPin, LOW);
  }

  if (relayStatus && millis() - lastOnTime >= timeout * 1000UL) {
    relayStatus = false;
    digitalWrite(relayPin, LOW);
    updateRelayStatus(relayPin, relayStatus);
  }
}

void updateRelayStatus(int relayPin, bool status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(updateRelayUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "relay_pin=" + String(relayPin) + "&status=" + (status ? "ON" : "OFF");
    int httpCode = http.POST(postData);

    Serial.print("Update Relay URL: ");
    Serial.println(updateRelayUrl);
    Serial.print("Update Relay Data: ");
    Serial.println(postData);
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);
    String payload = http.getString();
    Serial.print("Payload: ");
    Serial.println(payload);
    Serial.println("--------------------------------------------------");

    http.end();
  }
}
