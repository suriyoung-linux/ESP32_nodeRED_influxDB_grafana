# คู่มือติดตั้ง Docker Compose สำหรับ IoT Stack

คู่มือนี้เป็นขั้นตอนติดตั้งและเริ่มใช้งาน MQTT, Node-RED, InfluxDB และ Grafana ด้วย Docker Compose

> ให้รันคำสั่งทั้งหมดจากโฟลเดอร์โปรเจกต์นี้ เพื่อให้ Docker Compose อ่านไฟล์ `.env` และ `docker-compose.yml` ได้ถูกต้อง

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

## 2. ตรวจสอบไฟล์ตั้งค่าพอร์ต

ค่าพอร์ตหลักอยู่ในไฟล์ `.env`

```env
MQTT_PORT=1883
MQTT_WS_PORT=9001
NODERED_PORT=1880
INFLUXDB_PORT=8086
GRAFANA_PORT=3000
```

ค่าเริ่มต้นของ Grafana อยู่ในไฟล์เดียวกัน

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

ตรวจสอบสถานะ

```bash
docker compose ps
```

สถานะที่ควรเห็นเมื่อระบบพร้อมใช้งาน

```text
iot-mqtt      Up
iot-nodered   Up (healthy)
iot-influxdb  Up
iot-grafana   Up
```

## 5. ติดตั้งเครื่องมือใน Node-RED

เข้าไปติดตั้ง Node-RED nodes ใน volume `/data` ของ container `nodered`

```bash
docker compose exec nodered sh -lc 'cd /data && npm install node-red-dashboard node-red-contrib-influxdb'
```

ตรวจสอบรายการ node ที่ติดตั้งแล้ว

```bash
docker compose exec nodered sh -lc 'cd /data && npm ls --depth=0'
```

ควรเห็นอย่างน้อยรายการเหล่านี้

```text
node-red-dashboard
node-red-contrib-influxdb
```

Restart Node-RED เพื่อให้โหลด palette ใหม่

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
Node-RED dashboard: http://localhost:1880/ui
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

- Flow ของ Node-RED
- Node-RED nodes ที่ติดตั้งใน `/data`
- ข้อมูล InfluxDB
- การตั้งค่า Grafana
- ข้อมูล persistence ของ MQTT

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
