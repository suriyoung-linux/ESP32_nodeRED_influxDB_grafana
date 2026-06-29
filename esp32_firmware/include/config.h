#ifndef CONFIG_H
#define CONFIG_H

// ===== OpenWeatherMap =====
// สมัครฟรีที่ https://openweathermap.org/api
// แผน Free รองรับ Current Weather + Air Pollution API
#define OWM_API_KEY   "98caac13bfb0a879ed774f858db5d21c"

// พิกัด Nonthaburi (ใช้ lat/lon แม่นยำกว่า city name)
#define OWM_LAT       "13.909"
#define OWM_LON       "100.424"
#define OWM_CITY_NAME "Nonthaburi"

// อัปเดตทุกกี่วินาที (Free plan limit: 60 calls/min, แนะนำ 300s+)
#define WEATHER_UPDATE_SEC  5000

// ===== HiveMQ (Free public broker) =====
#define MQTT_HOST     "broker.hivemq.com"
#define MQTT_PORT     1883
// Client ID ควร unique — ใส่ mac address ท้าย 6 ตัวก็ได้
#define MQTT_CLIENT_ID  "esp32-level3"

// Base topic — เปลี่ยนให้ unique เพื่อไม่ชนกับคนอื่นบน public broker
#define MQTT_BASE       "ESP32-setup"

// Publish interval (ms)
#define MQTT_TELEMETRY_INTERVAL  5000

#endif // CONFIG_H
