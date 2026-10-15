# Lab 1: ESP32 เชื่อมต่อ Wi-Fi บน Wokwi

## เป้าหมาย

ให้ ESP32 เชื่อมต่อ Access Point จำลองชื่อ `Wokwi-GUEST` และรับ IP Address ผ่าน DHCP

## วิธีทดลองบนเว็บไซต์ Wokwi

1. เปิด https://wokwi.com/projects/new/choose-board/esp32
2. เลือก **ESP-IDF** ในหัวข้อ Advanced templates
3. เปิดแท็บ `main.c` แล้วแทนที่โค้ดเดิมด้วยเนื้อหาจาก `main/main.c`
4. เปิดแท็บ `diagram.json` แล้วแทนที่ด้วยไฟล์ `diagram.json` ชุดนี้
5. กดปุ่ม ▶ เพื่อเริ่มจำลอง

ผลที่ถือว่าผ่านใน Serial Monitor:

```text
Connected! IP address: 10.x.x.x
Lab 1 complete: the ESP32 is online.
```

หมายเลข IP อาจไม่เหมือนตัวอย่าง แต่ต้องมีข้อความ `Connected! IP address:`

## วิธีทดลองด้วย VS Code และ ESP-IDF

1. เปิดโฟลเดอร์นี้ด้วย VS Code ที่ติดตั้ง ESP-IDF Extension แล้ว
2. เปิด ESP-IDF Terminal
3. สั่ง `idf.py set-target esp32`
4. สั่ง `idf.py build`
5. กด F1 แล้วเลือก `Wokwi: Start Simulator`

## ลำดับการทำงานของโปรแกรม

1. `nvs_flash_init()` เตรียมพื้นที่เก็บค่าระบบ Wi-Fi
2. `esp_netif_init()` เตรียมระบบเครือข่าย
3. สร้าง Event Loop เพื่อรับเหตุการณ์จาก Wi-Fi
4. ตั้งค่า ESP32 เป็น Station Mode
5. เชื่อมต่อ `Wokwi-GUEST`
6. เมื่อได้รับ IP จะปลดล็อก Event Group และพิมพ์ข้อความว่าผ่าน

## คำถามที่มักถูกถาม

**ทำไมต้องเรียก NVS ก่อนเปิด Wi-Fi?**  
เพราะไดรเวอร์ Wi-Fi ใช้ NVS เก็บและอ่านค่าที่เกี่ยวกับการทำงานของระบบวิทยุและการตั้งค่าเครือข่าย

**ทำไมไม่ใส่รหัสผ่าน?**  
เพราะ `Wokwi-GUEST` เป็น Access Point จำลองแบบเปิดของ Wokwi

**Event Group ใช้ทำอะไร?**  
ใช้ให้ `app_main()` รอจนกว่าจะเกิดเหตุการณ์ว่า ESP32 ได้รับ IP แล้ว โดยไม่ต้องวนตรวจสถานะตลอดเวลา
