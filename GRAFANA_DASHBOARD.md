# Grafana Dashboard สำหรับ ESP32 Level3

เอกสารนี้อธิบายการเชื่อม Grafana กับ InfluxDB และการนำเข้า dashboard จากไฟล์
`grafana-dashboard-esp32.json` เพื่อแสดงข้อมูล telemetry ของ ESP32 Level3

## ภาพรวมการไหลของข้อมูล

```text
ESP32 -> MQTT -> Node-RED -> InfluxDB -> Grafana
```

- ESP32 ส่ง telemetry ไปยัง MQTT topic `ESP32-Level3/telemetry`
- Node-RED รับ payload แล้วแปลงข้อมูลเป็น fields
- Node-RED เขียนข้อมูลลง InfluxDB
- Grafana อ่านข้อมูลจาก InfluxDB ด้วยภาษา Flux

ค่าที่ใช้ในโปรเจกต์นี้:

| รายการ | ค่า |
|---|---|
| Grafana URL | `http://localhost:3000` |
| InfluxDB URL จากเครื่องผู้ใช้ | `http://localhost:8086` |
| InfluxDB URL จาก Grafana container | `http://influxdb:8086` |
| Organization | `mylab` |
| Bucket | `esp32_db` |
| Measurement | `ESP32level3_telemetry` |
| Dashboard JSON | `grafana-dashboard-esp32.json` |

## 1. เตรียมระบบ

สร้างไฟล์ environment หากยังไม่มี:

```bash
cp .env.example .env
```

ค่าเริ่มต้นสำหรับเข้าสู่ Grafana อยู่ใน `.env`:

```dotenv
GF_SECURITY_ADMIN_USER=admin
GF_SECURITY_ADMIN_PASSWORD=password
```

ควรเปลี่ยนรหัสผ่านก่อนนำระบบไปใช้งานจริง และไม่ควร commit `.env` ที่มีรหัสผ่านจริงขึ้น repository

เริ่ม Docker stack:

```bash
docker compose up -d
```

ตรวจสถานะ service:

```bash
docker compose ps
```

ควรเห็น `mqtt`, `nodered`, `influxdb` และ `grafana` อยู่ในสถานะทำงาน

## 2. เตรียม InfluxDB

เปิด `http://localhost:8086` แล้วสร้างค่าต่อไปนี้:

```text
Organization: mylab
Bucket: esp32_db
```

สร้าง API token ที่มีสิทธิ์อ่าน bucket `esp32_db` สำหรับ Grafana โดยแนะนำให้สร้าง token
แยกจาก token ที่ Node-RED ใช้เขียนข้อมูล

Node-RED ต้องเขียนข้อมูลด้วยค่าต่อไปนี้:

```text
URL: http://influxdb:8086
Organization: mylab
Bucket: esp32_db
Measurement: ESP32level3_telemetry
```

ชื่อ `influxdb` เป็น Docker service name จึงต้องใช้แทน `localhost` เมื่อเชื่อมต่อจาก container อื่น

## 3. เพิ่ม InfluxDB Data Source ใน Grafana

1. เปิด `http://localhost:3000`
2. เข้าสู่ระบบด้วยชื่อผู้ใช้และรหัสผ่านจาก `.env`
3. ไปที่ **Connections > Data sources**
4. เลือก **Add new data source** แล้วเลือก **InfluxDB**
5. ตั้งค่า **Query language** เป็น `Flux`
6. กรอกค่าดังนี้

| ช่อง | ค่า |
|---|---|
| Name | `InfluxDB-ESP32` |
| URL | `http://influxdb:8086` |
| Basic Auth | ปิด |
| Organization | `mylab` |
| Token | token สำหรับอ่านข้อมูลที่สร้างจาก InfluxDB |
| Default Bucket | `esp32_db` |

กด **Save & test** และตรวจว่าการเชื่อมต่อสำเร็จ

> ถ้า Grafana รันผ่าน Docker Compose ห้ามใช้ `http://localhost:8086` ใน Data Source เพราะ
> `localhost` จะหมายถึง Grafana container เอง

## 4. นำเข้า Dashboard

1. ไปที่ **Dashboards > New > Import**
2. กด **Upload dashboard JSON file**
3. เลือกไฟล์ `grafana-dashboard-esp32.json` จากโฟลเดอร์หลักของโปรเจกต์
4. ที่ช่อง `InfluxDB-ESP32` เลือก Data Source ที่สร้างไว้
5. กด **Import**

Dashboard จะตั้งค่า refresh ทุก 10 วินาที และแสดงข้อมูลย้อนหลัง 1 ชั่วโมงเป็นค่าเริ่มต้น

Time series ในไฟล์ใช้ `aggregateWindow(every: v.windowPeriod, ...)` และ panel แบบสถานะใช้
`last()` เพื่อลดจำนวนข้อมูลที่ InfluxDB ต้องส่งกลับเมื่อเลือกช่วงเวลานาน

