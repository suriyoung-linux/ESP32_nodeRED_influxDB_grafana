#ifndef DEV_WEB_SERVER_H
#define DEV_WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "dashboard.h"
#include "config.h"
#include "DevRelay.h"
#include "DevWeather.h"
#include "DevDS18B20.h"
#include "DevXYMDSensor.h"

class DevWebServer {
private:
  AsyncWebServer  server;
  AsyncWebSocket  ws;

  // ชี้ไปยัง objects ใน main.cpp
  DevRelay*       relay[3];
  DevWeather*     weather;
  DevDS18B20*     ds18;
  DevXYMDSensor*  xymd;

  unsigned long lastBroadcast = 0;
  static const unsigned long BROADCAST_INTERVAL = 2000; // ms
  bool mqttConnected = false;

  void (*onRelayChange)() = nullptr; // callback → เรียก _updateDisplay() ใน main.cpp

  char mqttTelemetryTopic[sizeof(MQTT_BASE) + 11] = {};
  char mqttStatusTopic[sizeof(MQTT_BASE) + 8] = {};
  char mqttRelayStateTopic[3][sizeof(MQTT_BASE) + 16] = {};
  char mqttRelaySetTopic[3][sizeof(MQTT_BASE) + 14] = {};

  void initMqttTopics() {
    snprintf(mqttTelemetryTopic, sizeof(mqttTelemetryTopic), "%s/telemetry", MQTT_BASE);
    snprintf(mqttStatusTopic, sizeof(mqttStatusTopic), "%s/status", MQTT_BASE);
    for (int i = 0; i < 3; i++) {
      snprintf(mqttRelayStateTopic[i], sizeof(mqttRelayStateTopic[i]),
               "%s/relay/%d/state", MQTT_BASE, i + 1);
      snprintf(mqttRelaySetTopic[i], sizeof(mqttRelaySetTopic[i]),
               "%s/relay/%d/set", MQTT_BASE, i + 1);
    }
  }

  // ─── สร้าง JSON payload ───────────────────────────────────────
  String buildJson() {
    JsonDocument doc;

    // relay array [r1, r2, r3]
    JsonArray relayArr = doc["relay"].to<JsonArray>();
    for (int i = 0; i < 3; i++) {
      relayArr.add(relay[i]->getState());
    }

    // weather
    const WeatherData& w = weather->getData();
    doc["weather"]["valid"]    = w.valid;
    doc["weather"]["temp"]     = roundf(w.temp * 10.0f) / 10.0f;
    doc["weather"]["hum"]      = w.humidity;
    doc["weather"]["rain"]     = w.rainChance;
    doc["weather"]["pm25"]     = roundf(w.pm25 * 10.0f) / 10.0f;
    doc["weather"]["aqi"]      = w.aqi;
    doc["weather"]["aqiLabel"] = aqiLabel(w.aqi);
    doc["weather"]["city"]     = OWM_CITY_NAME;

    // wifi
    char ip[20];
    IPAddress localIp = WiFi.localIP();
    snprintf(ip, sizeof(ip), "%u.%u.%u.%u", localIp[0], localIp[1], localIp[2], localIp[3]);
    doc["wifi"]["ssid"] = WiFi.SSID();
    doc["wifi"]["ip"]   = ip;
    doc["wifi"]["mac"]  = WiFi.macAddress();
    doc["wifi"]["rssi"] = WiFi.RSSI();

    // DS18B20
    doc["ds18"]["temp"] = roundf(ds18->getTemp() * 100.0f) / 100.0f;
    doc["ds18"]["sim"]  = ds18->isSimMode();

    // XYMD
    doc["xymd"]["temp"] = roundf(xymd->getTemperature() * 10.0f) / 10.0f;
    doc["xymd"]["hum"]  = roundf(xymd->getHumidity() * 10.0f) / 10.0f;
    doc["xymd"]["sim"]  = xymd->isSimMode();
    doc["xymd"]["id"]   = xymd->getSlaveID();

    // MQTT topics info (static — ไม่ต้องรู้ connected state ตรงนี้)
    doc["mqtt"]["host"]         = MQTT_HOST;
    doc["mqtt"]["port"]         = MQTT_PORT;
    doc["mqtt"]["base"]         = MQTT_BASE;
    doc["mqtt"]["connected"]    = mqttConnected;
    doc["mqtt"]["t_telemetry"]  = mqttTelemetryTopic;
    doc["mqtt"]["t_status"]     = mqttStatusTopic;
    doc["mqtt"]["t_r1_state"]   = mqttRelayStateTopic[0];
    doc["mqtt"]["t_r2_state"]   = mqttRelayStateTopic[1];
    doc["mqtt"]["t_r3_state"]   = mqttRelayStateTopic[2];
    doc["mqtt"]["t_r1_set"]     = mqttRelaySetTopic[0];
    doc["mqtt"]["t_r2_set"]     = mqttRelaySetTopic[1];
    doc["mqtt"]["t_r3_set"]     = mqttRelaySetTopic[2];

    // system
    doc["sys"]["heap"]   = ESP.getFreeHeap();
    doc["sys"]["uptime"] = millis() / 1000UL;

    String out;
    out.reserve(1024);
    serializeJson(doc, out);
    return out;
  }

