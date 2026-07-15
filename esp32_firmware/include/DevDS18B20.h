#ifndef DEV_DS18B20_H
#define DEV_DS18B20_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class DevDS18B20 {
private:
  OneWire           wire;
  DallasTemperature sensors;
  uint8_t           pin;
  bool              simMode;
  float             lastTemp;
  unsigned long     lastReadAt;
  unsigned long     simStart;
  unsigned long     conversionStartedAt;
  bool              conversionPending;

  static const unsigned long READ_INTERVAL = 2000; // ms
  static const unsigned long CONVERSION_TIME = 750; // 12-bit conversion, ms

  float _simTemp() {
    // จำลอง 28–34°C แกว่งตาม sine wave
    float t = (millis() - simStart) / 1000.0f;
    return 31.0f + 3.0f * sinf(t * 0.05f);
  }

public:
  DevDS18B20(uint8_t gpioPin)
    : wire(gpioPin), sensors(&wire),
      pin(gpioPin), simMode(false),
      lastTemp(0), lastReadAt(0), simStart(0),
      conversionStartedAt(0), conversionPending(false) {}

  // คืน true = sensor จริง, false = simulation
  bool begin() {
    sensors.begin();
    uint8_t count = sensors.getDeviceCount();
    if (count == 0) {
      simMode   = true;
      simStart  = millis();
      lastTemp  = _simTemp();
      Serial.printf("[DS18B20] No sensor on GPIO%d → Simulation mode\n", pin);
      return false;
    }
    sensors.setResolution(12); // 12-bit = 0.0625°C
    sensors.setWaitForConversion(false);
    simMode = false;
    sensors.requestTemperatures();
    conversionStartedAt = millis();
    conversionPending = true;
    Serial.printf("[DS18B20] Found %d sensor(s) on GPIO%d\n", count, pin);
    return true;
  }

  // เรียกใน loop() — request/read แบบ non-blocking ทุก READ_INTERVAL ms
  // คืน true ถ้ามีค่าใหม่
  bool update() {
    unsigned long now = millis();

    if (simMode) {
      if (now - lastReadAt < READ_INTERVAL) return false;
      lastReadAt = now;
      lastTemp = _simTemp();
      return true;
    }

    if (!conversionPending) {
      if (now - lastReadAt < READ_INTERVAL) return false;
      sensors.requestTemperatures();
      conversionStartedAt = now;
      conversionPending = true;
      return false;
    }

    if (now - conversionStartedAt < CONVERSION_TIME) return false;

    float t = sensors.getTempCByIndex(0);
    conversionPending = false;
    lastReadAt = now;
    if (t == DEVICE_DISCONNECTED_C) {
      // sensor หลุด → fallback simulation
      simMode  = true;
      simStart = millis();
      lastTemp = _simTemp();
      Serial.println("[DS18B20] Disconnected → Simulation mode");
    } else {
      lastTemp = t;
    }
    return true;
  }

  float    getTemp()   const { return lastTemp; }
  bool     isSimMode() const { return simMode;  }
};

#endif // DEV_DS18B20_H
