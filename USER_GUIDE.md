# ESP32 Level3 User Guide

คู่มือนี้ใช้สำหรับเริ่มระบบ, เปิด dashboard, ควบคุม relay, ตรวจ MQTT, และตั้งค่า Node-RED/InfluxDB ของโปรเจกต์ ESP32 Level3

## 1. ภาพรวมระบบ

ระบบนี้มี 2 ส่วนหลัก:

| ส่วน | หน้าที่ |
|---|---|
| ESP32 firmware | อ่าน sensor, ควบคุม relay, แสดง OLED, เปิด web dashboard, ส่ง MQTT telemetry |
| Docker stack | MQTT service, Node-RED dashboard, InfluxDB, Grafana |

URL หลักเมื่อ Docker stack ทำงาน:

| Service | URL |
|---|---|
| Node-RED editor | `http://localhost:1880` |
| Node-RED dashboard | `http://localhost:1880/ui/` |
| InfluxDB | `http://localhost:8086` |
| Grafana | `http://localhost:3000` |

ค่า MQTT ที่ firmware และ Node-RED ต้องตรงกัน:

```text
Host: broker.hivemq.com
Port: 1883
Base topic: ESP32-Level3
Telemetry: ESP32-Level3/telemetry
Relay command: ESP32-Level3/relay/{1|2|3}/set
```

## 2. เตรียมค่า Local

สร้างไฟล์ Docker env:

```bash
cp .env.example .env
```

สร้างไฟล์ firmware private config:

```bash
cp esp32_firmware/include/config_private.h.example esp32_firmware/include/config_private.h
```

แก้ `esp32_firmware/include/config_private.h` อย่างน้อยให้มี OpenWeatherMap API key:

```cpp
#define OWM_API_KEY "YOUR_OPENWEATHERMAP_KEY"
```

ถ้าใช้ public MQTT broker ร่วมกับคนอื่น ควรเปลี่ยน `MQTT_BASE` ให้ unique เพื่อลดโอกาส topic ชนกัน

## 3. เริ่ม Docker Stack

เริ่ม service ทั้งหมด:

```bash
docker compose up -d
```

ตรวจสถานะ:

```bash
docker compose ps
```

สถานะที่ควรเห็น:

```text
mqtt      Up
nodered   Up (healthy)
influxdb  Up
grafana   Up
```

ถ้าแก้ `nodered/Dockerfile`, `nodered/entrypoint.sh` หรือ flow seed ให้ rebuild Node-RED:

```bash
docker compose up -d --build nodered
```

## 4. ตั้งค่า InfluxDB ครั้งแรก

เปิด:

```text
http://localhost:8086
```

สร้าง organization และ bucket ให้ตรงกับ Node-RED flow:

```text
Org: mylab
Bucket: esp32_db
```

สร้าง API token ที่เขียน bucket นี้ได้ แล้วนำ token ไปใส่ใน Node-RED config node ชื่อ `InfluxDB Server`

ค่า InfluxDB ใน Node-RED ต้องเป็น:

```text
URL: http://influxdb:8086
Org: mylab
Bucket: esp32_db
Measurement: data_telemetry
```

หมายเหตุ: จากใน container Node-RED ต้องใช้ service name `influxdb` ไม่ใช่ `localhost`

## 5. ใช้งาน Node-RED

เปิด editor:

```text
http://localhost:1880
```

Flow มี 2 tab:

| Tab | หน้าที่ |
|---|---|
| `ESP32 Level3` | Dashboard, gauge, text, relay switch |
| `ESP32 to InfluxDB` | รับ MQTT telemetry แล้วเขียน InfluxDB |

หลังแก้ flow ใน editor ให้กด `Deploy`

Flow runtime อยู่ใน Docker volume ที่ `/data/flows.json` ส่วนไฟล์ `nodered/flows/esp32_level3_dashboard.json` เป็น seed สำหรับ image/เครื่องใหม่ อย่า bind mount seed file ตรงไปที่ `/data/flows.json` เพราะจะทำให้ Node-RED save flow แล้วเจอ `EBUSY`

## 6. Build และ Upload Firmware

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

ถ้า upload ไม่เจอบอร์ด:

```bash
/home/ubuntu/.platformio/penv/bin/pio device list
```

แล้วแก้ `upload_port` / `monitor_port` ใน `platformio.ini`

## 7. First Boot

1. Upload firmware ลง ESP32
2. ถ้ายังไม่มี WiFi credential บอร์ดจะเปิด portal `ESP32-Setup`
3. เชื่อม WiFi จากมือถือหรือคอม แล้วตั้งค่า SSID/password
4. OLED จะแสดง IP address
5. เปิด dashboard ของ ESP32 ที่ `http://<ESP32-IP>/`
6. เปิด Node-RED dashboard ที่ `http://localhost:1880/ui/`

