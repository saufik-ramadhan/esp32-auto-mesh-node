#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "app_i2c.h"

static const char *TAG = "APP_I2C";

#define I2C_HOST 0
#define LCD_PIXEL_CLOCK_HZ (400 * 1000)
#define PIN_NUM_SDA GPIO_NUM_21
#define PIN_NUM_SCL GPIO_NUM_22

void app_i2c_init(void)
{
  ESP_LOGI(TAG, "Initialize I2C bus");
  i2c_config_t i2c_conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = PIN_NUM_SDA,
      .scl_io_num = PIN_NUM_SCL,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = LCD_PIXEL_CLOCK_HZ,
  };
  ESP_ERROR_CHECK(i2c_param_config(I2C_HOST, &i2c_conf));
  ESP_ERROR_CHECK(i2c_driver_install(I2C_HOST, I2C_MODE_MASTER, 0, 0, 0));
}
