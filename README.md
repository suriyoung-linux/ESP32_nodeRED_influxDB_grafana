# ESP32_nodeRED_influxDB_grafana_Level3

Firmware สำหรับ ESP32 DevKit V1 ที่รวม OLED UI, relay control, sensor monitoring, WiFi setup, Web Dashboard, MQTT, Node-RED Dashboard, InfluxDB logging และ OpenWeatherMap ไว้ในโปรเจกต์เดียว

โปรเจกต์นี้ถูก optimize ล่าสุดเพื่อลด dynamic `String` ใน path ที่เรียกบ่อย, cache MQTT/Web topics, ใช้ `huge_app.csv` เพื่อเพิ่ม flash headroom, ปรับ MQTT reconnect state ให้ชัดขึ้น และแก้ Node-RED flow ให้ seed เข้า volume แทน bind mount ไฟล์ตรง

รายละเอียด architecture เชิงลึกอยู่ที่ [FIRMWARE_BLUEPRINT.md](FIRMWARE_BLUEPRINT.md)

ไดอะแกรมเส้นทางข้อมูลและคำสั่งควบคุมอยู่ที่ [SYSTEM_FLOW_DIAGRAM.md](SYSTEM_FLOW_DIAGRAM.md)

คู่มือใช้งานแบบ step-by-step อยู่ที่ [USER_GUIDE.md](USER_GUIDE.md)

## Documentation

| เอกสาร | เนื้อหา |
|---|---|
| [README.md](README.md) | ภาพรวม firmware, hardware, configuration และ troubleshooting |
| [USER_GUIDE.md](USER_GUIDE.md) | วิธีติดตั้งและใช้งานระบบแบบ step-by-step |
| [FIRMWARE_BLUEPRINT.md](FIRMWARE_BLUEPRINT.md) | Architecture, runtime flow และ class responsibilities |
| [SYSTEM_FLOW_DIAGRAM.md](SYSTEM_FLOW_DIAGRAM.md) | Mermaid diagrams ของ telemetry, relay control, Docker network และ data flow |
| [DOCKER_GUIDE.md](DOCKER_GUIDE.md) | Docker Compose, service management และ performance tuning |
| [GRAFANA_DASHBOARD.md](GRAFANA_DASHBOARD.md) | ตั้งค่า Grafana Data Source และนำเข้า dashboard |
| [GRAFANA_QUERY_GUIDE.md](GRAFANA_QUERY_GUIDE.md) | ตัวอย่าง Flux query สำหรับ sensor และสถานะระบบ |
| [GITHUB_UPLOAD_GUIDE.md](GITHUB_UPLOAD_GUIDE.md) | เตรียม repository, commit และ push ขึ้น GitHub |

## Project Naming

ชื่อหลักของโปรเจกต์และหัวข้อเอกสารคือ `ESP32_nodeRED_influxDB_grafana_Level3`

ชื่อแบบอ่านง่ายคือ **ESP32 + Node-RED + InfluxDB + Grafana — Level3** โดยคำว่า `Level3`
เป็นส่วนหนึ่งของชื่อโครงการเดิมและอธิบายที่มาของ runtime identifiers ด้านล่าง ส่วนค่าต่อไปนี้เป็น runtime identifiers
ที่ระบบใช้งานอยู่และต้องตรงกันระหว่าง firmware, Node-RED, InfluxDB และ Grafana:

| รายการ | ค่ามาตรฐาน |
|---|---|
| MQTT client prefix | `esp32-level3` |
| MQTT base topic | `ESP32-Level3` |
| Docker Compose project | `esp32-project-level3` |
| Node-RED dashboard tab | `ESP32 Level3` |
| Node-RED flow seed | `nodered/flows/esp32_level3_dashboard.json` |
| InfluxDB organization | `mylab` |
| InfluxDB bucket | `esp32_db` |
| InfluxDB measurement | `ESP32level3_telemetry` |
| Grafana dashboard | `grafana/dashboards/grafana-dashboard-esp32.json` |

การเปลี่ยนชื่อ runtime identifiers ต้องแก้ทุก service พร้อมกันและวางแผนย้ายข้อมูลเดิม จึงไม่ควรเปลี่ยน
เพียงเพื่อให้เหมือนชื่อ repository

