#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <Preferences.h>

#include "config.h"
#include "types.h"
#include "ui_renderer.h"
#include "icon_manager.h"

// Cấu hình chân SPI
#define TFT_MOSI 6
#define TFT_SCLK 4
#define TFT_CS   7
#define TFT_DC   2
#define TFT_RST  3
#define TFT_BL   8

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
UIRenderer renderer(tft);

SystemState sysState = {LAYOUT_FULL, false};
WeatherData currentWeather = {0, 0, "---", "--", 0, "--:--"};
DateTimeData currentTime = {0, 0, 0, 1, 1, 2026, "Mon"};

PetData petData = {0, {0}};
Preferences preferences;

unsigned long prevSecondMillis = 0;
volatile bool btnPressed = false;

double latitude = 10.8231;
double longitude = 106.6297;
int utcOffsetSeconds = 25200; // GMT+7 mặc định
bool ntpSynced = false;
int lastGeoCode = 0;
int lastWeatherCode = 0;

// Biến đếm giờ nội bộ bằng millis() — không cần gọi NTP mỗi giây
time_t baseEpoch = 0;
unsigned long baseMillis = 0;

// Biến quản lý thời gian cập nhật thời tiết
unsigned long lastWeatherUpdate = 0;
unsigned long weatherRetryInterval = WEATHER_UPDATE_INTERVAL;

void IRAM_ATTR handleButtonInterrupt() { 
  btnPressed = true; 
}

// ============ JSON PARSER ============

String parseJsonString(const String& json, const String& key) {
  String searchKey = "\"" + key + "\"";
  int keyIdx = json.indexOf(searchKey);
  if (keyIdx == -1) return "";
  int colonIdx = json.indexOf(':', keyIdx + searchKey.length());
  if (colonIdx == -1) return "";
  int quoteStart = json.indexOf('"', colonIdx + 1);
  if (quoteStart == -1) return "";
  int quoteEnd = json.indexOf('"', quoteStart + 1);
  if (quoteEnd == -1) return "";
  return json.substring(quoteStart + 1, quoteEnd);
}

double parseJsonNumber(const String& json, const String& key, int startPos = 0) {
  String searchKey = "\"" + key + "\"";
  int keyIdx = json.indexOf(searchKey, startPos);
  if (keyIdx == -1) return -99999.0;
  int colonIdx = json.indexOf(':', keyIdx + searchKey.length());
  if (colonIdx == -1) return -99999.0;
  int valStart = colonIdx + 1;
  while (valStart < (int)json.length() && json[valStart] == ' ') valStart++;
  int valEnd = valStart;
  while (valEnd < (int)json.length()) {
    char c = json[valEnd];
    if (c == ',' || c == '}' || c == ']' || c == ' ' || c == '\n') break;
    valEnd++;
  }
  return json.substring(valStart, valEnd).toDouble();
}

// ============ WIFI ============

void connectWiFi() {
  Serial.print("[WiFi] Connecting to: ");
  Serial.println(WIFI_SSID);
  renderer.drawConnectingScreen(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  
  for (int retry = 0; retry < 3; retry++) {
    Serial.printf("[WiFi] Attempt %d/3...\n", retry + 1);
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[WiFi] Connected!");
      Serial.print("[WiFi] IP: ");
      Serial.println(WiFi.localIP());
      sysState.wifiConnected = true;
      delay(500);
      return;
    }
    Serial.printf("\n[WiFi] Attempt %d failed.\n", retry + 1);
  }
  
  Serial.println("[WiFi] All 3 attempts failed!");
  sysState.wifiConnected = false;
}

// ============ NTP (chỉ sync 1 lần, sau đó dùng millis đếm) ============

void syncNTP(int maxAttempts = 10) {
  Serial.printf("[NTP] UTC offset: %d sec\n", utcOffsetSeconds);
  configTime(utcOffsetSeconds, 0, "pool.ntp.org", "time.nist.gov");
  
  Serial.print("[NTP] Syncing");
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo, 1000) && attempts < maxAttempts) {
    Serial.print(".");
    attempts++;
  }
  
  if (getLocalTime(&timeinfo, 500)) {
    ntpSynced = true;
    baseEpoch = mktime(&timeinfo);   // Lưu epoch hiện tại
    baseMillis = millis();            // Lưu millis tại thời điểm sync
    Serial.printf("\n[NTP] OK: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    ntpSynced = false;
    baseEpoch = 0;
    baseMillis = millis();
    Serial.println("\n[NTP] FAILED");
  }
}

// Cập nhật giờ bằng millis() offset — KHÔNG gọi NTP, KHÔNG block
void updateTimeFromMillis() {
  time_t now = baseEpoch + (millis() - baseMillis) / 1000;
  struct tm* t = localtime(&now);
  if (t) {
    currentTime.second = t->tm_sec;
    currentTime.minute = t->tm_min;
    currentTime.hour   = t->tm_hour;
    currentTime.day    = t->tm_mday;
    currentTime.month  = t->tm_mon + 1;
    currentTime.year   = t->tm_year + 1900;
    const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    currentTime.dayOfWeek = days[t->tm_wday];
  }
}

