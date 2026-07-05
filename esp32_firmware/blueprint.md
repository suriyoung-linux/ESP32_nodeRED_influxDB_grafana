# ESP32 DevKit Template Project - Blueprint

เอกสารนี้อธิบายโครงสร้าง firmware เวอร์ชันปัจจุบันหลัง optimization ล่าสุด โดยเน้นภาพรวมระบบ, class responsibility, runtime flow, pin map, web/MQTT payload และข้อควรระวังสำหรับการดูแลต่อยอดโปรเจกต์

## สถานะล่าสุด

| รายการ | ค่า |
|---|---|
| Board | `esp32doit-devkit-v1` |
| Framework | Arduino |
| Partition | `huge_app.csv` |
| Upload / Monitor port | `/dev/ttyUSB0` |
| Upload / Monitor speed | `115200` |
| Build flag | `CONFIG_ASYNC_TCP_RUNNING_CORE=1` |
| Build ล่าสุด | ผ่าน `pio run` |
| Memory ล่าสุด | RAM `15.9%`, Flash `37.9%` ของ huge app partition |

> `huge_app.csv` เพิ่มพื้นที่โปรแกรมให้เหลือ headroom เยอะขึ้น เหมาะกับ firmware ที่มี dashboard, HTTPS weather, MQTT และ async web server อยู่ในตัว แต่ partition นี้ไม่ใช่แบบ dual OTA

## ภาพรวมระบบ

Firmware นี้รวมหลาย subsystem ไว้ใน ESP32 ตัวเดียว:

- OLED 128x64 แสดง sensor, weather, relay และ menu
- WiFi provisioning ด้วย WiFiManager ผ่าน captive portal `ESP32-Setup`
- Dashboard web app ผ่าน HTTP + WebSocket
- MQTT telemetry/control ผ่าน HiveMQ public broker
- Weather จาก OpenWeatherMap ผ่าน HTTPS
- Relay 3 ช่อง พร้อม state persistence ใน NVS
- DS18B20 temperature sensor พร้อม simulation fallback
- XY-MD03 temperature/humidity sensor ผ่าน Modbus RTU พร้อม simulation fallback
- State machine สำหรับควบคุมเมนูด้วยปุ่ม 3 ปุ่ม

## Runtime Flow

```text
setup()
  Serial.begin(115200)
  OLED begin
  DevStateStore.load()
  Switch / Relay begin
  Restore relay state จาก NVS
  DS18B20 begin
  Check hold SW1 เพื่อ reset WiFi
  WiFiManager begin
  Cache local IP เป็น char[] สำหรับ OLED
  XY-MD03 begin บน Serial0 baud 9600
  Weather update
  MQTT begin
  WebServer begin
  Draw MONITOR screen

loop()
  update switches
  run DevStateMachine
  handle events: relay changed, WiFi reset, redraw, OLED speed
  update DS18B20 / XY-MD03 / Weather ตาม interval
  OLED auto-cycle เฉพาะ MONITOR
  update MQTT connection + telemetry
  cleanup / broadcast WebSocket
```

## Hardware Pin Assignment

| อุปกรณ์ | GPIO | โหมด / หมายเหตุ |
|---|---:|---|
| SW1 | 34 | Active Low, MODE/ENTER, input only, ต้องใช้ external pull-up |
| SW2 | 35 | Active Low, DOWN, input only, ต้องใช้ external pull-up |
| SW3 | 32 | Active Low, UP |
| Relay 1 | 17 | Active Low |
| Relay 2 | 16 | Active Low |
| Relay 3 | 4 | Active Low |
| OLED SDA | 21 | I2C |
| OLED SCL | 22 | I2C |
| DS18B20 | 14 | OneWire |
| XY-MD03 | Serial0 | Modbus RTU, Slave ID 2, 9600 8N1 |

> GPIO34 และ GPIO35 ไม่มี internal pull-up/pull-down ต้องมี external pull-up จริง

