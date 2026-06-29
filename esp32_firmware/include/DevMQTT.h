#ifndef DEV_MQTT_H
#define DEV_MQTT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "DevRelay.h"
#include "DevWeather.h"
#include "DevDS18B20.h"
#include "DevXYMDSensor.h"

// ── Topic map ────────────────────────────────────────────────────
//  MQTT_BASE/telemetry          pub  JSON ทุกค่า
//  MQTT_BASE/status             pub  online / offline (LWT)
//  MQTT_BASE/relay/1/state      pub  ON / OFF
//  MQTT_BASE/relay/2/state      pub
//  MQTT_BASE/relay/3/state      pub
//  MQTT_BASE/relay/1/set        sub  ON / OFF / TOGGLE
//  MQTT_BASE/relay/2/set        sub
//  MQTT_BASE/relay/3/set        sub

class DevMQTT {
private:
  WiFiClient    wifiClient;
  PubSubClient  client;

  DevRelay*       relay[3];
  DevWeather*     weather;
  DevDS18B20*     ds18;
  DevXYMDSensor*  xymd;

  void (*onRelayChange)() = nullptr;

  unsigned long lastTelemetry = 0;
  bool          connected     = false;

  char topicStatus[sizeof(MQTT_BASE) + 8] = {};
  char topicTelemetryBuf[sizeof(MQTT_BASE) + 11] = {};
  char relayStateTopic[3][sizeof(MQTT_BASE) + 16] = {};
  char relaySetTopic[3][sizeof(MQTT_BASE) + 14] = {};

  // ── Topics ────────────────────────────────────────────────────
  void _initTopics() {
    snprintf(topicStatus, sizeof(topicStatus), "%s/status", MQTT_BASE);
    snprintf(topicTelemetryBuf, sizeof(topicTelemetryBuf), "%s/telemetry", MQTT_BASE);
    for (int i = 0; i < 3; i++) {
      snprintf(relayStateTopic[i], sizeof(relayStateTopic[i]),
               "%s/relay/%d/state", MQTT_BASE, i + 1);
      snprintf(relaySetTopic[i], sizeof(relaySetTopic[i]),
               "%s/relay/%d/set", MQTT_BASE, i + 1);
    }
  }

  // ── Build telemetry JSON ──────────────────────────────────────
  String _buildTelemetry() {
    JsonDocument doc;

    // relay
    JsonObject rel = doc["relay"].to<JsonObject>();
    for (int i = 0; i < 3; i++) {
      char key[2] = { char('1' + i), '\0' };
      rel[key] = relay[i]->getState() ? "ON" : "OFF";
    }

    // DS18B20
    doc["ds18"]["temp"] = roundf(ds18->getTemp() * 100.0f) / 100.0f;
    doc["ds18"]["sim"]  = ds18->isSimMode();

    // XYMD
    doc["xymd"]["temp"] = roundf(xymd->getTemperature() * 10.0f) / 10.0f;
    doc["xymd"]["hum"]  = roundf(xymd->getHumidity() * 10.0f) / 10.0f;
    doc["xymd"]["sim"]  = xymd->isSimMode();

    // Weather
    const WeatherData& w = weather->getData();
    doc["weather"]["valid"]    = w.valid;
    doc["weather"]["temp"]     = roundf(w.temp * 10.0f) / 10.0f;
    doc["weather"]["hum"]      = w.humidity;
    doc["weather"]["rain"]     = w.rainChance;
    doc["weather"]["pm25"]     = roundf(w.pm25 * 10.0f) / 10.0f;
    doc["weather"]["aqi"]      = w.aqi;
    doc["weather"]["aqiLabel"] = aqiLabel(w.aqi);

    // system
    char ip[20];
    IPAddress localIp = WiFi.localIP();
    snprintf(ip, sizeof(ip), "%u.%u.%u.%u", localIp[0], localIp[1], localIp[2], localIp[3]);
    doc["sys"]["ip"]     = ip;
    doc["sys"]["rssi"]   = WiFi.RSSI();
    doc["sys"]["heap"]   = ESP.getFreeHeap();
    doc["sys"]["uptime"] = millis() / 1000UL;

    String out;
    out.reserve(512);
    serializeJson(doc, out);
    return out;
  }