โฟลเดอร์ workspace และ GitHub remote ปัจจุบันยังใช้ชื่อ `ESP32-Project-Level3` การเปลี่ยนสองส่วนนี้
ทำแยกภายหลังได้โดยไม่กระทบชื่อหัวข้อเอกสาร แต่หากเปลี่ยนชื่อโฟลเดอร์ต้อง regenerate
`compile_commands.json` และเปิด workspace ใน VS Code ใหม่

## Features

- OLED SSD1306 128x64 พร้อม monitor pages และเมนูควบคุม
- ปุ่ม 3 ปุ่มผ่าน state machine: menu, relay control, settings, confirm
- Relay 3 ช่อง Active Low พร้อมบันทึกสถานะลง NVS
- DS18B20 temperature sensor พร้อม simulation fallback
- XY-MD03 temperature/humidity sensor ผ่าน Modbus RTU พร้อม simulation fallback
- WiFiManager captive portal ชื่อ `ESP32-Setup`
- Web Dashboard ผ่าน HTTP + WebSocket
- MQTT telemetry/control ผ่าน HiveMQ public broker
- Node-RED Dashboard + InfluxDB telemetry flow ผ่าน Docker Compose
- Weather, rain chance, PM2.5 และ AQI จาก OpenWeatherMap
- Build partition แบบ `huge_app.csv` สำหรับ firmware ขนาดใหญ่

## Current Build Profile

| รายการ | ค่า |
|---|---|
| Board | `esp32doit-devkit-v1` |
| Framework | Arduino |
| Partition | `huge_app.csv` |
| Upload port | `/dev/ttyUSB0` |
| Monitor speed | `115200` |
| Build flag | `CONFIG_ASYNC_TCP_RUNNING_CORE=1` |
| Build ล่าสุด | `pio run` ผ่าน |
| Memory ล่าสุด | RAM `16.2%`, Flash `37.9%` |

> `huge_app.csv` ช่วยให้ firmware มีพื้นที่ app มากขึ้น แต่ไม่ใช่ partition แบบ dual OTA

## Hardware

| อุปกรณ์ | รายละเอียด |
|---|---|
| MCU | ESP32 DevKit V1 |
| Display | OLED SSD1306 128x64 I2C address `0x3C` |
| Relay | 3-channel Active Low |
| Buttons | SW1/SW2/SW3 Active Low |
| Temperature | DS18B20 on OneWire |
| Temp/Humidity | XY-MD03 Modbus RTU RS485 |
| RS485 | MAX13487 Auto Direction |

## Pin Assignment

| อุปกรณ์ | GPIO | หมายเหตุ |
|---|---:|---|
| SW1 | 34 | MODE/ENTER, Active Low, input only, external pull-up |
| SW2 | 35 | DOWN, Active Low, input only, external pull-up |
| SW3 | 32 | UP, Active Low |
| Relay 1 | 17 | Active Low |
| Relay 2 | 16 | Active Low |
| Relay 3 | 4 | Active Low |
| DS18B20 | 14 | OneWire, pull-up 4.7k recommended |
| OLED SDA | 21 | I2C |
| OLED SCL | 22 | I2C |
| XY-MD03 | Serial0 | Modbus RTU, Slave ID 2, 9600 8N1 |

> GPIO34 และ GPIO35 ไม่มี internal pull-up/pull-down ต้องใช้ external pull-up จริง

> XY-MD03 ใช้ `Serial0` ร่วมกับ USB serial/upload/log หลัง init sensor แล้ว firmware จะ reinit Serial0 เป็น 9600 สำหรับ Modbus ทำให้ serial monitor ที่ 115200 อ่าน log หลังจุดนั้นไม่ได้ชัดเจน ถ้าต้อง debug ยาว ๆ แนะนำย้าย XY-MD03 ไป `Serial2`

## Project Structure