> XY-MD03 ตอนนี้ใช้ `Serial0` ซึ่งเป็น UART เดียวกับ USB Serial/upload/log หลังจาก init sensor แล้ว code จะ reinit Serial0 เป็น 9600 ทำให้ serial monitor ที่ 115200 อ่าน log หลังจุดนั้นไม่ได้ชัดเจน ถ้าต้อง debug ระยะยาว แนะนำย้าย XY-MD03 ไป `Serial2` เช่น GPIO16/17 หรือคู่ pin ที่ว่าง

## Project Structure

```text
src/
  main.cpp                  Main orchestration, object wiring, setup/loop

include/
  config.h                  API keys, weather location, MQTT config, intervals
  dashboard.h               HTML/CSS/JS dashboard stored in PROGMEM
  DevOLED.h                 OLED cache + drawing + menu screens
  DevWifiManager.h          WiFiManager wrapper + OLED status
  DevWeather.h              OpenWeatherMap current/forecast/air pollution
  DevMQTT.h                 MQTT telemetry, relay control topics, reconnect
  DevWebServer.h            Async HTTP + WebSocket dashboard API
  DevStateStore.h           NVS relay state + boot counter
  DevStateMachine.h         Button-driven UI state machine
  DevRelay.h                Relay output and optional timer relay
  DevSwitch.h               Debounced push buttons
  DevDS18B20.h              DS18B20 sensor + simulation fallback
  DevXYMDSensor.h           XY-MD03 Modbus sensor + simulation fallback
  DevIsoInput.h             Reusable isolated input helper
  DevPZEM.h                 PZEM-016 helper, currently not wired in main.cpp

docker-compose.yml          MQTT, Node-RED, InfluxDB, Grafana stack
.env                        Host port mapping and service env
nodered/flows/
  esp32_level3_dashboard.json  Classic Node-RED Dashboard flow, mounted to /data/flows.json
```

## Class Responsibilities

| Class | หน้าที่หลัก | ถูกใช้ใน `main.cpp` |
|---|---|---|
| `DevOLED` | แสดง monitor pages, menu, relay control, settings, confirm, boot messages | Yes |
| `DevWifiManager` | เชื่อม WiFi หรือเปิด portal setup | Yes |
| `DevWeather` | ดึง weather, rain chance, PM2.5, AQI จาก OpenWeatherMap | Yes |
| `DevMQTT` | Publish telemetry, subscribe relay commands, publish relay retained state | Yes |
| `DevWebServer` | Serve dashboard, `/api/status`, WebSocket `/ws` | Yes |
| `DevStateStore` | เก็บ relay state และ boot count ใน NVS | Yes |
| `DevStateMachine` | แปลงปุ่ม SW1/SW2/SW3 เป็น state/event | Yes |
| `DevRelay` | คุม relay active-low | Yes |
| `DevSwitch` | debounce และ edge detect ปุ่ม | Yes |
| `DevDS18B20` | อ่าน DS18B20 ทุก 2s, fallback simulation | Yes |
| `DevXYMDSensor` | อ่าน XY-MD03 ทุก 5s, fallback simulation, scan slave ID | Yes |
| `DevIsoInput` | isolated input reusable class | Included, not actively instantiated |
| `DevPZEM` | PZEM-016 reusable class | Included, not actively instantiated |

## State Machine

```text
MONITOR
  SW1 press -> MENU

MENU
  SW3 -> cursor up
  SW2 -> cursor down
  SW1 -> enter selected item

RELAY_CTRL
  SW3 -> selected relay ON
  SW2 -> selected relay OFF
  SW1 -> next relay
  Hold SW1 2s -> MONITOR

SETTINGS
  SW3/SW2 -> move cursor
  SW1 on WiFi Reset -> CONFIRM
  SW1 on OLED Speed -> toggle 5s/2s page interval
  SW1 on Back -> MENU

CONFIRM
  SW3 -> YES
  SW2 -> NO
  SW1 -> execute/cancel
```

State machine ส่ง event กลับให้ `main.cpp`:

| Event | Handler |
|---|---|
| `RELAY_CHANGED` | save NVS, publish MQTT relay state, redraw OLED |
| `WIFI_RESET` | clear WiFi credentials แล้ว restart |
| `OLED_SPEED_TOGGLE` | redraw settings screen |
| `REDRAW` | redraw screen ตาม state ปัจจุบัน |

