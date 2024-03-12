#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

lv_disp_t *app_oled_lvgl_init(void);
void app_oled_lvgl_demo(lv_disp_t *disp);
void app_oled_node_info(lv_disp_t *disp, int mesh_layer, bool is_running, bool is_mesh_connected);