```text
esp32_firmware/src/
  main.cpp                  Main orchestration, setup/loop, event dispatch

esp32_firmware/include/
  config.h                  API key, weather location, MQTT config
  dashboard.h               Web Dashboard HTML/CSS/JS in PROGMEM
  DevOLED.h                 OLED screens and cached display data
  DevWifiManager.h          WiFiManager wrapper
  DevWeather.h              OpenWeatherMap client
  DevMQTT.h                 MQTT telemetry/control
  DevWebServer.h            AsyncWebServer + WebSocket API
  DevStateStore.h           NVS relay state and boot counter
  DevStateMachine.h         Button UI state machine
  DevRelay.h                Relay output helpers
  DevSwitch.h               Debounced button helper
  DevDS18B20.h              DS18B20 + simulation fallback
  DevXYMDSensor.h           XY-MD03 Modbus + simulation fallback
  DevIsoInput.h             Reusable isolated input helper
  DevPZEM.h                 Optional PZEM-016 helper, not wired in main.cpp

esp32_firmware/platformio.ini  PlatformIO build/upload config

FIRMWARE_BLUEPRINT.md       Detailed architecture notes
docker-compose.yml          Docker services: MQTT, Node-RED, InfluxDB, Grafana
.env.example                Example local Docker settings
.env                        Local Docker overrides and secrets, not committed
nodered/flows/
  esp32_level3_dashboard.json  Node-RED Dashboard + InfluxDB flow seed
nodered/entrypoint.sh       Seed flow into /data/flows.json when volume is empty
grafana/dashboards/
  grafana-dashboard-esp32.json  Grafana dashboard สำหรับ import/export
```

## Configuration

ค่า default อยู่ใน [esp32_firmware/include/config.h](esp32_firmware/include/config.h) และค่าลับ/ค่าประจำเครื่องให้ใส่ใน `esp32_firmware/include/config_private.h`

```bash
cp esp32_firmware/include/config_private.h.example esp32_firmware/include/config_private.h
```

จากนั้นแก้ `config_private.h`:

```cpp
#define OWM_API_KEY   "YOUR_OPENWEATHERMAP_KEY"
#define OWM_LAT       "13.909"
#define OWM_LON       "100.424"
#define OWM_CITY_NAME "Nonthaburi"
#define WEATHER_UPDATE_SEC  300

#define MQTT_HOST     "broker.hivemq.com"
#define MQTT_PORT     1883
#define MQTT_CLIENT_ID  "esp32-level3"
#define MQTT_BASE       "ESP32-Level3"
#define MQTT_TELEMETRY_INTERVAL  5000
```

หมายเหตุสำคัญ:

- `WEATHER_UPDATE_SEC` เป็นวินาที ถ้าต้องการ 5 นาทีให้ตั้ง `300`
- `MQTT_CLIENT_ID` และ `MQTT_BASE` ควรตั้งให้ unique ถ้าใช้ broker public
- ไม่ควร commit API key จริงขึ้น public repository; `config_private.h` ถูก ignore แล้ว
- ถ้าเปลี่ยน `OWM_CITY_NAME` หน้า web dashboard จะเปลี่ยนตาม payload โดยไม่ต้องแก้ `dashboard.h`

## Build / Upload

Build:

```bash
/home/ubuntu/.platformio/penv/bin/pio run
```

Upload:

```bash
/home/ubuntu/.platformio/penv/bin/pio run -t upload
```

Serial monitor:

```bash
/home/ubuntu/.platformio/penv/bin/pio device monitor
```

ถ้า upload ไม่เจอบอร์ด ให้ดู port:

```bash
/home/ubuntu/.platformio/penv/bin/pio device list
```

แล้วแก้ `upload_port` / `monitor_port` ใน [esp32_firmware/platformio.ini](esp32_firmware/platformio.ini) เป็น `/dev/ttyUSB0` หรือ `/dev/ttyACM0` ตามที่เครื่องเห็นจริง

## First Boot

1. Upload firmware
2. ถ้ายังไม่มี WiFi credentials บอร์ดจะเปิด portal `ESP32-Setup`
3. เชื่อม WiFi จากมือถือ/คอม แล้วตั้งค่า SSID/password
   - หมายเหตุ: ESP32 จะต่อ WiFi และจัดการการเข้ารหัสเองตาม AP ที่เลือกใน captive portal
   - หากเครือข่ายเปลี่ยนหรือ credential ผิด ให้กด SW1 ค้างตอน boot เพื่อ clear credentials แล้วเปิด portal ใหม่
4. OLED จะแสดง IP address
5. เปิด browser ไปที่ `http://<IP>`
6. Dashboard จะ update ผ่าน WebSocket ทุก 2 วินาที

Reset WiFi credentials:

- กด SW1 ค้างตอน boot ประมาณ 5 วินาที
- หรือเข้าเมนู OLED: `Settings -> WiFi Reset -> Confirm`

## Button UI

