#include "Arduino.h"

#include "lvgl.h"
#include "TFT_eSPI.h"

#include "GFX.h"

namespace GFX
{
    TFT_eSPI screen;
    constexpr auto DRAW_BUF_SIZE = TFT_WIDTH * TFT_HEIGHT / 10 * (LV_COLOR_DEPTH / 8);
    uint32_t draw_buf[DRAW_BUF_SIZE / 4];

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

    void init()
    {
        screen.begin();
        screen.setRotation(1);
        screen.setSwapBytes(true);
        uint16_t calData[5] = {383, 3506, 311, 3408, 3};
        screen.setTouch(calData);

        lv_init();
        lv_tick_set_cb(millis);

        auto disp = lv_display_create(TFT_HEIGHT, TFT_WIDTH);
        lv_display_set_flush_cb(disp, flushScreen);
        lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

        auto indev = lv_indev_create();
        lv_indev_set_type(
            indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
        lv_indev_set_read_cb(indev, readTouch);
    }
} // namespace GFX
