# Lab 2: ESP32 ควบคุม LCD 16x2 ผ่าน I2C

## เป้าหมาย

ให้ ESP32 ใช้ไดรเวอร์ I2C Master ของ ESP-IDF ติดต่อจอ LCD 16x2 ที่ address `0x27` และแสดงข้อความจริงบนจอ

```text
ESP32 + ESP-IDF
I2C Lab 2 OK
```

## การต่อสาย

| ESP32 | LCD 16x2 I2C | หน้าที่ |
|---|---|---|
| GPIO 21 | SDA | ส่งและรับข้อมูล |
| GPIO 22 | SCL | สัญญาณนาฬิกา 100 kHz |
| 5V | VCC | จ่ายไฟให้จอ |
| GND | GND | กราวด์ร่วม |

วงจรถูกเตรียมไว้แล้วใน `diagram.json`

## วิธี Build

เปิดโฟลเดอร์นี้ใน VS Code แล้วเปิด `ESP-IDF: Open ESP-IDF Terminal` จากนั้นรัน:

```powershell
idf.py set-target esp32
idf.py build
```

เมื่อ build สำเร็จ กด `F1` แล้วเลือก `Wokwi: Start Simulator`

## ผลลัพธ์ที่ถือว่าผ่าน

1. LCD แสดงข้อความครบทั้งสองบรรทัด
2. Serial Monitor แสดง:

```text
LCD acknowledged at 0x27 and text was written successfully
```

หากโปรแกรมหยุดด้วย error ตอน `i2c_master_probe()` ให้ตรวจสาย SDA/SCL และตรวจว่าจอตั้ง address เป็น `0x27`

## โค้ดทำงานอย่างไร

1. สร้าง I2C bus ที่ GPIO 21 และ GPIO 22
2. ใช้ `i2c_master_probe()` ตรวจว่าจอที่ `0x27` ตอบ ACK
3. เพิ่มจอเป็นอุปกรณ์บน I2C bus ด้วยความเร็ว 100 kHz
4. ตั้ง LCD เป็นโหมด 4-bit และเปิด backlight
5. ส่งตัวอักษรไปยังบรรทัดที่ 1 และ 2

## จุดที่พัฒนาจากเอกสารของอาจารย์

โค้ดในเอกสารส่งเพียง `0x08` เพื่อทดสอบ ACK จึงยังไม่แสดงข้อความบนจอ โปรเจกต์นี้เพิ่มลำดับเริ่มต้นของ HD44780 และฟังก์ชันส่งคำสั่ง/ตัวอักษรครบแล้ว