| ปุ่ม | GPIO | หน้าที่ |
|---|---:|---|
| SW1 | 34 | MODE/ENTER, เข้าเมนู, ยืนยัน, เลือก relay ถัดไป |
| SW2 | 35 | DOWN, relay OFF ในหน้า relay control |
| SW3 | 32 | UP, relay ON ในหน้า relay control |
| Hold SW1 | - | กลับ MONITOR จาก state อื่น |

State หลัก:

```text
MONITOR -> MENU -> RELAY_CTRL
              \-> SETTINGS -> CONFIRM
```

OLED monitor มี 2 หน้า:

- Sensors: DS18B20, XY-MD03, relay status
- Weather: OpenWeatherMap, rain, PM2.5, AQI, IP

OLED speed toggle:

- Settings -> OLED Speed
- สลับระหว่าง 5 วินาที และ 2 วินาที

## Web Dashboard

เปิด:

```text
http://<ESP32-IP>/
```

Endpoints:

| Endpoint | รายละเอียด |
|---|---|
| `GET /` | Dashboard HTML |
| `GET /api/status` | JSON snapshot |
| `WS /ws` | Realtime update + relay command |

WebSocket relay command:

```json
{"cmd":"relay","n":1}
```

Snapshot มีข้อมูลหลัก:

- `relay`: relay 1-3
- `weather`: temp, humidity, rain, PM2.5, AQI, city
- `wifi`: SSID, IP, MAC, RSSI
- `ds18`: DS18B20 temp + sim flag
- `xymd`: XY-MD03 temp/humidity + sim flag + slave ID
- `mqtt`: broker info + connection status
- `sys`: heap + uptime

## MQTT

Base topic ปัจจุบัน:

```text
ESP32-Level3
```

Node-RED Dashboard flow ใช้ base topic เดียวกันนี้ และถูก seed จาก:

```text
nodered/flows/esp32_level3_dashboard.json
```

เข้า image แล้ว copy ไปเป็นไฟล์ runtime เมื่อ volume ยังว่าง:

```text
/data/flows.json
```

อย่า bind mount ไฟล์ seed ตรงไปที่ `/data/flows.json` เพราะ Node-RED ใช้วิธีเขียนไฟล์ temp แล้ว rename ทับไฟล์จริง ซึ่งอาจเกิด `EBUSY` เมื่อ target เป็น bind-mounted file

Publish:

| Topic | Payload |
|---|---|
| `ESP32-Level3/telemetry` | JSON telemetry |
| `ESP32-Level3/status` | `online` / `offline` retained |
| `ESP32-Level3/relay/1/state` | `ON` / `OFF` retained |
| `ESP32-Level3/relay/2/state` | `ON` / `OFF` retained |
| `ESP32-Level3/relay/3/state` | `ON` / `OFF` retained |

Subscribe:

| Topic | Payload |
|---|---|
| `ESP32-Level3/relay/1/set` | `ON`, `OFF`, `TOGGLE` |
| `ESP32-Level3/relay/2/set` | `ON`, `OFF`, `TOGGLE` |
| `ESP32-Level3/relay/3/set` | `ON`, `OFF`, `TOGGLE` |

ตัวอย่าง:

```bash
mosquitto_pub -h broker.hivemq.com -t ESP32-Level3/relay/1/set -m ON
mosquitto_pub -h broker.hivemq.com -t ESP32-Level3/relay/2/set -m TOGGLE
mosquitto_sub -h broker.hivemq.com -t "ESP32-Level3/#"
```

## Node-RED Dashboard

Docker Compose expose Node-RED ด้วยพอร์ตมาตรฐาน `1880`

```text
Node-RED editor: http://localhost:1880
Node-RED dashboard: http://localhost:1880/ui/
```

Flow หลักอยู่ที่:

```text
nodered/flows/esp32_level3_dashboard.json
```

Flow นี้มี 2 tab:

```text
ESP32 Level3          Dashboard + relay control
ESP32 to InfluxDB    MQTT telemetry -> InfluxDB
```

Flow นี้ใช้ `node-red-dashboard@3.6.6` node แบบ classic และ `node-red-contrib-influxdb@0.7.0`:

```text
ui_gauge
ui_text
ui_switch
influxdb out
```

InfluxDB config ใน Node-RED เมื่อรันผ่าน Docker Compose:

