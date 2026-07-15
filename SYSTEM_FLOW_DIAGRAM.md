# ESP32_nodeRED_influxDB_grafana_Level3 — System Flow Diagram

เอกสารนี้อธิบายการเชื่อมโยงและทิศทางข้อมูลของระบบตั้งแต่ sensor บน ESP32 ไปจนถึง
Node-RED, InfluxDB และ Grafana รวมถึงทิศทางคำสั่งควบคุม relay ย้อนกลับมายัง ESP32

> Mermaid diagram แสดงได้โดยตรงบน GitHub และ VS Code ที่รองรับ Mermaid

## 1. ภาพรวมระบบ

```mermaid
flowchart LR
    subgraph FIELD["Hardware / Field"]
        DS["DS18B20<br/>Temperature"]
        XY["XY-MD03<br/>Temperature + Humidity"]
        BTN["SW1-SW3<br/>Local Control"]
        RELAY["Relay 1-3"]
        OLED["OLED Display"]
    end

    subgraph DEVICE["ESP32 DevKit V1"]
        FW["Firmware<br/>Read sensors + control relay"]
        WEB["Web Dashboard<br/>HTTP + WebSocket"]
        NVS["NVS<br/>Relay state + boot count"]
    end

    OWM["OpenWeatherMap API<br/>Weather + PM2.5 + AQI"]
    MQTT["HiveMQ Public Broker<br/>broker.hivemq.com:1883"]

    subgraph DOCKER["Docker Compose"]
        NR["Node-RED<br/>:1880"]
        NRUI["Node-RED Dashboard<br/>:1880/ui/"]
        INFLUX["InfluxDB 2<br/>:8086"]
        GRAFANA["Grafana<br/>:3000"]
        LOCALMQTT["Mosquitto<br/>:1883 / :9001<br/>optional"]
    end

    USER["User / Browser"]

    DS --> FW
    XY --> FW
    BTN --> FW
    FW --> RELAY
    FW --> OLED
    FW <--> NVS
    OWM -->|"HTTPS every 5 min"| FW
    FW <--> WEB
    USER <-->|"LAN"| WEB

    FW -->|"Telemetry + state"| MQTT
    MQTT -->|"Telemetry"| NR
    NR --> NRUI
    USER <-->|"View + relay switch"| NRUI
    NR -->|"Relay set command"| MQTT
    MQTT -->|"ON / OFF / TOGGLE"| FW
    NR -->|"Write fields"| INFLUX
    INFLUX -->|"Flux query"| GRAFANA
    USER <-->|"Historical dashboard"| GRAFANA

    FW -. "Can be configured later" .-> LOCALMQTT
    LOCALMQTT -. "Can replace HiveMQ" .-> NR
```

เส้นทึบคือเส้นที่ระบบใช้งานอยู่ปัจจุบัน ส่วนเส้นประคือทางเลือกที่ยังต้องแก้ค่า
`MQTT_HOST` ใน firmware และ MQTT broker config ใน Node-RED ให้ตรงกันก่อนใช้ Mosquitto

## 2. Telemetry Flow: ESP32 → Grafana

```mermaid
sequenceDiagram
    autonumber
    participant Sensor as Sensors / Weather
    participant ESP as ESP32 Firmware
    participant MQTT as HiveMQ Broker
    participant NR as Node-RED
    participant DB as InfluxDB
    participant GF as Grafana
    participant User as User

    Sensor->>ESP: Update sensor and weather values
    ESP->>ESP: Build telemetry JSON every 5000 ms
    ESP->>MQTT: Publish ESP32-Level3/telemetry (retained)
    MQTT-->>NR: Deliver telemetry JSON
    NR->>NR: Parse JSON and build numeric/string fields
    NR->>DB: Write ESP32level3_telemetry
    User->>GF: Open or refresh dashboard
    GF->>DB: Flux query via http://influxdb:8086
    DB-->>GF: Time-series records
    GF-->>User: Render gauges, status and graphs
```

จุดที่ต้องตรงกันตลอดเส้นทาง:

| ช่วง | ค่าที่ใช้ |
|---|---|
| ESP32 → MQTT | `ESP32-Level3/telemetry` |
| Node-RED → InfluxDB | Organization `mylab`, bucket `esp32_db` |
| InfluxDB measurement | `ESP32level3_telemetry` |
| Grafana → InfluxDB | `http://influxdb:8086` จากภายใน container |
| Grafana Data Source | Flux + token ที่มีสิทธิ์อ่าน `esp32_db` |

## 3. Relay Control Flow: Node-RED → ESP32

```mermaid
sequenceDiagram
    autonumber
    participant User as User
    participant UI as Node-RED Dashboard
    participant NR as Node-RED Flow
    participant MQTT as HiveMQ Broker
    participant ESP as ESP32 Firmware
    participant Relay as Relay 1-3
    participant NVS as ESP32 NVS

    User->>UI: Change relay switch
    UI->>NR: Boolean ON / OFF
    NR->>MQTT: Publish ESP32-Level3/relay/N/set
    MQTT-->>ESP: ON / OFF
    ESP->>Relay: Apply active-low output
    ESP->>NVS: Save state only when changed
    ESP->>MQTT: Publish ESP32-Level3/relay/N/state (retained)
    MQTT-->>NR: Return actual relay state
    NR-->>UI: Synchronize dashboard switch
```

คำสั่ง MQTT ที่ firmware รองรับคือ `ON`, `OFF` และ `TOGGLE` โดยแทน `N` ด้วย `1`, `2` หรือ `3`

## 4. Local Control Flow บน ESP32

