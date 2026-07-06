#ifndef CONFIG_H
#define CONFIG_H

#if __has_include("config_private.h")
#include "config_private.h"
#endif

// ===== OpenWeatherMap =====
// สมัครฟรีที่ https://openweathermap.org/api
// แผน Free รองรับ Current Weather + Air Pollution API
#ifndef OWM_API_KEY
#define OWM_API_KEY   "YOUR_OPENWEATHERMAP_KEY"
#endif

// พิกัด Nonthaburi (ใช้ lat/lon แม่นยำกว่า city name)
#ifndef OWM_LAT
#define OWM_LAT       "13.909"
#endif
#ifndef OWM_LON
#define OWM_LON       "100.424"
#endif
#ifndef OWM_CITY_NAME
#define OWM_CITY_NAME "Nonthaburi"
#endif

// อัปเดตทุกกี่วินาที (Free plan limit: 60 calls/min, แนะนำ 300s+)
#ifndef WEATHER_UPDATE_SEC
#define WEATHER_UPDATE_SEC  300
#endif

// ===== HiveMQ (Free public broker) =====
#ifndef MQTT_HOST
#define MQTT_HOST     "broker.hivemq.com"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT     1883
#endif
// Client ID ควร unique — ใส่ mac address ท้าย 6 ตัวก็ได้
#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID  "esp32-level3"
#endif

// Base topic — เปลี่ยนให้ unique เพื่อไม่ชนกับคนอื่นบน public broker
#ifndef MQTT_BASE
#define MQTT_BASE       "ESP32-Level3"
#endif

// Publish interval (ms)
#ifndef MQTT_TELEMETRY_INTERVAL
#define MQTT_TELEMETRY_INTERVAL  5000
#endif

#endif // CONFIG_H
