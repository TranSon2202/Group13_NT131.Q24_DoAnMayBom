#define SENSOR_PIN 35
#define RELAY_PIN 17
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
const char* ssid = "Xeko";
const char* password = "01011972";
const char* serverName = "https://api-tuoi-cay-g0g2cdfmbkc7dubq.southeastasia-01.azurewebsites.net/api/Pump/Status";
const char* PostDoam = "https://api-tuoi-cay-g0g2cdfmbkc7dubq.southeastasia-01.azurewebsites.net/api/Humidity";
const char* GetDoam = "https://api-tuoi-cay-g0g2cdfmbkc7dubq.southeastasia-01.azurewebsites.net/api/Device/config?deviceName=ESP32";  
// ===== NTP TIME =====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600; // GMT+7
const int daylightOffset_sec = 0;


// ===== MODE =====
bool autoMode = true;
unsigned long lastApiTime = 0;
const long apiTimeout = 60000; // 60s

String lastCommand = ""; // lưu lệnh trước đó

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }


  Serial.println(WiFi.localIP());
}

// =====================================================
// CHECK WATERING TIME
// KHÔNG tưới:
// 10:00 -> 15:00
// 18:00 -> 06:00
// =====================================================
bool allowWatering() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Khong lay duoc gio");
    return true;
  }

  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;

  Serial.print("Time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);

  // block 10:00 -> 14:59
  if (hour >= 10 && hour < 15) {
    return false;
  }

  // block 18:00 -> 05:59
  if (hour >= 18 || hour < 6) {
    return false;
  }

  return true;
}

void setup() {
  Serial.begin(115200);

  initWifi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // tắt bơm ban đầu
}

void loop() {
  bool canWater = allowWatering();

  // ===== 1. ĐỌC CẢM BIẾN =====

  int value = analogRead(SENSOR_PIN);
  Serial.println(value);

  int humidityPercent = map(value, 4095, 2000, 0, 100);
  humidityPercent = constrain(humidityPercent, 0, 100);

  Serial.print("Do am theo %: ");
  Serial.println(humidityPercent);

  int lower = 30;
  int upper = 80;

  
  

  // ===== GET DOAM =====
   if (WiFi.status() == WL_CONNECTED) {
    HTTPClient getdoam;

    getdoam.begin(GetDoam);
    int DoamResponseCode = getdoam.GET();

    if (DoamResponseCode > 0) {
      String payload_DoAm = getdoam.getString();
      Serial.println(payload_DoAm);
      DynamicJsonDocument doc_DoAm(256);

    DeserializationError error_DoAm = deserializeJson(doc_DoAm, payload_DoAm);

    if (error_DoAm) {
    Serial.println("JSON Parse Failed");
    getdoam.end();
    return;
    } 

    int lowerThreshold = doc_DoAm["lowerThreshold"];
    int upperThreshold = doc_DoAm["upperThreshold"];

    // tinh toan ra do am
    
    lower = lowerThreshold;
    upper = upperThreshold;
    
  }
  }

  // ===== POST API =====
  if(WiFi.status() == WL_CONNECTED)
  {
    HTTPClient post;
    post.begin(PostDoam);
    post.addHeader("Content-Type", "application/json");

    String jsonData = "{";
    jsonData += "\"value\": " + String(humidityPercent) + ",";
    jsonData += "\"deviceName\": \"ESP32\"";
    jsonData += "}";

    int responseCode = post.POST(jsonData);

    Serial.print("Response Code: ");
    Serial.println(responseCode);

    String response = post.getString();
    Serial.println(response);

    post.end();
  }
  // ===== 2. GỌI API =====
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      DynamicJsonDocument doc(256);

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
    Serial.println("JSON Parse Failed");
    http.end();
    return;
    } 

      String command = "";

      bool pumpOnOrOff = doc["pumpOnOrOff"];

      // detect ON/OFF
      if (pumpOnOrOff) {
        command = "ON";
      } 
      else{
        command = "OFF";
      }

      // ===== CHỈ XỬ LÝ KHI LỆNH THAY ĐỔI =====
   if (command != "") 
  {
    autoMode = false;
  
  

  // chỉ đổi relay khi lệnh đổi
    if (command != lastCommand) 
    {
      lastCommand = command;
      if (command == "ON") 
      {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("API: ON");
        
      } 
      else 
      {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("API: OFF");
      lastApiTime = millis();
      }
    } 
  } 

    } else {
      Serial.println("API Error");
    }

    http.end();
  }

  // ===== 3. TIMEOUT → QUAY LẠI AUTO =====
  if (!autoMode && millis() - lastApiTime > apiTimeout) {
    autoMode = true;
    Serial.println("Back to AUTO");
  }
  
 

  // ===== 4. AUTO MODE =====
  if (autoMode) 
  {

    
     // ===== BLOCK BY TIME =====
    if (!canWater) {

      digitalWrite(RELAY_PIN, LOW);

      Serial.println("AUTO: BLOCKED BY TIME");
    }
    else
    { 

      // ===== NORMAL AUTO =====
     if (humidityPercent <= lower) {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("AUTO: ON");
      } 
     else if ( humidityPercent >= upper)
     {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("AUTO: OFF");
      }
    }
  }

  delay(2000); // gọi API mỗi 5s
}