  // ── Publish relay state ───────────────────────────────────────
  void _pubRelayState(int n) {   // n = 1..3
    const char* val = relay[n - 1]->getState() ? "ON" : "OFF";
    client.publish(relayStateTopic[n - 1], val, true);  // retain=true
  }

  // ── Subscribe to all relay/set topics ────────────────────────
  void _subscribeAll() {
    for (int i = 1; i <= 3; i++) {
      client.subscribe(relaySetTopic[i - 1]);
    }
    Serial.printf("[MQTT] Subscribed relay/1-3/set\n");
  }

  // ── Connect / reconnect ───────────────────────────────────────
  bool _connect() {
    bool ok = client.connect(
      MQTT_CLIENT_ID,
      nullptr, nullptr,           // no auth on public broker
      topicStatus, 1, true,       // LWT: QoS1, retain
      "offline"
    );
    if (!ok) {
      Serial.printf("[MQTT] Connect failed rc=%d\n", client.state());
      return false;
    }

    // publish online
    client.publish(topicStatus, "online", true);

    _subscribeAll();

    // publish current relay states immediately
    for (int i = 1; i <= 3; i++) _pubRelayState(i);

    connected = true;
    Serial.printf("[MQTT] Connected to %s:%d\n", MQTT_HOST, MQTT_PORT);
    return true;
  }

  // ── Message callback ──────────────────────────────────────────
  static void _callback(char* topic, byte* payload, unsigned int len, DevMQTT* self) {
    String msg;
    msg.reserve(len + 1);
    for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
    msg.toUpperCase();

    Serial.printf("[MQTT] << %s = %s\n", topic, msg.c_str());

    // match relay/N/set
    for (int n = 1; n <= 3; n++) {
      if (strcmp(topic, self->relaySetTopic[n - 1]) == 0) {
        if      (msg == "ON")     self->relay[n-1]->on();
        else if (msg == "OFF")    self->relay[n-1]->off();
        else if (msg == "TOGGLE") self->relay[n-1]->toggle();
        else break;

        self->_pubRelayState(n);
        if (self->onRelayChange) self->onRelayChange();

        Serial.printf("[MQTT] Relay%d → %s\n",
                      n, self->relay[n-1]->getState() ? "ON" : "OFF");
        break;
      }
    }
  }

public:
  DevMQTT(DevRelay* r1, DevRelay* r2, DevRelay* r3,
          DevWeather* wth, DevDS18B20* d18, DevXYMDSensor* xym)
    : client(wifiClient), weather(wth), ds18(d18), xymd(xym) {
    relay[0] = r1; relay[1] = r2; relay[2] = r3;
  }

  void setOnRelayChange(void (*cb)()) { onRelayChange = cb; }

  void begin() {
    _initTopics();
    client.setServer(MQTT_HOST, MQTT_PORT);
    client.setBufferSize(1024);
    // ส่ง this ผ่าน lambda capture
    client.setCallback([this](char* t, byte* p, unsigned int l) {
      _callback(t, p, l, this);
    });
    _connect();
  }

  // เรียกใน loop() ──────────────────────────────────────────────
  void loop() {
    // reconnect ถ้าหลุด
    if (!client.connected()) {
      connected = false;
      static unsigned long lastRetry = 0;
      if (millis() - lastRetry > 5000) {
        lastRetry = millis();
        _connect();
      }
      return;
    }

    client.loop();

    // telemetry publish
    if (millis() - lastTelemetry >= MQTT_TELEMETRY_INTERVAL) {
      lastTelemetry = millis();
      String payload = _buildTelemetry();
      client.publish(topicTelemetryBuf, payload.c_str());
      Serial.printf("[MQTT] >> telemetry (%u bytes)\n", payload.length());
    }
  }

  // เรียกเมื่อ relay เปลี่ยนจาก switch/web ─────────────────────
  void publishRelayState(int n) {   // n = 1..3
    if (client.connected()) _pubRelayState(n);
  }

  bool isConnected() { return client.connected(); }

  // Topic strings สำหรับแสดงบน dashboard / OLED
  const char* topicTelemetry() const { return topicTelemetryBuf; }
  const char* topicStatusText() const { return topicStatus; }
  const char* topicRelayState(int n) const { return relayStateTopic[n - 1]; }
  const char* topicRelaySet(int n) const { return relaySetTopic[n - 1]; }
};

#endif // DEV_MQTT_H