```mermaid
flowchart TD
    INPUT{"Control source"}
    BUTTON["SW1-SW3 / OLED menu"]
    LOCALWEB["ESP32 Web Dashboard<br/>WebSocket /ws"]
    MQTTCMD["MQTT relay/N/set"]
    EVENT["Relay changed event"]
    OUTPUT["Set GPIO relay<br/>Active Low"]
    SAVE["Save changed state to NVS"]
    PUB["Publish retained relay state"]
    REDRAW["Refresh OLED + Web snapshot"]

    INPUT --> BUTTON
    INPUT --> LOCALWEB
    INPUT --> MQTTCMD
    BUTTON --> EVENT
    LOCALWEB --> EVENT
    MQTTCMD --> EVENT
    EVENT --> OUTPUT
    EVENT --> SAVE
    EVENT --> PUB
    EVENT --> REDRAW
```

ไม่ว่าจะสั่ง relay จากปุ่ม, ESP32 Web Dashboard หรือ MQTT ผลลัพธ์จะถูกส่งเข้าเส้นทาง
อัปเดตเดียวกัน ทำให้ GPIO, NVS, OLED, Web Dashboard และ MQTT state สอดคล้องกัน

## 5. Docker Services และ Network

```mermaid
flowchart TB
    HOST["Host machine / Browser"]

    subgraph NET["Docker network: iot_net"]
        MOSQ["mqtt / Mosquitto<br/>container port 1883, 9001"]
        NR["nodered<br/>container port 1880"]
        DB["influxdb<br/>container port 8086"]
        GF["grafana<br/>container port 3000"]
    end

    V1[(mqtt_data + mqtt_log)]
    V2[(nodered_data)]
    V3[(influxdb_data + influxdb_config)]
    V4[(grafana_data)]

    HOST -->|"localhost:1880"| NR
    HOST -->|"localhost:8086"| DB
    HOST -->|"localhost:3000"| GF
    HOST -->|"localhost:1883 / 9001"| MOSQ

    NR -->|"http://influxdb:8086"| DB
    GF -->|"http://influxdb:8086"| DB
    MOSQ --- V1
    NR --- V2
    DB --- V3
    GF --- V4
```

`localhost` ใช้เมื่อเข้าบริการจาก browser บนเครื่อง host แต่ container ที่คุยกันต้องใช้ Docker service name
เช่น `http://influxdb:8086` ไม่ใช่ `http://localhost:8086`

## 6. InfluxDB Field Mapping

```mermaid
flowchart LR
    JSON["ESP32 telemetry JSON"]
    PARSE["Node-RED<br/>Build InfluxDB Points"]
    MEASURE["Measurement<br/>ESP32level3_telemetry"]
    SENSOR["Sensor fields<br/>ds18_temp<br/>xymd_temp<br/>xymd_hum"]
    WEATHER["Weather fields<br/>weather_temp<br/>weather_hum<br/>weather_rain<br/>weather_pm25<br/>weather_aqi"]
    SYSTEM["System fields<br/>sys_rssi<br/>sys_heap<br/>sys_uptime<br/>sys_ip"]
    RELAY["Relay fields<br/>relay1<br/>relay2<br/>relay3"]
    PANELS["Grafana panels"]

    JSON --> PARSE --> MEASURE
    MEASURE --> SENSOR --> PANELS
    MEASURE --> WEATHER --> PANELS
    MEASURE --> SYSTEM --> PANELS
    MEASURE --> RELAY --> PANELS
```

Grafana Time series ควรใช้ `aggregateWindow(every: v.windowPeriod, ...)` ส่วน panel ค่าปัจจุบันหรือสถานะ
ควรใช้ `last()` เพื่อลดจำนวน record ที่ส่งกลับมา

## 7. จุดตรวจเมื่อข้อมูลขาดช่วง

| อาการ | จุดที่ควรตรวจตามลำดับ |
|---|---|
| Node-RED ไม่มี telemetry | ESP32 WiFi → HiveMQ → topic `ESP32-Level3/telemetry` → MQTT input node |
| InfluxDB ไม่มีข้อมูล | Node-RED flow deployed → URL `http://influxdb:8086` → write token → org/bucket/measurement |
| Grafana ขึ้น `No data` | InfluxDB Data Explorer → Grafana Data Source → read token → Flux field → dashboard time range |
| สั่ง relay ไม่ทำงาน | Node-RED switch → `relay/N/set` → HiveMQ → ESP32 subscription → GPIO |
| Dashboard แสดงสถานะ relay ไม่ตรง | `relay/N/state` retained → Node-RED state input → UI switch synchronization |

## 8. ไฟล์ที่รับผิดชอบแต่ละจุด

| ส่วนของระบบ | ไฟล์หลัก |
|---|---|
| Firmware orchestration | `esp32_firmware/src/main.cpp` |
| MQTT telemetry/control | `esp32_firmware/include/DevMQTT.h` |
| MQTT/API configuration | `esp32_firmware/include/config.h`, `config_private.h` |
| ESP32 local web | `esp32_firmware/include/DevWebServer.h`, `dashboard.h` |
| Node-RED flow | `nodered/flows/esp32_level3_dashboard.json` |
| Docker services/network/volumes | `docker-compose.yml`, `.env` |
| Grafana dashboard | `grafana/dashboards/grafana-dashboard-esp32.json` |
| Grafana Flux examples | `GRAFANA_QUERY_GUIDE.md` |

## หมายเหตุเรื่อง MQTT Broker

ปัจจุบัน firmware และ Node-RED flow ใช้ `broker.hivemq.com:1883` ทั้งคู่ ดังนั้น Mosquitto service ใน
`docker-compose.yml` ยังไม่ได้อยู่ใน data path หลัก หากจะย้ายมาใช้ local Mosquitto ต้องแก้ broker
ทั้งใน firmware และ Node-RED พร้อมกัน และ ESP32 ต้องเข้าถึง IP ของเครื่อง Docker ได้
