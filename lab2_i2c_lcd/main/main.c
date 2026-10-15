#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_PORT          I2C_NUM_0
#define I2C_SDA_PIN       GPIO_NUM_21
#define I2C_SCL_PIN       GPIO_NUM_22
#define I2C_FREQUENCY_HZ  100000
#define LCD_I2C_ADDRESS   0x27

// PCF8574 to HD44780 pin mapping used by the Wokwi I2C LCD:
// P0=RS, P1=RW, P2=EN, P3=Backlight, P4..P7=D4..D7
#define LCD_RS            0x01
#define LCD_ENABLE        0x04
#define LCD_BACKLIGHT     0x08

static const char *TAG = "I2C_LCD_LAB";
static i2c_master_dev_handle_t lcd_device;

static void lcd_write_expander(uint8_t value)
{
    ESP_ERROR_CHECK(i2c_master_transmit(lcd_device, &value, 1, 1000));
}

static void lcd_pulse_enable(uint8_t value)
{
    lcd_write_expander(value | LCD_ENABLE);
    esp_rom_delay_us(1);
    lcd_write_expander(value & (uint8_t)~LCD_ENABLE);
    esp_rom_delay_us(50);
}

static void lcd_send_nibble(uint8_t nibble, uint8_t mode)
{
    uint8_t value = (uint8_t)((nibble & 0x0f) << 4);
    value |= mode | LCD_BACKLIGHT;
    lcd_pulse_enable(value);
}

static void lcd_send_byte(uint8_t value, uint8_t mode)
{
    lcd_send_nibble((uint8_t)(value >> 4), mode);
    lcd_send_nibble((uint8_t)(value & 0x0f), mode);
}

static void lcd_command(uint8_t command)
{
    lcd_send_byte(command, 0);
}

static void lcd_write_char(char character)
{
    lcd_send_byte((uint8_t)character, LCD_RS);
}

static void lcd_print(const char *text)
{
    while (*text != '\0') {
        lcd_write_char(*text++);
    }
}

static void lcd_set_cursor(uint8_t column, uint8_t row)
{
    static const uint8_t row_offsets[] = {0x00, 0x40};
    lcd_command((uint8_t)(0x80 | (row_offsets[row] + column)));
}

static void lcd_initialize(void)
{
    // HD44780 power-on sequence for 4-bit mode.
    vTaskDelay(pdMS_TO_TICKS(50));
    lcd_send_nibble(0x03, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_send_nibble(0x03, 0);
    esp_rom_delay_us(150);
    lcd_send_nibble(0x03, 0);
    lcd_send_nibble(0x02, 0);

    lcd_command(0x28); // 4-bit, 2 lines, 5x8 font
    lcd_command(0x0c); // Display on, cursor off
    lcd_command(0x06); // Move cursor to the right
    lcd_command(0x01); // Clear display
    vTaskDelay(pdMS_TO_TICKS(2));
}

void app_main(void)
{
    ESP_LOGI(TAG, "Creating I2C master bus on SDA=GPIO21, SCL=GPIO22");

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_SCL_PIN,
        .sda_io_num = I2C_SDA_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    ESP_LOGI(TAG, "Checking LCD at I2C address 0x%02X", LCD_I2C_ADDRESS);
    ESP_ERROR_CHECK(i2c_master_probe(bus_handle, LCD_I2C_ADDRESS, 1000));

    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LCD_I2C_ADDRESS,
        .scl_speed_hz = I2C_FREQUENCY_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(
        bus_handle, &device_config, &lcd_device));

    lcd_initialize();
    lcd_set_cursor(0, 0);
    lcd_print("ESP32 + ESP-IDF");
    lcd_set_cursor(0, 1);
    lcd_print("I2C Lab 2 OK");

    ESP_LOGI(TAG, "LCD acknowledged at 0x27 and text was written successfully");
}