// ============ GEOLOCATION ============

void fetchGeolocation() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;
  String url = "http://ip-api.com/json/?fields=status,city,countryCode,lat,lon,offset";
  Serial.println("[GEO] GET: " + url);
  
  http.begin(url);
  http.setTimeout(10000);
  lastGeoCode = http.GET();
  
  if (lastGeoCode == 200) {
    String payload = http.getString();
    Serial.println("[GEO] " + payload);
    
    String status = parseJsonString(payload, "status");
    if (status == "success") {
      double lat = parseJsonNumber(payload, "lat");
      double lon = parseJsonNumber(payload, "lon");
      String city = parseJsonString(payload, "city");
      String country = parseJsonString(payload, "countryCode");
      double offset = parseJsonNumber(payload, "offset");
      
      if (lat > -99990) latitude = lat;
      if (lon > -99990) longitude = lon;
      if (offset > -99990) utcOffsetSeconds = (int)offset;
      if (city.length() > 0) currentWeather.city = city;
      if (country.length() > 0) currentWeather.country = country;
    }
  } else {
    Serial.printf("[GEO] Error: %d\n", lastGeoCode);
  }
  http.end();
}

// ============ WEATHER (wttr.in siêu nhẹ ~30 bytes response) ============

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(latitude, 4) +
               "&lon=" + String(longitude, 4) +
               "&appid=" + String(OWM_API_KEY) +
               "&units=metric";
  
  Serial.print("[WEATHER] GET: ");
  Serial.println(url);
  
  http.begin(url);
  http.setTimeout(10000);
  lastWeatherCode = http.GET();
  
  if (lastWeatherCode == 200) {
    String payload = http.getString();
    
    double tempVal = parseJsonNumber(payload, "temp");
    double humidVal = parseJsonNumber(payload, "humidity");
    
    int weatherIdx = payload.indexOf("\"weather\"");
    int wCode = 800;
    if (weatherIdx != -1) {
      double parsedCode = parseJsonNumber(payload, "id", weatherIdx);
      if (parsedCode > -99990) {
        wCode = (int)parsedCode;
      }
    }
    
    if (tempVal > -99990) currentWeather.temp = (int)(tempVal + (tempVal >= 0 ? 0.5 : -0.5));
    if (humidVal > -99990) currentWeather.humidity = (int)humidVal;
    currentWeather.weatherCode = wCode;
    
    updateTimeFromMillis();
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", currentTime.hour, currentTime.minute);
    currentWeather.lastUpdated = String(buf);
    
    Serial.printf("[WEATHER] OK! temp=%dC humid=%d%% wcode=%d\n", currentWeather.temp, currentWeather.humidity, currentWeather.weatherCode);
  } else {
    String errorPayload = http.getString();
    Serial.printf("[WEATHER] Error %d: %s\n", lastWeatherCode, errorPayload.c_str());
  }
  http.end();
}

// ============ SETUP ============

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== ESP32-C3 Weather Clock v14 ===");
  
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleButtonInterrupt, FALLING);
  
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, 80);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 240);
  tft.setRotation(0);
  renderer.initScreen();
  
  connectWiFi();
  
  if (WiFi.status() == WL_CONNECTED) {
    fetchGeolocation();
    syncNTP();
    fetchWeather();
    if (lastWeatherCode == 200) {
      lastWeatherUpdate = millis();
    }
  } else {
    syncNTP();
  }
  
  updateTimeFromMillis();
  
  preferences.begin("bongo", false);
  petData.todayKeystrokes = preferences.getUInt("today", 0);
  petData.maxStreak = preferences.getUInt("maxStreak", 0);
  petData.currentStreak = 0;
  petData.lastKeystrokeTime = millis();
  preferences.getBytes("history", petData.history, sizeof(petData.history));
  
  renderer.drawStaticLayout(currentWeather, currentTime, sysState.currentLayout, &petData);
  renderer.updateDynamicUI(currentWeather, currentTime, sysState.currentLayout, true, &petData);
}

// ============ LOOP (KHÔNG CÓ DELAY, KHÔNG BLOCK) ============

