# คู่มือติดตั้ง Docker Compose สำหรับ IoT Stack

คู่มือนี้เป็นขั้นตอนติดตั้งและเริ่มใช้งาน MQTT, Node-RED, InfluxDB และ Grafana ด้วย Docker Compose

> ให้รันคำสั่งทั้งหมดจากโฟลเดอร์โปรเจกต์นี้ เพื่อให้ Docker Compose อ่านไฟล์ `.env` และ `docker-compose.yml` ได้ถูกต้อง

ถ้ายังไม่มี `.env` ให้สร้างจากไฟล์ตัวอย่างก่อน:

```bash
cp .env.example .env
```

## 1. ตรวจสอบเครื่องมือที่ต้องมี

ตรวจสอบ Docker

```bash
docker --version
```

ตรวจสอบ Docker Compose

```bash
docker compose version
```

ถ้าทั้งสองคำสั่งแสดงเวอร์ชัน แปลว่าเครื่องพร้อมใช้งาน

## 2. ตรวจสอบพอร์ตมาตรฐาน

พอร์ตหลักกำหนดไว้ตรงใน `docker-compose.yml` เพื่อให้ host port และ container port ตรงกันทั้งหมด

```text
MQTT: 1883
MQTT WebSocket: 9001
Node-RED: 1880
InfluxDB: 8086
Grafana: 3000
```

ค่าเริ่มต้นของ image, path และ Grafana อยู่ในไฟล์ `.env` โดยมีค่า default สำรองใน `docker-compose.yml`

```env
GF_SECURITY_ADMIN_USER=admin
GF_SECURITY_ADMIN_PASSWORD=password
```

ถ้ามี container หรือโปรเจกต์อื่นใช้พอร์ตเดียวกันอยู่ ต้องหยุดตัวนั้นก่อน เพราะ Docker ไม่สามารถ publish host port เดียวกันพร้อมกันสอง container ได้

## 3. ตรวจสอบ Docker Compose config

ตรวจสอบว่า `docker-compose.yml` และ `.env` ถูกอ่านแล้ว resolve ค่าได้ถูกต้อง

```bash
docker compose config
```

ใช้คำสั่งนี้ก่อนเริ่มระบบเมื่อมีการแก้ `.env` หรือ `docker-compose.yml`

## 4. เริ่มระบบ

เริ่มทุก service แบบ background

```bash
docker compose up -d
```

Node-RED ใช้ image ของโปรเจกต์ที่ติดตั้ง `node-red-dashboard@3.6.6` และ `node-red-contrib-influxdb@0.7.0` ไว้แล้ว ถ้ายังไม่มี image ในเครื่อง Docker Compose จะ build จาก `nodered/Dockerfile`

ตรวจสอบสถานะ

```bash
docker compose ps
```

สถานะที่ควรเห็นเมื่อระบบพร้อมใช้งาน

```text
mqtt      Up
nodered   Up (healthy)
influxdb  Up
grafana   Up
```

## 5. ตรวจสอบเครื่องมือใน Node-RED

Flow dashboard ของโปรเจกต์นี้ใช้ `node-red-dashboard` รุ่น classic และใช้ `node-red-contrib-influxdb` สำหรับ InfluxDB โดยติดตั้งไว้ใน custom image แล้ว

```bash
docker compose exec nodered sh -lc 'npm ls --depth=0 node-red-dashboard node-red-contrib-influxdb'
```

ถ้าเปิด Node-RED แล้วเจอ node แบบ `ui_*` หรือ `influxdb` เป็น unknown ให้ rebuild image แล้วเปิดใหม่:

```bash
docker compose up -d --build nodered
```

โปรเจกต์นี้ bind mount flow จากไฟล์ใน workspace เข้า Node-RED โดยตรง:

```text
nodered/flows/esp32_level3_dashboard.json -> /data/flows.json
```

ดังนั้นเมื่อแก้ไฟล์ flow ใน VS Code ให้ restart Node-RED เพื่อโหลด flow ใหม่:

```bash
docker compose restart nodered
```

ตรวจสอบอีกครั้งว่า Node-RED กลับมา healthy

```bash
docker compose ps
```

## 6. เข้าใช้งานผ่าน Browser