## Web Dashboard

Dashboard อยู่ใน `include/dashboard.h` เป็น HTML/CSS/JS ใน `PROGMEM`

Endpoints:

| Endpoint | รายละเอียด |
|---|---|
| `GET /` | Dashboard HTML |
| `GET /api/status` | JSON snapshot |
| `WS /ws` | realtime snapshot + relay command |

WebSocket command สำหรับ toggle relay:

```json
{"cmd":"relay","n":1}
```

JSON snapshot หลัก:

```json
{
  "relay": [true, false, true],
  "weather": {
    "valid": true,
    "temp": 27.2,
    "hum": 63,
    "rain": 0,
    "pm25": 0.6,
    "aqi": 1,
    "aqiLabel": "Good",
    "city": "Nonthaburi"
  },
  "wifi": {
    "ssid": "...",
    "ip": "192.168.1.x",
    "mac": "...",
    "rssi": -55
  },
  "ds18": {"temp": 31.25, "sim": false},
  "xymd": {"temp": 28.4, "hum": 68.2, "sim": false, "id": 2},
  "mqtt": {
    "host": "broker.hivemq.com",
    "port": 1883,
    "base": "ESP32-Level3",
    "connected": true
  },
  "sys": {"heap": 120000, "uptime": 1234}
}
```

## MQTT

Config อยู่ใน `include/config.h`

| ค่า | ปัจจุบัน |
|---|---|
| Host | `broker.hivemq.com` |
| Port | `1883` |
| Client ID | `esp32-level3` |
| Base topic | `ESP32-Level3` |
| Telemetry interval | `5000 ms` |

Node-RED / Docker:

| ค่า | ปัจจุบัน |
|---|---|
| Node-RED editor | `http://localhost:1881` |
| Node-RED dashboard | `http://localhost:1881/ui/` |
| Flow file | `nodered/flows/esp32_level3_dashboard.json` |
| Container flow path | `/data/flows.json` |
| Dashboard package | `node-red-dashboard@3.6.6` |

Topic map:

| Topic | Direction | Payload |
|---|---|---|
| `ESP32-Level3/telemetry` | publish | JSON telemetry |
| `ESP32-Level3/status` | publish/LWT | `online` / `offline` retained |
| `ESP32-Level3/relay/1/state` | publish | `ON` / `OFF` retained |
| `ESP32-Level3/relay/2/state` | publish | `ON` / `OFF` retained |
| `ESP32-Level3/relay/3/state` | publish | `ON` / `OFF` retained |
| `ESP32-Level3/relay/1/set` | subscribe | `ON` / `OFF` / `TOGGLE` |
| `ESP32-Level3/relay/2/set` | subscribe | `ON` / `OFF` / `TOGGLE` |
| `ESP32-Level3/relay/3/set` | subscribe | `ON` / `OFF` / `TOGGLE` |

Optimization ล่าสุดใน `DevMQTT`:

- cache topic string เป็น `char[]` ตอน `begin()`
- ใช้ `strcmp()` แทนสร้าง `String` topic ใน callback
- telemetry JSON ใช้ตัวเลขจริงแทน `serialized(String(float))`
- reserve output buffer ก่อน `serializeJson()`
- MQTT client id ต่อท้าย MAC 6 หลักอัตโนมัติ เพื่อลดปัญหา client id ซ้ำบน public broker
- publish telemetry แบบ retained และส่งทันทีหลัง MQTT connect สำเร็จ
- serialize telemetry ลง `char[]` โดยตรง แทนสร้าง `String` payload ทุกครั้ง
- parse MQTT command payload ด้วย fixed buffer แทนต่อ `String` ใน callback

Node-RED flow ใช้ topic ที่ต้องตรงกับ firmware:

```text
ESP32-Level3/telemetry
ESP32-Level3/relay/1/set
ESP32-Level3/relay/2/set
ESP32-Level3/relay/3/set
```

## Weather

OpenWeatherMap config:

