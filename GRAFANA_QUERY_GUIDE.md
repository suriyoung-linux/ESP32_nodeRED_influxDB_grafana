# ESP32_nodeRED_influxDB_grafana_Level3 — Grafana Flux Query Guide

คู่มือนี้ใช้สำหรับสร้าง Grafana panel ด้วยภาษา Flux เพื่ออ่านข้อมูลจาก InfluxDB ของโปรเจกต์
โดยอ้างอิงค่าที่ระบบปัจจุบันใช้งานจริง

## ค่าหลักที่ใช้

| รายการ | ค่า |
|---|---|
| Data source | `influxdb` |
| Query language | `Flux` |
| Organization | `mylab` |
| Bucket | `esp32_db` |
| Measurement | `ESP32level3_telemetry` |
| URL ภายใน Docker | `http://influxdb:8086` |

> Query ในเอกสารนี้ใช้ measurement `ESP32level3_telemetry` เพราะเป็นชื่อที่ Node-RED runtime
> กำลังเขียนข้อมูลอยู่ หากภายหลังเปลี่ยน measurement ใน Node-RED ต้องแก้ `_measurement` ใน query ให้ตรงกัน

## 1. วิธีสร้าง Panel

1. เปิด Grafana ที่ `http://localhost:3000`
2. เปิด dashboard ที่ต้องการ
3. กด **Add > Visualization**
4. เลือก Data source ชื่อ `influxdb`
5. เปลี่ยน Query editor เป็น **Code**
6. วาง Flux query จากตัวอย่างในเอกสารนี้
7. กด **Run queries** แล้วเลือกชนิด Visualization
8. กด **Apply** และ **Save dashboard**

## 2. Query ค่า DS18B20 Temperature

ใช้กับ Visualization ชนิด **Time series** และตั้ง Unit เป็น `Celsius (°C)`:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "ds18_temp")
  |> aggregateWindow(every: v.windowPeriod, fn: mean, createEmpty: false)
  |> yield(name: "DS18B20 Temperature")
```

`aggregateWindow()` ช่วยรวมจุดข้อมูลตามความกว้างของกราฟ ทำให้กราฟเร็วและอ่านง่ายเมื่อเลือกช่วงเวลานาน

ถ้าต้องการแสดงเฉพาะค่าล่าสุดด้วย Visualization ชนิด **Stat** หรือ **Gauge**:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "ds18_temp")
  |> last()
```

## 3. Query ค่า Sensor และ Weather

เปลี่ยนชื่อ `_field` ตามค่าที่ต้องการ โดยใช้โครงสร้างเดียวกับ DS18B20

| ข้อมูล | Field | Unit ที่แนะนำใน Grafana |
|---|---|---|
| DS18B20 temperature | `ds18_temp` | Celsius (°C) |
| DS18B20 simulation flag | `ds18_sim` | Boolean หรือ Short |
| XY-MD temperature | `xymd_temp` | Celsius (°C) |
| XY-MD humidity | `xymd_hum` | Percent (0-100) |
| Weather temperature | `weather_temp` | Celsius (°C) |
| Weather humidity | `weather_hum` | Percent (0-100) |
| Rain chance | `weather_rain` | Percent (0-100) |
| PM2.5 | `weather_pm25` | µg/m³ |
| Air Quality Index | `weather_aqi` | None |
| AQI label | `weather_aqiLabel` | String |

ตัวอย่าง XY-MD temperature:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "xymd_temp")
  |> aggregateWindow(every: v.windowPeriod, fn: mean, createEmpty: false)
```

ตัวอย่าง XY-MD humidity:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "xymd_hum")
  |> aggregateWindow(every: v.windowPeriod, fn: mean, createEmpty: false)
```

ตัวอย่าง PM2.5:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "weather_pm25")
  |> aggregateWindow(every: v.windowPeriod, fn: mean, createEmpty: false)
```

## 4. Query ข้อมูลระบบ ESP32

| ข้อมูล | Field | Unit ที่แนะนำใน Grafana |
|---|---|---|
| Wi-Fi signal | `sys_rssi` | Decibel-milliwatts (dBm) |
| Free heap | `sys_heap` | Bytes (IEC) |
| Uptime | `sys_uptime` | Seconds หรือ Duration |
| IP address | `sys_ip` | String |

ตัวอย่าง Wi-Fi RSSI แบบ Time series:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "sys_rssi")
  |> aggregateWindow(every: v.windowPeriod, fn: mean, createEmpty: false)
```

