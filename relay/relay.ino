#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Galaxy A52s 5GD68D";
const char* password = "12345678";

// Constants for the pin numbers
const int pin1 = 14;
const int pin2 = 27;
const int pin3 = 26;
const int pin4 = 25;
const int pin5 = 33;
const int pin6 = 32;

// URLs of the API endpoints
const char* relayStatusUrl = "http://192.168.238.190:8000/relaystatus/1";
const char* configUrl = "http://192.168.238.190:8000/getconfig";
const char* updateRelayUrl = "http://192.168.238.190:8000/updaterelay";

// Variables to track the last time each relay was turned on
unsigned long lastOnTime1 = 0;
unsigned long lastOnTime2 = 0;
unsigned long lastOnTime3 = 0;
unsigned long lastOnTime4 = 0;
unsigned long lastOnTime5 = 0;
unsigned long lastOnTime6 = 0;

// Variables to store configuration values
int timeout1 = 0;
int timeout2 = 0;
int timeout3 = 0;
int timeout4 = 0;
int timeout5 = 0;
int timeout6 = 0;

// Variables to store relay statuses
bool relay1_on = false;
bool relay2_on = false;
bool relay3_on = false;
bool relay4_on = false;
bool relay5_on = false;
bool relay6_on = false;

void setup() {
  Serial.begin(9600);
  connectWiFi();
  fetchConfig();
  // Initialize the digital pins as outputs and set them to LOW (OFF)
  pinMode(pin1, OUTPUT);
  digitalWrite(pin1, LOW);
  pinMode(pin2, OUTPUT);
  digitalWrite(pin2, LOW);
  pinMode(pin3, OUTPUT);
  digitalWrite(pin3, LOW);
  pinMode(pin4, OUTPUT);
  digitalWrite(pin4, LOW);
  pinMode(pin5, OUTPUT);
  digitalWrite(pin5, LOW);
  pinMode(pin6, OUTPUT);
  digitalWrite(pin6, LOW);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(relayStatusUrl);
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);

        // Allocate a JSON document
        const size_t capacity = JSON_OBJECT_SIZE(7) + 200;
        DynamicJsonDocument doc(capacity);

        // Parse the JSON payload
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }

        // Get the relay statuses and is_sync from the JSON document
        const char* relay1_status = doc["Relay1_is"];
        const char* relay2_status = doc["Relay2_is"];
        const char* relay3_status = doc["Relay3_is"];
        const char* relay4_status = doc["Relay4_is"];
        const char* relay5_status = doc["Relay5_is"];
        const char* relay6_status = doc["Relay6_is"];
        int is_sync = doc["is_sync"];

        // If is_sync is 0, fetch the configuration again
        if (is_sync == 0) {
          fetchConfig();
        }

        // Update the pins based on the relay statuses and track on time
        updateRelay(pin1, relay1_status, lastOnTime1, timeout1, relay1_on);
        updateRelay(pin2, relay2_status, lastOnTime2, timeout2, relay2_on);
        updateRelay(pin3, relay3_status, lastOnTime3, timeout3, relay3_on);
        updateRelay(pin4, relay4_status, lastOnTime4, timeout4, relay4_on);
        updateRelay(pin5, relay5_status, lastOnTime5, timeout5, relay5_on);
        updateRelay(pin6, relay6_status, lastOnTime6, timeout6, relay6_on);
      }
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    connectWiFi();
  }

  // Wait for 1 second before the next request
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

  Serial.print("connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void fetchConfig() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(configUrl);
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);

        // Allocate a JSON document
        const size_t capacity = JSON_OBJECT_SIZE(8) + 200;
        DynamicJsonDocument doc(capacity);

        // Parse the JSON payload
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }

        // Get the configuration values from the JSON document
        timeout1 = doc["ph_up"];
        timeout2 = doc["ph_down"];
        timeout3 = doc["nut_a"];
        timeout4 = doc["nut_b"];
        timeout5 = doc["fan"];
        timeout6 = doc["light"];
      }
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

void updateRelay(int pin, const char* status, unsigned long& lastOnTime, int timeout, bool& relay_on) {
  unsigned long currentMillis = millis();

  if (strcmp(status, "on") == 0) {
    if (digitalRead(pin) == LOW) {
      // Relay is being turned on, record the time
      lastOnTime = currentMillis;
      digitalWrite(pin, HIGH);
      relay_on = true;
    } else if (currentMillis - lastOnTime >= timeout * 1000) {
      // Relay has been on for the configured timeout, turn it off
      digitalWrite(pin, LOW);
      relay_on = false;
      Serial.print("Relay on pin ");
      Serial.print(pin);
      Serial.println(" turned off after timeout.");
      sendRelayStatus();
    }
  } else {
    // Relay should be off
    digitalWrite(pin, LOW);
    relay_on = false;
  }
}

void sendRelayStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(updateRelayUrl);

    // Construct the POST data string
    String postData = "Relay_1=" + String(relay1_on ? "0" : "0") +
                      "&Relay_2=" + String(relay2_on ? "0" : "0") +
                      "&Relay_3=" + String(relay3_on ? "0" : "0") +
                      "&Relay_4=" + String(relay4_on ? "0" : "0") +
                      "&Relay_5=" + String(relay5_on ? "0" : "0") +
                      "&Relay_6=" + String(relay6_on ? "0" : "0");

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST(postData);
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.printf("POST request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}