```text
URL: http://influxdb:8086
Org: mylab
Bucket: esp32_db
Measurement: ESP32level3_telemetry
```

ถ้าแก้ flow ใน Node-RED editor ให้กด Deploy ได้ตามปกติและ runtime จะบันทึกลง volume `nodered_data` ถ้าแก้ flow seed ใน VS Code ให้ rebuild image สำหรับเครื่องใหม่:

```bash
docker compose up -d --build nodered
```

ตรวจสถานะ:

```bash
docker compose logs nodered
docker compose ps nodered
```

Telemetry example:

```json
{
  "relay": {"1": "ON", "2": "OFF", "3": "OFF"},
  "ds18": {"temp": 31.25, "sim": false},
  "xymd": {"temp": 28.4, "hum": 68.2, "sim": false},
  "weather": {
    "valid": true,
    "temp": 27.2,
    "hum": 63,
    "rain": 0,
    "pm25": 0.6,
    "aqi": 1,
    "aqiLabel": "Good"
  },
  "sys": {
    "ip": "192.168.1.105",
    "rssi": -55,
    "heap": 120000,
    "uptime": 3600
  }
}
```

## Persistence

Relay state ถูกบันทึกลง NVS ทุกครั้งที่เปลี่ยนจากปุ่ม, Web หรือ MQTT

NVS namespace:

```text
appstate
```

Keys:

| Key | Type | รายละเอียด |
|---|---|---|
| `r1` | bool | Relay 1 state |
| `r2` | bool | Relay 2 state |
| `r3` | bool | Relay 3 state |
| `boot_cnt` | uint32 | Boot counter |

ระบบเขียน NVS เฉพาะเมื่อค่า relay เปลี่ยนจริง เพื่อลด flash wear

## Optimization Notes

สิ่งที่ปรับแล้วในรอบล่าสุด:

- เพิ่ม `board_build.partitions = huge_app.csv`
- เพิ่ม `monitor_speed = 115200`
- cache local IP เป็น `char[]` ใน `main.cpp`
- cache MQTT topics เป็น `char[]` ใน `DevMQTT`
- ย้าย MQTT reconnect timer เป็น member state และเพิ่ม guard relay index
- cache dashboard MQTT topics ใน `DevWebServer`
- ลด heap allocation ใน MQTT telemetry, MQTT command callback และ WebSocket JSON broadcast
- เก็บ MQTT telemetry buffer เป็น member เพื่อลด peak stack usage
- ใช้ fixed JSON buffer ใน `DevWebServer` เพื่อลด heap fragmentation ระหว่าง broadcast ทุก 2 วินาที
- อ่าน DS18B20 แบบ asynchronous ไม่ block loop ระหว่างรอ conversion 12-bit
- จำกัด WebSocket client cleanup ทุก 5 วินาทีแทนการ scan ทุก loop
- ใส่ `delay(1)` เพื่อคืนเวลาให้ WiFi/AsyncTCP task และลด CPU busy-spin
- เขียน NVS เฉพาะ key ของ relay ที่เปลี่ยนจริง
- ลดการสร้าง `String` ใน Weather URL และ MQTT/Web payload
- JSON float ส่งเป็น number แทน stringified number
- reserve JSON output buffer ก่อน serialize
- ป้องกัน `strncpy` ไม่ใส่ null terminator ใน OLED cache
- Dashboard weather city อ่านจาก `OWM_CITY_NAME`
- ปิด Node-RED debug nodes ใน seed และใช้ measurement เดียว `ESP32level3_telemetry`
- Grafana Time series ใช้ `aggregateWindow()` และ Stat ใช้ `last()`

## Troubleshooting

### Upload ไม่ผ่าน

- เช็ก port ด้วย `pio device list`
- แก้ `upload_port` เป็น `/dev/ttyUSB0` หรือ `/dev/ttyACM0`
- บางบอร์ดต้องกดปุ่ม BOOT ตอนเริ่ม upload
- บน Linux อาจต้องติดตั้ง PlatformIO udev rules

### Dashboard ยังไม่เปลี่ยนหลัง upload

- Hard refresh browser
- ปิดเปิด tab ใหม่
- ตรวจว่า upload เข้า board ตัวที่ใช้งานจริง
- เปิด `/api/status` เพื่อดู payload ล่าสุด

### Weather city หรือค่า weather ไม่ตรง

