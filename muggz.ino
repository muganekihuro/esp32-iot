#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "";
const char* password = "";
const char* serverUrl = "https://iot-639m.onrender.com"; 
const char* dataEndpoint = "/data";
const char* pumpEndpoint = "/pump";

const int soilMoisturePin = 34; 
const int pumpPin = 4; 
const int minSoilMoistureThreshold = 40;
const int maxSoilMoistureThreshold = 80;

void setup() {
  Serial.begin(115200);
  delay(4000); 

  connectToWiFi(); 

  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW); 
}

void loop() {
  int soil_moisture_raw = readSoilMoisture(); 
  int soil_moisture_percentage = map(soil_moisture_raw, 4095, 0, 0, 100); 

  Serial.print("Soil Moisture Percentage: ");
  Serial.println(soil_moisture_percentage); 


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

  sendSoilMoistureData(soil_moisture_percentage); 

  delay(15000); 
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

  String url = String(serverUrl) + dataEndpoint; 

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String requestBody = "{\"soil_moisture\": " + String(soil_moisture_percentage) + "}";

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    String payload = http.getString();
    Serial.print("Server response: ");
    Serial.println(payload); 
  } else {
    Serial.print("Error in HTTP POST request. HTTP Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void sendPumpStateData(bool isActive) {
  HTTPClient http;

  String url = String(serverUrl) + pumpEndpoint; 

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
    Serial.println(payload); 
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