เมื่อ container ทำงานครบแล้ว เข้าใช้งานได้ที่

```text
Node-RED editor: http://localhost:1880
Node-RED dashboard: http://localhost:1880/ui/
InfluxDB: http://localhost:8086
Grafana: http://localhost:3000
MQTT: localhost:1883
MQTT WebSocket: localhost:9001
```

Grafana ใช้ user/password จาก `.env`

```text
User: admin
Password: password
```

## 7. ดู Log

ดู log ของทุก service

```bash
docker compose logs
```

ดู log แบบ real-time

```bash
docker compose logs -f
```

ดู log เฉพาะ service

```bash
docker compose logs -f mqtt
docker compose logs -f nodered
docker compose logs -f influxdb
docker compose logs -f grafana
```

## 8. Restart Service

Restart ทุก service

```bash
docker compose restart
```

Restart เฉพาะ service

```bash
docker compose restart mqtt
docker compose restart nodered
docker compose restart influxdb
docker compose restart grafana
```

ใช้หลังแก้ config หรือหลังติดตั้ง node เพิ่มใน Node-RED

## 9. หยุดและเริ่มระบบ

หยุด container โดยยังเก็บ container เดิมไว้

```bash
docker compose stop
```

เริ่ม container ที่หยุดไว้กลับมา

```bash
docker compose start
```

หยุดและลบ container กับ network แต่ยังเก็บ volume ข้อมูลไว้

```bash
docker compose down
```

เริ่มใหม่หลัง `down`

```bash
docker compose up -d
```

## 10. ล้างข้อมูลทั้งหมด

คำสั่งนี้จะลบ container, network และ volume ทั้งหมดของโปรเจกต์

```bash
docker compose down -v
```

ใช้เมื่ออยากเริ่มใหม่จากศูนย์เท่านั้น เพราะ volume เก็บข้อมูลสำคัญ เช่น

- Node-RED nodes ที่ติดตั้งใน `/data`
- ข้อมูล InfluxDB
- การตั้งค่า Grafana
- ข้อมูล persistence ของ MQTT

หมายเหตุ: flow หลักของ Node-RED ถูก bind mount จาก `nodered/flows/esp32_level3_dashboard.json` จึงอยู่ใน workspace ไม่ได้หายไปพร้อม volume

## 11. อัปเดต Image

ดาวน์โหลด image เวอร์ชันล่าสุดตามที่ระบุใน `docker-compose.yml`

```bash
docker compose pull
```

เริ่ม container ใหม่หลัง pull

```bash
docker compose up -d
```

## 12. เข้า Shell ของ Container

เข้า container เพื่อ debug หรือดูไฟล์ภายใน

```bash
docker compose exec mqtt sh
docker compose exec nodered sh
docker compose exec influxdb sh
docker compose exec grafana sh
```

## 13. แก้ปัญหา Port ซ้ำ

ถ้า `docker compose up -d` แล้วเจอ error ประมาณนี้

```text
Bind for :::1883 failed: port is already allocated
```

แปลว่ามี process หรือ container อื่นใช้พอร์ตนั้นอยู่ ให้ดู container ที่กำลังรัน

```bash
docker ps
```

จากนั้นหยุด container ที่ใช้พอร์ตซ้ำ เช่น

```bash
docker stop <container-name>
```

แล้ว start โปรเจกต์นี้ใหม่

```bash
docker compose up -d
```

## 14. คำสั่งที่ใช้บ่อย

```bash
docker compose up -d
docker compose ps
docker compose logs -f
docker compose restart nodered
docker compose down
```

## 15. Node-RED / MQTT ที่ใช้ในโปรเจกต์นี้

Flow หลัก:

```text
nodered/flows/esp32_level3_dashboard.json
```

Dashboard ใช้ node-red-dashboard รุ่นเก่า (`ui_gauge`, `ui_text`, `ui_switch`) และเปิดที่ path `/ui/`

MQTT settings ที่ต้องตรงกับ firmware:

```text
Host: broker.hivemq.com
Port: 1883
Base topic: ESP32-Level3
Telemetry: ESP32-Level3/telemetry
Relay command: ESP32-Level3/relay/{1|2|3}/set
```
