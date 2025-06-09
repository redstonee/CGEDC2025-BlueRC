#include "Arduino.h"

#include "lvgl.h"
#include "TFT_eSPI.h"

#include "GFX.h"
#include "tabs/BatteryTab.hpp"
#include "tabs/DevicesTab.hpp"
#include "config.h"

namespace GFX
{
    static TFT_eSPI screen;
    constexpr auto DRAW_BUF_SIZE = TFT_WIDTH * TFT_HEIGHT / 10 * (LV_COLOR_DEPTH / 8);

    static void flushScreen(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
    {
        uint32_t w = lv_area_get_width(area);
        uint32_t h = lv_area_get_height(area);

        screen.startWrite();
        screen.setAddrWindow(area->x1, area->y1, w, h);
        screen.pushPixels((uint16_t *)px_map, w * h);
        screen.endWrite();

        lv_disp_flush_ready(disp);
    }

    static void readTouch(lv_indev_t *indev, lv_indev_data_t *data)
    {
        uint16_t touchX, touchY;
        if (!screen.getTouch(&touchX, &touchY, 600))
        {
            data->state = LV_INDEV_STATE_REL;
        }
        else
        {
            data->state = LV_INDEV_STATE_PR;
            data->point.x = touchX;
            data->point.y = touchY;
        }
    }

    static void printLog(lv_log_level_t level, const char *buf)
    {
        constexpr auto TAG = "LVGL";
        switch (level)
        {
        case LV_LOG_LEVEL_TRACE:
            ESP_LOGD(TAG, "%s", buf);
            break;
        case LV_LOG_LEVEL_INFO:
            ESP_LOGI(TAG, "%s", buf);
            break;
        case LV_LOG_LEVEL_WARN:
            ESP_LOGW(TAG, "%s", buf);
            break;
        case LV_LOG_LEVEL_ERROR:
            ESP_LOGE(TAG, "%s", buf);
            break;
        case LV_LOG_LEVEL_USER:
            ESP_LOGI(TAG, "%s", buf);
            break;
        }
    }

    void lvglTask(void *arg)
    {
        auto tabs = static_cast<Tab **>(arg);
        uint8_t nTabs = 0;
        while (tabs[nTabs]) // Initialize all tabs before starting the loop
        {
            tabs[nTabs++]->initTab();
        }

        while (true)
        {
            for (auto i = 0; i < nTabs; i++)
            {
                tabs[i]->updateTab(); // Update each tab
            }
            delay(lv_timer_handler());
        }
    }

    void init()
    {
        // Initialize the TFT display and LVGL
        screen.init();
        screen.setRotation(3);
        screen.setSwapBytes(true);
        uint16_t calData[5] = {378, 3550, 296, 3491, 5};
        screen.setTouch(calData);

        pinMode(LCD_BL_PIN, OUTPUT);
        digitalWrite(LCD_BL_PIN, HIGH); // Turn on backlight

        lv_init();
        lv_tick_set_cb(millis);
        lv_log_register_print_cb(printLog); // Register the log callback

        auto disp = lv_display_create(TFT_HEIGHT, TFT_WIDTH);
        static uint32_t draw_buf[DRAW_BUF_SIZE / 4];
        lv_display_set_flush_cb(disp, flushScreen);
        lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

        auto indev = lv_indev_create();
        lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev, readTouch);

        lv_display_set_theme(nullptr, lv_theme_default_init(nullptr,
                                                            lv_palette_main(LV_PALETTE_BLUE),
                                                            lv_palette_main(LV_PALETTE_RED),
                                                            false, LV_FONT_DEFAULT));

        // Build the tab view
        auto tabview = lv_tabview_create(lv_screen_active());
        lv_tabview_set_tab_bar_position(tabview, LV_DIR_LEFT);
        lv_tabview_set_tab_bar_size(tabview, 80);

        // Create the battery tab
        auto batteryTab = new BatteryTab(tabview);
        auto devicesTab = new DevicesTab(tabview);

        static Tab *tabs[]{batteryTab, devicesTab, nullptr};

        xTaskCreate(lvglTask, "lvglTask", 8192, tabs, 5, NULL); /*Create a task to handle LVGL events*/
    }

} // namespace GFX
