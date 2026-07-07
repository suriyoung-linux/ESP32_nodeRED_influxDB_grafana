# วิธีสร้าง Repository ใหม่บน GitHub และอัพโหลดโปรเจกต์

คู่มือนี้ใช้สำหรับอัพโหลดโปรเจกต์ในเครื่องขึ้น GitHub หรืออัพเดตโค้ดใหม่เข้า repository เดิม

## 1. ตรวจสอบ Git ในเครื่อง

เปิด Terminal ที่โฟลเดอร์โปรเจกต์ แล้วตรวจสอบสถานะ:

```bash
git status
```

ถ้ายังไม่ได้ตั้งชื่อผู้ใช้ Git ให้ตั้งค่าก่อน:

```bash
git config --global user.name "ชื่อของคุณ"
git config --global user.email "อีเมล GitHub ของคุณ"
```

## 2. สร้าง Repository ใหม่บน GitHub

1. เข้าเว็บไซต์ <https://github.com>
2. กดปุ่ม **New repository**
3. ตั้งชื่อ repository เช่น `ESP32-Project-Level3`
4. เลือกเป็น **Public** หรือ **Private**
5. ไม่ต้องติ๊ก `Add a README file` ถ้าในโปรเจกต์มีไฟล์อยู่แล้ว
6. กด **Create repository**

หลังจากสร้างเสร็จ GitHub จะแสดง URL ของ repository เช่น:

```text
https://github.com/USERNAME/REPOSITORY.git
```

หรือแบบ SSH:

```text
git@github.com:USERNAME/REPOSITORY.git
```

## 3. เตรียมโปรเจกต์ในเครื่อง

ถ้าโปรเจกต์ยังไม่เคยใช้ Git ให้เริ่มด้วย:

```bash
git init
```

เพิ่มไฟล์ทั้งหมดเข้า Git:

```bash
git add .
```

สร้าง commit แรก:

```bash
git commit -m "Initial commit"
```

## 4. เชื่อมโปรเจกต์กับ GitHub

เพิ่ม remote repository:

```bash
git remote add origin https://github.com/USERNAME/REPOSITORY.git
```

ตรวจสอบ remote:

```bash
git remote -v
```

ถ้าเคยตั้ง `origin` ไว้แล้วและต้องการเปลี่ยน URL ให้ใช้:

```bash
git remote set-url origin https://github.com/USERNAME/REPOSITORY.git
```

## 5. อัพโหลดขึ้น GitHub ครั้งแรก

ตั้งชื่อ branch หลักเป็น `main`:

```bash
git branch -M main
```

อัพโหลดขึ้น GitHub:

```bash
git push -u origin main
```

หลังจากนั้นเปิดหน้า repository บน GitHub เพื่อตรวจสอบว่าไฟล์ถูกอัพโหลดแล้ว

## 6. อัพเดตโค้ดขึ้น GitHub ครั้งต่อไป

เมื่อแก้ไขไฟล์ในโปรเจกต์แล้ว ให้ใช้คำสั่งตามลำดับนี้:

```bash
git status
git add .
git commit -m "อธิบายสิ่งที่แก้ไข"
git push
```

ตัวอย่าง:

```bash
git commit -m "Update ESP32 Level3 MQTT and Node-RED dashboard"
git push
```

ไฟล์สำคัญของโปรเจกต์นี้ที่ควรตรวจว่าอยู่ใน commit:

```text
esp32_firmware/
docker-compose.yml
.env.example
nodered/Dockerfile
nodered/entrypoint.sh
nodered/flows/esp32_level3_dashboard.json
USER_GUIDE.md
DOCKER_GUIDE.md
```

> ไม่ควร commit ไฟล์ `.env` จริง เพราะอาจมีรหัสผ่านหรือ token ให้ commit `.env.example` แทน แล้วค่อย copy เป็น `.env` ในเครื่องที่ deploy

> Node-RED credentials เช่น InfluxDB token อยู่ใน `flows_cred.json` แบบ encrypted ภายใน volume/runtime ไม่ควร commit token จริงขึ้น repository ให้ตั้งค่า token ผ่าน Node-RED editor ในเครื่อง deploy

## 7. ดึงโค้ดล่าสุดจาก GitHub

ถ้าต้องการดึงโค้ดล่าสุดจาก GitHub ลงมาในเครื่อง:

```bash
git pull
```

ควรรันคำสั่งนี้ก่อนเริ่มแก้ไขงาน ถ้ามีการทำงานจากหลายเครื่องหรือหลายคน

## 8. คำสั่งที่ใช้บ่อย

ดูสถานะไฟล์:

```bash
git status
```

ดูประวัติ commit:

```bash
git log --oneline
```

ดู remote repository:

```bash
git remote -v
```

ดู branch ปัจจุบัน:

```bash
git branch
```

## 9. ปัญหาที่พบบ่อย

### Push ไม่ผ่านเพราะยังไม่ได้ login

ถ้าใช้ HTTPS GitHub อาจให้ login ผ่าน browser หรือใช้ Personal Access Token แทนรหัสผ่าน

### Remote origin มีอยู่แล้ว

ถ้ารัน `git remote add origin ...` แล้วเจอ error ว่า `origin already exists` ให้ใช้:

```bash
git remote set-url origin https://github.com/USERNAME/REPOSITORY.git
```

### GitHub มีไฟล์อยู่ก่อนแล้ว

ถ้า repository บน GitHub มีไฟล์อยู่ก่อน เช่น README หรือ LICENSE ให้ดึงลงมาก่อน:

```bash
git pull origin main --allow-unrelated-histories
```

จากนั้นแก้ conflict ถ้ามี แล้วค่อย commit และ push อีกครั้ง

## 10. ตัวอย่างคำสั่งแบบครบชุด

ใช้สำหรับโปรเจกต์ใหม่ที่ยังไม่เคยอัพขึ้น GitHub:

```bash
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://github.com/USERNAME/REPOSITORY.git
git push -u origin main
```

หลังจากอัพโหลดครั้งแรกแล้ว ครั้งต่อไปใช้แค่:

```bash
git add .
git commit -m "Update project"
git push
```
