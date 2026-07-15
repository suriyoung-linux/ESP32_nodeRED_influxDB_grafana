#ifndef DEV_WIFI_MANAGER_H
#define DEV_WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "DevOLED.h"

class DevWifiManager {
private:
  DevOLED* oled;
  WiFiManager wm;
  String apName;

public:
  DevWifiManager(DevOLED* oledPtr, const char* portalName = "ESP32-Setup")
    : oled(oledPtr), apName(portalName) {}

  // เรียกใน setup() — บล็อกจนกว่าจะเชื่อมต่อสำเร็จ
  // resetConfig = true จะลบ credentials ก่อนแล้วเปิด portal
  bool begin(bool resetConfig = false,
             unsigned long configPortalTimeoutSec = 180,
             unsigned long connectTimeoutSec = 30) {
    if (resetConfig) {
      WiFi.disconnect(true, true);
      wm.resetSettings();
    }

    wm.setCleanConnect(true);
    wm.setConnectTimeout(connectTimeoutSec);
    wm.setConfigPortalTimeout(configPortalTimeoutSec);
    wm.setConfigPortalTimeoutCallback([this]() {
      if (oled) oled->showMessage("WiFi Setup", "Timeout", "Retry boot");
    });

    _showConnecting();

    wm.setAPCallback([this](WiFiManager* wm) {
      _showPortal();
    });

    bool ok = wm.autoConnect(apName.c_str());

    if (ok) {
      _showConnected();
      return true;
    } else {
      _showFailed();
      delay(3000);
      ESP.restart();
      return false;
    }
  }

  IPAddress localIP() const {
    return WiFi.localIP();
  }

private:
  void _showConnecting() {
    if (!oled) return;
    oled->showMessage("WiFi", "Connecting...", "");
  }

  void _showPortal() {
    if (!oled) return;
    String line2 = "AP: " + apName;
    oled->showMessage("WiFi Setup", line2.c_str(), "Connect & config");
  }

  void _showConnected() {
    if (!oled) return;
    String ip = WiFi.localIP().toString();
    oled->showMessage("WiFi OK", ip.c_str(), "");
  }

  void _showFailed() {
    if (!oled) return;
    oled->showMessage("WiFi FAILED", "Restarting...", "");
  }
};

#endif // DEV_WIFI_MANAGER_H
