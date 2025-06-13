#include "Arduino.h"

#include "lvgl.h"
#include "TFT_eSPI.h"

#include "GFX.h"
#include "tabs/BatteryTab.hpp"
#include "tabs/ControlTab.h"
#include "tabs/ParingTab.hpp"
#include "config.h"

//TODO: Use ESP_LCD components which supports DMA

namespace GFX
{
    static TFT_eSPI screen;
    constexpr auto DRAW_BUF_SIZE = TFT_WIDTH * TFT_HEIGHT / 10 * (LV_COLOR_DEPTH / 8);

    static uint32_t lastTouchTime;

    uint32_t getLastTouchTime()
    {
        return lastTouchTime;
    }

    /**
     * @brief Send the pixel map to the display.
     *
     * @param disp LVGL display object
     * @param area Display area to flush
     * @param px_map Pointer to the pixel map to be displayed
     *
     * @note This function is called by LVGL to flush the pixel map to the display.
     */
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

    /**
     * @brief Read touch input from the display.
     *
     * @param indev Pointer to the input device
     * @param data Pointer to the input data structure to fill
     *
     * @note This function is called by LVGL to read touch input.
     */
    static void readTouch(lv_indev_t *indev, lv_indev_data_t *data)
    {
        uint16_t touchX, touchY;
        if (!screen.getTouch(&touchX, &touchY, 600))
        {
            data->state = LV_INDEV_STATE_REL;
        }
        else
        {
            lastTouchTime = millis();
            data->state = LV_INDEV_STATE_PR;
            data->point.x = touchX;
            data->point.y = touchY;
        }
    }

    /**
     * @brief Print log messages from LVGL to the ESP-IDF log system.
     *
     * @param level The log level of the message
     * @param buf The message buffer containing the log message
     *
     * @note This function is called by LVGL to print log messages.
     */
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

    /**
     * @brief Task to handle LVGL events and update the display.
     *
     * @param arg Pointer to the array of Tab pointers
     */
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

    void onTabChanged(lv_event_t *e)
    {
        auto tabview = lv_event_get_target_obj(e);
        auto tabIndex = lv_tabview_get_tab_active(tabview);
        ESP_LOGI("GFX", "Tab changed to: %lu", tabIndex);
    }

    /**
     * @brief Initialize the TFT display and LVGL.
     * This function sets up the display, initializes LVGL, and creates the tab view.
     */
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
        lv_obj_add_event_cb(tabview, onTabChanged, LV_EVENT_VALUE_CHANGED, nullptr);
        lv_obj_remove_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);

        // Create the battery tab
        auto controlTab = new ControlTab(tabview);
        auto pairingTab = new Pairing(tabview);
        auto batteryTab = new BatteryTab(tabview);

        static Tab *tabs[]{controlTab, pairingTab, batteryTab, nullptr};

        xTaskCreate(lvglTask, "lvglTask", 8192, tabs, 5, NULL); /*Create a task to handle LVGL events*/
    }

} // namespace GFX