Reset WiFi credentials:

- กด SW1 ค้างตอน boot ประมาณ 5 วินาที
- หรือใช้เมนู OLED `Settings -> WiFi Reset -> Confirm`

## 8. ควบคุม Relay

จาก OLED:

| ปุ่ม | หน้าที่ |
|---|---|
| SW1 | เข้าเมนู, ยืนยัน, เลือก relay ถัดไป |
| SW2 | เลื่อนลง หรือสั่ง relay OFF |
| SW3 | เลื่อนขึ้น หรือสั่ง relay ON |
| Hold SW1 | กลับหน้า monitor |

จาก Node-RED dashboard:

```text
http://localhost:1880/ui/
```

ใช้ switch `Relay 1`, `Relay 2`, `Relay 3`

จาก MQTT:

```bash
mosquitto_pub -h broker.hivemq.com -t ESP32-Level3/relay/1/set -m ON
mosquitto_pub -h broker.hivemq.com -t ESP32-Level3/relay/2/set -m OFF
mosquitto_pub -h broker.hivemq.com -t ESP32-Level3/relay/3/set -m TOGGLE
```

ดูทุก topic:

```bash
mosquitto_sub -h broker.hivemq.com -t "ESP32-Level3/#"
```

## 9. ตรวจสอบข้อมูล

ตรวจ Node-RED log:

```bash
docker compose logs -f nodered
```

ตรวจ InfluxDB health จาก Node-RED container:

```bash
docker compose exec nodered node -e 'require("http").get("http://influxdb:8086/health", r => { let s=""; r.on("data", d => s += d); r.on("end", () => console.log(r.statusCode, s)); })'
```

ตรวจ firmware build:

```bash
/home/ubuntu/.platformio/penv/bin/pio run
```

## 10. Backup และ Sync Flow

ถ้าแก้ flow ใน Node-RED editor แล้วต้องการเก็บเป็น seed ใน repo ให้ export flow จาก editor แล้วอัปเดต:

```text
nodered/flows/esp32_level3_dashboard.json
```

หลังจากแก้ seed file ให้ rebuild image:

```bash
docker compose up -d --build nodered
```

ก่อนล้าง volume ด้วย `docker compose down -v` ควร export flow และจด token/config สำคัญไว้ก่อน เพราะ `/data/flows.json` และ `/data/flows_cred.json` จะหายไปพร้อม volume

## 11. Troubleshooting

### Node-RED Deploy แล้ว save ไม่ได้

อาการ:

```text
EBUSY ... rename '/data/flows.json.$$$' -> '/data/flows.json'
```

สาเหตุหลักคือ bind mount ไฟล์ตรงไปที่ `/data/flows.json` ให้ใช้ named volume `nodered_data:/data` และ seed flow ผ่าน `nodered/entrypoint.sh`

### Node-RED ไม่มี telemetry

- ตรวจ MQTT input topic ต้องเป็น `ESP32-Level3/telemetry`
- ตรวจ ESP32 ต่อ internet และ MQTT ได้
- ตรวจ debug sidebar ใน Node-RED
- ตรวจว่า `MQTT_BASE` ใน firmware ตรงกับ flow

### InfluxDB ไม่ได้ข้อมูล

- URL ใน Node-RED ต้องเป็น `http://influxdb:8086`
- ตรวจ Org/Bucket/Measurement: `mylab`, `esp32_db`, `data_telemetry`
- ตรวจ token ใน config node `InfluxDB Server`
- ถ้า InfluxDB log ขึ้น `Unauthorized` ให้สร้างหรือใส่ token ใหม่

### Node-RED Dashboard ไม่ขึ้น

- เปิด `http://localhost:1880/ui/`
- ตรวจ log ต้องเห็น `Dashboard version 3.6.6 started at /ui`
- ถ้า node `ui_*` หรือ `influxdb` เป็น unknown ให้ rebuild:

```bash
docker compose up -d --build nodered
```

### ESP32 อ่าน XY-MD03 ไม่ได้

- ตรวจ RS485 A/B
- ตรวจ Slave ID เป็น `2`
- ตรวจ baud `9600`
- ถ้าต้อง debug serial หลัง init sensor ควรพิจารณาย้าย XY-MD03 ไป `Serial2`

### DS18B20 เข้า simulation mode

- ตรวจสาย data ที่ GPIO14
- ตรวจ pull-up 4.7k ไป 3.3V
- ตรวจ ground ร่วม
