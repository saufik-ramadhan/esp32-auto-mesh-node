#include <stdio.h>
#include "app_oled.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"
#include "esp_lcd_panel_vendor.h"
#include "lvgl.h"

static const char *TAG = "APP_OLED";

#define I2C_HOST 0
#define I2C_HW_ADDR 0x3C
#define PIN_NUM_RST -1
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8
#define LCD_H_RES 128
#define LCD_V_RES 64

/* The LVGL port component calls esp_lcd_panel_draw_bitmap API for send data to the screen. There must be called
lvgl_port_flush_ready(disp) after each transaction to display. The best way is to use on_color_trans_done
callback from esp_lcd IO config structure. In IDF 5.1 and higher, it is solved inside LVGL port component. */
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
  lv_disp_t *disp = (lv_disp_t *)user_ctx;
  lvgl_port_flush_ready(disp);
  return false;
}

lv_disp_t *app_oled_lvgl_init(void)
{
  ESP_LOGI(TAG, "Install panel IO");
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_i2c_config_t io_config = {
      .dev_addr = I2C_HW_ADDR,
      .control_phase_bytes = 1,       // According to SSD1306 datasheet
      .lcd_cmd_bits = LCD_CMD_BITS,   // According to SSD1306 datasheet
      .lcd_param_bits = LCD_CMD_BITS, // According to SSD1306 datasheet
      .dc_bit_offset = 6              // According to SSD1306 datasheet
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_HOST, &io_config, &io_handle));

  ESP_LOGI(TAG, "Install SSD1306 panel driver");
  esp_lcd_panel_handle_t panel_handle = NULL;
  esp_lcd_panel_dev_config_t panel_config = {
      .bits_per_pixel = 1,
      .reset_gpio_num = PIN_NUM_RST,
  };

  ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

  ESP_LOGI(TAG, "Initialize LVGL");
  const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
  lvgl_port_init(&lvgl_cfg);

  const lvgl_port_display_cfg_t disp_cfg = {
      .io_handle = io_handle,       // io_handle
      .panel_handle = panel_handle, // panel_handle
      .buffer_size = LCD_H_RES * LCD_V_RES,
      .double_buffer = true,
      .hres = LCD_H_RES,
      .vres = LCD_V_RES,
      .monochrome = true,
      .rotation = {
          .swap_xy = false,
          .mirror_x = false,
          .mirror_y = false,
      }};
  lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);
  /* Register done callback for IO */
  const esp_lcd_panel_io_callbacks_t cbs = {
      .on_color_trans_done = notify_lvgl_flush_ready,
  };
  esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, disp); // (io_handle, ...,...)

  /* Rotation of the screen */
  lv_disp_set_rotation(disp, LV_DISP_ROT_NONE);
  return disp;
};

void app_oled_lvgl_demo(lv_disp_t *disp)
{
  ESP_LOGI(TAG, "Printing DEMO OLED Text!");
  lv_obj_t *scr = lv_disp_get_scr_act(disp);
  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); /* Circular scroll */
  lv_label_set_text(label, ".");
  /* Size of the screen (if you use rotation 90 or 270, please set disp->driver->ver_res) */
  lv_obj_set_width(label, disp->driver->hor_res);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
}

void app_oled_node_info(lv_disp_t *disp, int mesh_layer, bool is_running, bool is_mesh_connected)
{
  char str[100];
  sprintf(str, "Layer: %d, Is Connected: %s", mesh_layer, is_mesh_connected ? "TRUE" : "FALSE");
  lv_obj_t *scr = lv_disp_get_scr_act(disp);
  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); /* Circular scroll */
  lv_label_set_text(label, str);
  /* Size of the screen (if you use rotation 90 or 270, please set disp->driver->ver_res) */
  lv_obj_set_width(label, disp->driver->hor_res);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
}
