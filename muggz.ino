#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "";
const char* password = "";
const char* serverUrl = "https://iot-639m.onrender.com"; // Update with your server's IP address
const char* dataEndpoint = "/data";
const char* pumpEndpoint = "/pump";

const int soilMoisturePin = 34; 
const int pumpPin = 4; 
const int minSoilMoistureThreshold = 40;
const int maxSoilMoistureThreshold = 80;

void setup() {
  Serial.begin(115200);
  delay(4000); // Delay to ensure serial monitor is ready

  connectToWiFi(); // Connect to WiFi

  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW); // Ensure pump is initially off
}

void loop() {
  int soil_moisture_raw = readSoilMoisture(); // Read raw soil moisture value
  int soil_moisture_percentage = map(soil_moisture_raw, 4095, 0, 0, 100); // Map raw value to percentage

  Serial.print("Soil Moisture Percentage: ");
  Serial.println(soil_moisture_percentage); // Display soil moisture percentage on Serial Monitor

  // Flag to keep track of pump state changes
  static int prevPumpState = LOW;

  if (soil_moisture_percentage < minSoilMoistureThreshold) {
    digitalWrite(pumpPin, HIGH);
    Serial.println("Pump activated");

    if (prevPumpState != HIGH) {
      sendPumpStateData(true); 
      prevPumpState = HIGH; 
    }
  } else if (soil_moisture_percentage > maxSoilMoistureThreshold) {
    digitalWrite(pumpPin, LOW); 
    Serial.println("Pump deactivated");

    if (prevPumpState != LOW) {
      sendPumpStateData(false); 
      prevPumpState = LOW; 
    }
  }

  sendSoilMoistureData(soil_moisture_percentage); // Send soil moisture percentage to API

  delay(15000); // Wait for 15 seconds before sending next data
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("Connected to WiFi");
}

void sendSoilMoistureData(int soil_moisture_percentage) {
  HTTPClient http;

  String url = String(serverUrl) + dataEndpoint; // Append /data endpoint for sending soil moisture data

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String requestBody = "{\"soil_moisture\": " + String(soil_moisture_percentage) + "}";

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    String payload = http.getString();
    Serial.print("Server response: ");
    Serial.println(payload); // Print server response
  } else {
    Serial.print("Error in HTTP POST request. HTTP Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void sendPumpStateData(bool isActive) {
  HTTPClient http;

  String url = String(serverUrl) + pumpEndpoint; // Append /pump endpoint for sending pump state data

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String state = isActive ? "activated" : "deactivated";
  String requestBody = "{\"pump_state\": \"" + state + "\"}";

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    Serial.print("Pump State HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    String payload = http.getString();
    Serial.print("Server response: ");
    Serial.println(payload); // Print server response
  } else {
    Serial.print("Error in Pump State HTTP POST request. HTTP Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

int readSoilMoisture() {
  int soilMoistureValue = analogRead(soilMoisturePin);
  return soilMoistureValue;
}