- ตรวจ `OWM_CITY_NAME`, `OWM_LAT`, `OWM_LON` ใน `config.h`
- ชื่อเมืองเป็น label เท่านั้น ค่า weather ใช้ lat/lon
- ตรวจว่า `WEATHER_UPDATE_SEC` เป็นวินาที

### WiFi เชื่อมต่อไม่ได้

- ESP32 รองรับ WiFi 2.4GHz เท่านั้น
- Reset credentials ด้วย SW1 ตอน boot หรือเมนู Settings
- ตรวจ portal `ESP32-Setup`

### MQTT ไม่เชื่อมต่อ

- ต้องมี internet
- เปลี่ยน `MQTT_CLIENT_ID` ให้ unique
- ถ้าใช้ public broker ให้เปลี่ยน `MQTT_BASE` กัน topic ชนคนอื่น

### Node-RED Dashboard ไม่ขึ้น

- เปิด URL ให้ถูก: `http://localhost:1880/ui/`
- ตรวจว่า `node-red-dashboard@3.6.6` ติดตั้งแล้ว
- ตรวจว่า flow runtime ใน `/data/flows.json` มี tabs `ESP32 Level3` และ `ESP32 to InfluxDB`
- ตรวจ log ต้องเห็น `Dashboard version 3.6.6 started at /ui`
- ตรวจ MQTT topic ต้องเป็น `ESP32-Level3/telemetry`

### Node-RED Deploy แล้ว save flow ไม่ได้

- ถ้า log มี `EBUSY ... rename '/data/flows.json.$$$' -> '/data/flows.json'` ให้ตรวจว่าไม่ได้ bind mount ไฟล์ตรงไปที่ `/data/flows.json`
- Compose ปัจจุบันควรใช้ named volume `nodered_data:/data` และ seed flow ผ่าน `nodered/entrypoint.sh`

### InfluxDB ไม่ได้ข้อมูลจาก Node-RED

- Influx URL ใน Node-RED container ต้องเป็น `http://influxdb:8086` ไม่ใช่ `localhost` หรือ `127.0.0.1`
- ตรวจ topic ใน Influx tab ต้องเป็น `ESP32-Level3/telemetry`
- ตรวจ `org=mylab`, `bucket=esp32_db`, `measurement=ESP32level3_telemetry`
- ถ้า log InfluxDB ขึ้น `Unauthorized` ให้ตรวจ token ใน Node-RED config node เพราะ token อยู่ใน `flows_cred.json` แบบ encrypted

### XY-MD03 เป็น SIM ตลอด

- ตรวจ RS485 A/B
- ตรวจ Slave ID เป็น `2`
- ตรวจ baud `9600`
- ตรวจว่าเลือกโหมด RS485 ถูก
- ถ้าต้องดู log หลัง init ให้พิจารณาย้าย sensor ไป `Serial2`

### DS18B20 เป็น SIM

- ตรวจสาย data ที่ GPIO14
- ตรวจ pull-up 4.7k ไป 3.3V
- ตรวจ sensor และ ground ร่วม

### ปุ่มไม่ตอบสนอง

- GPIO34/35 ต้องมี external pull-up
- ตรวจ wiring Active Low
- ตรวจว่า SW1/SW2/SW3 ต่อกับ GPIO ตาม pin map

## Libraries

จัดการผ่าน `esp32_firmware/platformio.ini`

| Library | หน้าที่ |
|---|---|
| `ModbusMaster` | Modbus RTU |
| `Adafruit SSD1306` | OLED driver |
| `Adafruit GFX Library` | Graphics primitives |
| `WiFiManager` | Captive portal |
| `ArduinoJson` | JSON |
| `ESPAsyncWebServer` | HTTP + WebSocket |
| `OneWire` | DS18B20 protocol |
| `DallasTemperature` | DS18B20 driver |
| `PubSubClient` | MQTT client |

## Maintenance

- README นี้เป็นคู่มือใช้งานและเริ่มต้น
- [FIRMWARE_BLUEPRINT.md](FIRMWARE_BLUEPRINT.md) เป็นเอกสาร architecture และ flow เชิงลึก
- ถ้าเพิ่ม feature ใหม่ ให้แก้ทั้ง README และ blueprint ให้ตรงกับ code

สร้างด้วย PlatformIO, Arduino Framework และ ESP32 DevKit V1