## 5. ตรวจ Measurement ให้ตรงกับ Node-RED

ทุก panel ต้องใช้ measurement ชื่อ `ESP32level3_telemetry` ซึ่งเป็นชื่อเดียวกับ Node-RED flow
หาก panel ขึ้น `No data` ให้เปิด **Edit** แล้วตรวจ `_measurement` ใน Flux query

ตัวอย่าง query ที่ตรงกับ flow ปัจจุบัน:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "xymd_temp")
```

## 6. ข้อมูลที่แสดงบน Dashboard

| Panel | InfluxDB field | หน่วย/รูปแบบ |
|---|---|---|
| DS18B20 Temperature | `ds18_temp` | °C |
| XY-MD03 Temperature | `xymd_temp` | °C |
| XY-MD03 Humidity | `xymd_hum` | % |
| Weather Temperature | `weather_temp` | °C |
| Weather Humidity | `weather_hum` | % |
| Weather Rain Chance | `weather_rain` | % |
| PM2.5 | `weather_pm25` | µg/m³ |
| AQI | `weather_aqi` | AQI level |
| WiFi RSSI | `sys_rssi` | dBm |
| Free Heap | `sys_heap` | bytes |
| Uptime | `sys_uptime` | seconds |
| ESP32 IP Address | `sys_ip` | text |
| Relay 1 Status | `relay1` | `0` = OFF, `1` = ON |
| Relay 2 Status | `relay2` | `0` = OFF, `1` = ON |
| Relay 3 Status | `relay3` | `0` = OFF, `1` = ON |

Node-RED ยังเขียน fields อื่นที่สามารถนำไปสร้าง panel เพิ่มได้ เช่น `ds18_sim` และ
`weather_aqiLabel`

## 7. ตรวจสอบข้อมูล

ก่อนตรวจ Grafana ควรยืนยันว่า ESP32 ส่ง MQTT telemetry และ Node-RED เขียนข้อมูลเข้า InfluxDB แล้ว

เปิด **Data Explorer** ใน InfluxDB แล้วใช้ query:

```flux
from(bucket: "esp32_db")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
```

หรือเปิด **Explore** ใน Grafana แล้วทดลอง query เดียวกัน หากมีข้อมูลใน InfluxDB แต่ไม่มีใน
dashboard ให้ตรวจ Data Source, measurement, field และช่วงเวลาที่เลือก

## 8. การแก้ปัญหา

### Grafana เปิดไม่ได้

ตรวจสถานะและ log:

```bash
docker compose ps grafana
docker compose logs --tail=100 grafana
```

ตรวจว่า port `3000` ไม่ถูกโปรแกรมอื่นใช้งาน

### Save & test ไม่ผ่าน

- URL ภายใน Grafana container ต้องเป็น `http://influxdb:8086`
- ปิด Basic Auth; InfluxDB v2 ใช้ token ในส่วน InfluxDB Details
- Organization ต้องเป็น `mylab`
- Token ต้องมีสิทธิ์อ่าน bucket `esp32_db`
- ตรวจว่า InfluxDB container ทำงานอยู่

### Dashboard ขึ้น No data

- ตรวจว่า measurement เป็น `ESP32level3_telemetry`
- ตรวจว่าเลือก Data Source `InfluxDB-ESP32`
- ตรวจช่วงเวลามุมขวาบน เช่น `Last 1 hour`
- ตรวจว่า ESP32 เชื่อม MQTT และส่ง topic `ESP32-Level3/telemetry`
- ตรวจว่า Node-RED flow `ESP32 to InfluxDB` ถูก Deploy แล้ว
- ตรวจ token ของ Node-RED ว่ามีสิทธิ์เขียน bucket

### มีข้อมูลบาง panel เท่านั้น

บาง field จะไม่มีข้อมูลจนกว่าอุปกรณ์หรือบริการต้นทางจะพร้อม เช่น sensor ไม่ได้เชื่อมต่อ หรือยังไม่ได้ตั้ง
OpenWeatherMap API key ให้ตรวจ payload MQTT และ debug sidebar ของ Node-RED เพื่อดูว่า field นั้นถูกส่งมาหรือไม่

## 9. การเก็บรักษา Dashboard

ข้อมูลและ dashboard ที่สร้างใน Grafana ถูกเก็บใน Docker volume `grafana_data` จึงยังอยู่หลัง restart
container แต่จะหายหากลบ volume นี้

เมื่อแก้ dashboard แล้วต้องการเก็บเวอร์ชันล่าสุดใน repository:

1. เปิด dashboard ใน Grafana
2. ไปที่ **Dashboard settings > JSON model** หรือเลือก **Export**
3. บันทึกทับ `grafana-dashboard-esp32.json`
4. ตรวจว่าไฟล์ไม่มี token, password หรือข้อมูลลับก่อน commit
