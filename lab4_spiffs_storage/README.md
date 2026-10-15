# Lab 4: บันทึกและอ่านไฟล์ด้วย SPIFFS

## เป้าหมาย

ให้ ESP32 สร้าง filesystem ใน flash, เขียนข้อมูลลง `log.txt`, อ่านกลับมาเปรียบเทียบ และรายงานผล `PASS` ผ่าน Serial Monitor

## ไฟล์สำคัญ

- `partitions.csv` แบ่งพื้นที่ flash และสร้าง partition ชื่อ `storage` ขนาด 960 KB ซึ่งพอดีกับ flash 2 MB
- `sdkconfig.defaults` บอก ESP-IDF ให้ใช้ partition table ดังกล่าว
- `main/main.c` mount, write, read, compare และ unmount SPIFFS
- `wokwi.toml` และ `diagram.json` ใช้จำลองบน Wokwi

## วิธี Build

เปิดโฟลเดอร์นี้ใน VS Code แล้วกด `F1` จากนั้นเลือก `ESP-IDF: Open ESP-IDF Terminal` แล้วรัน:

```powershell
idf.py set-target esp32
idf.py build
```

เมื่อ build สำเร็จ กด `F1` แล้วเลือก `Wokwi: Start Simulator`

## ผลลัพธ์ที่ถือว่าผ่าน

Serial Monitor ต้องมีข้อความลักษณะนี้:

```text
Mounting SPIFFS partition 'storage' at /spiffs
Wrote 48 characters to /spiffs/log.txt
Read back: Sensor Log Entry #104: Temp=24.5C, Humidity=60%
PASS: SPIFFS readback matches the written data
SPIFFS unmounted cleanly
```

จำนวน byte หรือจำนวนตัวอักษรอาจต่างได้เล็กน้อยหากแก้ข้อความ แต่ต้องมีบรรทัด `PASS`

## ลำดับการทำงาน

```text
partition storage
       ↓
esp_vfs_spiffs_register()
       ↓
fopen("w") → fprintf() → fclose()
       ↓
fopen("r") → fgets() → fclose()
       ↓
strcmp() ตรวจว่าข้อมูลเหมือนเดิม
       ↓
esp_vfs_spiffs_unregister()
```

## จุดที่แก้จากเอกสารของอาจารย์

- แก้ string literal ที่ขึ้นบรรทัดใหม่ผิดตำแหน่งและทำให้คอมไพล์ไม่ผ่าน
- ระบุ partition label เป็น `storage` ให้ตรงกับ `partitions.csv`
- เพิ่ม `sdkconfig.defaults` เพื่อให้ custom partition table ถูกใช้จริง
- ตรวจผลของการเขียน ปิดไฟล์ อ่านกลับ และเปรียบเทียบข้อมูล
- unmount filesystem ในทุกเส้นทางที่เปิดใช้งานสำเร็จ

## ถ้าขึ้นว่าไม่พบ partition

หากเห็น `Partition 'storage' was not found` ให้ลบโฟลเดอร์ `build` แล้ว build ใหม่ เพราะไฟล์ partition table เดิมอาจถูก cache ไว้:

```powershell
idf.py fullclean
idf.py build
```

## ถ้า build แจ้งว่า partition ใช้พื้นที่เกิน flash 2 MB

ไฟล์ชุดนี้ตั้ง `storage` เป็น `0xF0000` (960 KB) แล้ว ทำให้ partition ทั้งหมดจบพอดีที่ตำแหน่ง `0x200000` ของ flash 2 MB หลังอัปเดตไฟล์ให้ล้าง build เดิมก่อน:

```powershell
idf.py fullclean
idf.py build
```
