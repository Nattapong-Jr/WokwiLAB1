# Lab 3: GPIO Interrupt และ FreeRTOS Queue

## เป้าหมาย

เมื่อกดปุ่มที่ต่อกับ GPIO 4 ให้ hardware interrupt ส่งเหตุการณ์เข้า FreeRTOS Queue แล้วปลุก worker task มานับจำนวนครั้ง

## การต่อสาย

| ESP32 | Pushbutton |
|---|---|
| GPIO 4 | ขาปุ่มด้านหนึ่ง |
| GND | ขาปุ่มอีกด้าน |

โปรแกรมเปิด internal pull-up ไว้ ดังนั้น:

- ไม่ได้กดปุ่ม = GPIO 4 เป็น HIGH
- กดปุ่ม = GPIO 4 ถูกต่อกับ GND และเปลี่ยนเป็น LOW
- interrupt ทำงานที่ขอบสัญญาณขาลง (`GPIO_INTR_NEGEDGE`)

## วิธี Build

เปิดโฟลเดอร์นี้ใน VS Code แล้วกด `F1` จากนั้นเลือก `ESP-IDF: Open ESP-IDF Terminal` แล้วรัน:

```powershell
idf.py set-target esp32
idf.py build
```

เมื่อ build สำเร็จ กด `F1` แล้วเลือก `Wokwi: Start Simulator`

## วิธีทดสอบ

1. รอ Serial Monitor แสดง `Ready. Press the button connected to GPIO 4.`
2. คลิกปุ่มสีแดงบนวงจร
3. Serial Monitor ควรแสดง:

```text
Button pressed: count=1, GPIO=4
Button pressed: count=2, GPIO=4
Button pressed: count=3, GPIO=4
```

## ลำดับการทำงาน

```text
กดปุ่ม
   ↓
GPIO 4 เกิด falling edge
   ↓
button_isr_handler() ทำงานสั้น ๆ ใน interrupt context
   ↓  xQueueSendFromISR()
FreeRTOS Queue
   ↓  xQueueReceive()
button_worker_task() ตื่นขึ้นมา ตรวจ debounce และนับจำนวน
```

## ทำไม ISR ห้ามทำงานหนัก

ISR แทรกการทำงานปกติของ CPU และต้องจบให้เร็ว จึงไม่ควรใช้ `printf()`, `vTaskDelay()` หรือฟังก์ชันที่อาจ block ภายใน ISR งานจริงถูกส่งผ่าน Queue ไปทำใน task แทน

## สิ่งที่เพิ่มจากตัวอย่างในเอกสาร

- ตรวจผลของ `xQueueCreate()` และ `xTaskCreate()`
- ใช้ `portYIELD_FROM_ISR()` เพื่อปลุก task ทันทีเมื่อจำเป็น
- เพิ่ม debounce 150 ms ป้องกันหนึ่งครั้งถูกนับหลายครั้ง
- ใช้ `uintptr_t` ตอนแปลงหมายเลข GPIO เป็น pointer เพื่อหลีกเลี่ยง warning