  // ─── WebSocket event handler ──────────────────────────────────
  void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                 AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("[WS] Client #%u connected from %s\n",
                    client->id(), client->remoteIP().toString().c_str());
      // ส่ง snapshot ทันทีที่ client เชื่อมต่อ
      client->text(buildJson());

    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("[WS] Client #%u disconnected\n", client->id());

    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo* info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len
          && info->opcode == WS_TEXT) {
        // รับคำสั่งจาก browser
        String msg = String((char*)data, len);
        handleWsMessage(client, msg);
      }
    }
  }

  // ─── ประมวลผลคำสั่งจาก browser ───────────────────────────────
  void handleWsMessage(AsyncWebSocketClient* client, const String& msg) {
    JsonDocument doc;
    if (deserializeJson(doc, msg) != DeserializationError::Ok) return;

    String cmd = doc["cmd"].as<String>();

    if (cmd == "relay") {
      int n = doc["n"].as<int>(); // 1-3
      if (n >= 1 && n <= 3) {
        relay[n - 1]->toggle();
        Serial.printf("[WS] Relay%d toggled → %s\n",
                      n, relay[n-1]->getState() ? "ON" : "OFF");
        // อัปเดต OLED
        if (onRelayChange) onRelayChange();
        // broadcast ทันทีให้ทุก client เห็นพร้อมกัน
        ws.textAll(buildJson());
      }
    }
  }

public:
  DevWebServer(DevRelay* r1, DevRelay* r2, DevRelay* r3,
               DevWeather* wth, DevDS18B20* d18, DevXYMDSensor* xym)
    : server(80), ws("/ws"), weather(wth), ds18(d18), xymd(xym) {
    relay[0] = r1;
    relay[1] = r2;
    relay[2] = r3;
    initMqttTopics();
  }

  void setOnRelayChange(void (*cb)()) { onRelayChange = cb; }
  void setMqttConnected(bool v)       { mqttConnected = v; }

  void begin() {
    // WebSocket handler
    ws.onEvent([this](AsyncWebSocket* s, AsyncWebSocketClient* c,
                      AwsEventType t, void* a, uint8_t* d, size_t l) {
      onWsEvent(s, c, t, a, d, l);
    });
    server.addHandler(&ws);

    // GET / → dashboard HTML
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
      req->send(200, "text/html", DASHBOARD_HTML);
    });

    // GET /api/status → JSON snapshot (สำหรับ REST polling สำรอง)
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
      req->send(200, "application/json", buildJson());
    });

    // 404
    server.onNotFound([](AsyncWebServerRequest* req) {
      req->send(404, "text/plain", "Not found");
    });

    server.begin();
    Serial.printf("[WebServer] Started — http://%s\n",
                  WiFi.localIP().toString().c_str());
  }

  // เรียกใน loop() — broadcast ทุก BROADCAST_INTERVAL ms
  void loop() {
    ws.cleanupClients(); // คืน memory ของ client ที่ disconnect

    unsigned long now = millis();
    if (now - lastBroadcast >= BROADCAST_INTERVAL && ws.count() > 0) {
      lastBroadcast = now;
      ws.textAll(buildJson());
    }
  }
};

#endif // DEV_WEB_SERVER_H