void loop() {
  unsigned long currentMillis = millis();

  if (btnPressed) {
    delay(50);
    if (digitalRead(BTN_PIN) == LOW) {
      int nextLayout = (int)sysState.currentLayout + 1;
      if (nextLayout >= LAYOUT_COUNT) nextLayout = 0;
      sysState.currentLayout = (LayoutMode)nextLayout;
      
      renderer.drawStaticLayout(currentWeather, currentTime, sysState.currentLayout, &petData);
      renderer.updateDynamicUI(currentWeather, currentTime, sysState.currentLayout, true, &petData);
    }
    btnPressed = false;
  }

  if (currentMillis - prevSecondMillis >= 1000) {
    prevSecondMillis = currentMillis;
    updateTimeFromMillis();
    renderer.updateDynamicUI(currentWeather, currentTime, sysState.currentLayout, false, &petData);
  }

  static unsigned long lastNtpRetry = 0;
  if (WiFi.status() == WL_CONNECTED && !ntpSynced) {
    if (currentMillis - lastNtpRetry >= 10000) {
      lastNtpRetry = currentMillis;
      Serial.println("[NTP] Retrying sync...");
      syncNTP(3);
    }
  }

  static unsigned long reconnectStart = 0;
  static bool apiDataLoaded = false;
  
  if (WiFi.status() != WL_CONNECTED) {
    if (reconnectStart == 0 || currentMillis - reconnectStart >= 30000) {
      reconnectStart = currentMillis;
      Serial.println("[WiFi] Reconnecting (non-blocking)...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  } else if (!apiDataLoaded && lastGeoCode != 200) {
    Serial.println("[RECOVER] WiFi OK! Loading API...");
    fetchGeolocation();
    if (lastGeoCode == 200) {
      syncNTP();
      updateTimeFromMillis();
    }
    fetchWeather();
    apiDataLoaded = true;
    renderer.drawStaticLayout(currentWeather, currentTime, sysState.currentLayout, &petData);
    renderer.updateDynamicUI(currentWeather, currentTime, sysState.currentLayout, true, &petData);
  }

  static bool wasConnected = true;
  bool isConnected = (WiFi.status() == WL_CONNECTED);
  if (isConnected && !wasConnected) {
    Serial.println("[WiFi] Connected! Triggering immediate weather update.");
    lastWeatherUpdate = 0;
  }
  wasConnected = isConnected;

  if (isConnected) {
    if (lastWeatherUpdate == 0 || currentMillis - lastWeatherUpdate >= weatherRetryInterval) {
      fetchWeather();
      lastWeatherUpdate = currentMillis;
      
      if (lastWeatherCode == 200) {
        weatherRetryInterval = WEATHER_UPDATE_INTERVAL;
      } else {
        weatherRetryInterval = 60000;
        Serial.println("[WEATHER] Update failed. Will retry in 1 minute.");
      }
      
      renderer.drawStaticLayout(currentWeather, currentTime, sysState.currentLayout, &petData);
      renderer.updateDynamicUI(currentWeather, currentTime, sysState.currentLayout, true, &petData);
    }
  }

  if (petData.currentStreak > 0 && currentMillis - petData.lastKeystrokeTime > 3000) {
    petData.currentStreak = 0;
    if (sysState.currentLayout == LAYOUT_BONGO_CAT) {
      renderer.updateDynamicUI(currentWeather, currentTime, sysState.currentLayout, true, &petData);
    }
  }

  static unsigned long lastBongoHit = 0;
  static bool bongoIsDown = false;
  static unsigned long lastSerialKeystroke = 0;
  static String serialBuffer = "";
  
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      serialBuffer.trim();
      if (serialBuffer.length() >= 3 && serialBuffer.startsWith("K:")) {
        char key = serialBuffer.charAt(2);
        if (key == 'L' || key == 'R' || key == 'B') {
          petData.todayKeystrokes++;
          
          if (currentMillis - petData.lastKeystrokeTime < 3000) {
            petData.currentStreak++;
          } else {
            petData.currentStreak = 1;
          }
          petData.lastKeystrokeTime = currentMillis;
          if (petData.currentStreak > petData.maxStreak) {
            petData.maxStreak = petData.currentStreak;
          }
          
          if (petData.todayKeystrokes % 100 == 0) {
            preferences.putUInt("today", petData.todayKeystrokes);
            preferences.putUInt("maxStreak", petData.maxStreak);
          }
          
          if (sysState.currentLayout == LAYOUT_BONGO_CAT) {
            int state = (key == 'L') ? 1 : (key == 'R') ? 2 : 3;
            renderer.updateBongoAnimation(state);
            renderer.updateDynamicUI(currentWeather, currentTime, sysState.currentLayout, true, &petData);
            lastBongoHit = currentMillis;
            bongoIsDown = true;
          }
        }
      }
      serialBuffer = "";
    } else {
      if (serialBuffer.length() < 10) {
        serialBuffer += c;
      }
    }
  }
  
  if (bongoIsDown && sysState.currentLayout == LAYOUT_BONGO_CAT) {
    if (currentMillis - lastBongoHit >= 150) {
      renderer.updateBongoAnimation(0);
      bongoIsDown = false;
    }
  }

  static int lastDay = -1;
  if (lastDay == -1 && currentTime.day > 0) {
    lastDay = currentTime.day;
  }
  if (lastDay != -1 && currentTime.year > 2024 && currentTime.day != lastDay) {
    lastDay = currentTime.day;
    for (int i = 6; i > 0; i--) {
      petData.history[i] = petData.history[i-1];
    }
    petData.history[0] = petData.todayKeystrokes;
    petData.todayKeystrokes = 0;
    
    preferences.putUInt("today", petData.todayKeystrokes);
    preferences.putBytes("history", petData.history, sizeof(petData.history));
    preferences.putInt("day", currentTime.day);
    
    if (sysState.currentLayout == LAYOUT_BONGO_CAT) {
      renderer.drawStaticLayout(currentWeather, currentTime, sysState.currentLayout, &petData);
    }
  }
}