ตัวอย่าง Free Heap แบบ Time series:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "sys_heap")
  |> aggregateWindow(every: v.windowPeriod, fn: mean, createEmpty: false)
```

ตัวอย่าง IP address ล่าสุด ใช้ Visualization ชนิด **Stat** หรือ **Table**:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "sys_ip")
  |> last()
```

ไม่ควรใช้ `mean()` กับ `sys_ip` หรือ `weather_aqiLabel` เพราะเป็นข้อมูลชนิดข้อความ

## 5. Query สถานะ Relay

Relay ถูกบันทึกเป็นตัวเลข:

```text
0 = OFF
1 = ON
```

ตัวอย่าง Relay 1 ล่าสุด:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) => r._field == "relay1")
  |> last()
```

เปลี่ยน `_field` เป็น `relay2` หรือ `relay3` สำหรับ relay ตัวอื่น

แนะนำให้ใช้ Visualization ชนิด **Stat** และตั้งค่า Value mappings:

| Value | Text | Color |
|---|---|---|
| `0` | `OFF` | Red |
| `1` | `ON` | Green |

## 6. แสดงหลายค่าในกราฟเดียว

ตัวอย่างเปรียบเทียบอุณหภูมิจาก DS18B20, XY-MD และ OpenWeatherMap:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> filter(fn: (r) =>
    r._field == "ds18_temp" or
    r._field == "xymd_temp" or
    r._field == "weather_temp"
  )
  |> aggregateWindow(every: v.windowPeriod, fn: mean, createEmpty: false)
  |> yield(name: "Temperatures")
```

Grafana จะแยกเส้นตามชื่อ `_field` โดยอัตโนมัติ

## 7. ดูข้อมูลล่าสุดทุก Field

ใช้กับ Visualization ชนิด **Table** เพื่อตรวจสอบว่าข้อมูลใดถูกส่งเข้า InfluxDB แล้ว:

```flux
from(bucket: "esp32_db")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> group(columns: ["_field"])
  |> last()
  |> keep(columns: ["_time", "_field", "_value"])
```

## 8. ค้นหารายชื่อ Field ทั้งหมด

เปิด **Explore** ใน Grafana แล้วใช้ query นี้:

```flux
import "influxdata/influxdb/schema"

schema.measurementFieldKeys(
  bucket: "esp32_db",
  measurement: "ESP32level3_telemetry",
  start: v.timeRangeStart,
  stop: v.timeRangeStop
)
```

หากไม่พบ field ให้ขยายช่วงเวลาเป็น `Last 24 hours` หรือ `Last 7 days`

## 9. Query ด้วยช่วงเวลาคงที่

ปกติควรใช้ช่วงเวลาจาก Grafana:

```flux
|> range(start: v.timeRangeStart, stop: v.timeRangeStop)
```

ถ้าต้องการบังคับช่วงเวลาสำหรับทดสอบ สามารถใช้:

```flux
|> range(start: -1h)
```

ตัวอย่าง `-15m`, `-1h`, `-24h` และ `-7d` หมายถึงย้อนหลัง 15 นาที, 1 ชั่วโมง,
24 ชั่วโมง และ 7 วันตามลำดับ

## 10. แก้ปัญหา No data

ตรวจตามลำดับนี้:

1. Data source ต้องเชื่อมต่อสำเร็จและใช้ URL `http://influxdb:8086`
2. Bucket ต้องเป็น `esp32_db`
3. Measurement ต้องเป็น `ESP32level3_telemetry`
4. ตรวจตัวพิมพ์เล็ก-ใหญ่ของ field เช่น `ds18_temp`
5. เลือกช่วงเวลา `Last 1 hour` หรือกว้างกว่า
6. ตรวจว่า Node-RED flow รับ MQTT telemetry และเขียน InfluxDB อยู่
7. ใช้ query ในหัวข้อ **ดูข้อมูลล่าสุดทุก Field** เพื่อตรวจข้อมูลก่อนสร้าง panel

Query สำหรับทดสอบแบบสั้น:

```flux
from(bucket: "esp32_db")
  |> range(start: -24h)
  |> filter(fn: (r) => r._measurement == "ESP32level3_telemetry")
  |> limit(n: 20)
```