| ค่า | ปัจจุบัน |
|---|---|
| `OWM_CITY_NAME` | `Nonthaburi` |
| `OWM_LAT` | `13.909` |
| `OWM_LON` | `100.424` |
| `WEATHER_UPDATE_SEC` | `5000` |

API ที่ใช้:

- Current Weather: temp, humidity, current weather id, cloud proxy
- Forecast: precipitation probability (`pop`)
- Air Pollution: PM2.5, AQI

Optimization ล่าสุดใน `DevWeather`:

- URL เป็น `static const char[]` แทนสร้าง `String`
- `fetchJson()` รับ `const char*`
- dashboard city แสดงจาก `OWM_CITY_NAME` ผ่าน payload ไม่ hardcode ใน HTML

> ชื่อ `WEATHER_UPDATE_SEC` เป็นวินาที ดังนั้นค่า `5000` หมายถึง 5000 วินาที ไม่ใช่ 5000 ms ถ้าต้องการอัปเดตทุก 5 นาทีให้ใช้ `300`

## OLED

OLED แบ่งเป็น 2 monitor pages:

| Page | รายละเอียด |
|---|---|
| Page A | DS18B20, XY-MD03, relay bar |
| Page B | Weather, rain bar, PM2.5/AQI, relay bar, IP |

Screen อื่น:

- Menu
- Relay control
- Settings
- Confirm WiFi reset
- Boot/status messages
- WiFi reset countdown

Optimization ล่าสุด:

- `main.cpp` cache IP เป็น `char ipText[20]`
- `DevOLED::showMain()` copy string แบบ null-terminated ปลอดภัยขึ้น
- OLED redraw เฉพาะเมื่อ state/data เปลี่ยน หรือ page interval ครบ

## Persistence

`DevStateStore` ใช้ ESP32 NVS ผ่าน `Preferences`

Namespace: `appstate`

| Key | ค่า |
|---|---|
| `r1` | relay 1 state |
| `r2` | relay 2 state |
| `r3` | relay 3 state |
| `boot_cnt` | boot counter |

Relay state จะเขียน NVS เฉพาะเมื่อค่าเปลี่ยนจริง ลด flash wear

## Optimization Summary

สิ่งที่ปรับแล้ว:

- ใช้ `huge_app.csv` เพื่อเพิ่ม flash headroom
- ลด dynamic `String` ใน path ที่เรียกบ่อย เช่น MQTT topic, dashboard topic, weather URL, OLED IP
- cache topic MQTT/Web dashboard ล่วงหน้า
- ใช้ fixed JSON buffer สำหรับ WebSocket/API snapshot เพื่อลด heap fragmentation
- ใช้ numeric JSON value แทน stringified float
- reserve JSON output `String` ก่อน serialize
- ป้องกัน string buffer ไม่ null-terminated ใน OLED cache
- dashboard city ไม่ hardcode แล้ว อ่านจาก `OWM_CITY_NAME`

ข้อที่ยังควรพิจารณาต่อ:

- ย้าย XY-MD03 ออกจาก `Serial0` ไป `Serial2` เพื่อให้ USB serial monitor ใช้งานต่อได้หลัง boot
- เปลี่ยน `WEATHER_UPDATE_SEC` เป็นค่าที่ตั้งใจจริง เช่น `300` สำหรับ 5 นาที
- ถ้าต้องการ OTA ให้เปลี่ยน partition จาก `huge_app.csv` เป็น partition แบบ OTA และลดขนาด dashboard/libraries ตามจำเป็น
- ไม่ควร commit API key จริงลง repo public

## Build / Upload

Build:

```bash
/home/ubuntu/.platformio/penv/bin/pio run
```

Upload:

```bash
/home/ubuntu/.platformio/penv/bin/pio run -t upload
```

ถ้า upload ไม่เจอบอร์ด ให้เช็ก port:

```bash
/home/ubuntu/.platformio/penv/bin/pio device list
```

แล้วแก้ `upload_port` และ `monitor_port` ใน `platformio.ini` ให้ตรงกับ `/dev/ttyUSB0` หรือ `/dev/ttyACM0`
