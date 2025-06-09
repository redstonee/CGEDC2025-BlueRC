#pragma once

#include "Tab.hpp"
#include "Arduino.h"
#include "AXP173.h"

// WARNING: This is a global variable, use with CAUTION!
extern QueueHandle_t batteryInfoQueue;

class BatteryTab : public Tab
{
private:
    static constexpr auto TAG = "BatteryTab";

    lv_obj_t *batteryLevelBar;
    lv_obj_t *batteryLevelLabel;

    lv_obj_t *voltageLabel;
    lv_obj_t *currentLabel;

public:
    BatteryTab(lv_obj_t *parent)
        : Tab(parent, "Battery") {};

    void initTab() override
    {
        lv_obj_set_size(root, lv_pct(100), lv_pct(100));
        static lv_style_t tabStyle;
        lv_style_init(&tabStyle);
        lv_style_set_flex_main_place(&tabStyle, LV_FLEX_ALIGN_SPACE_EVENLY);
        lv_style_set_flex_flow(&tabStyle, LV_FLEX_FLOW_COLUMN);
        lv_style_set_layout(&tabStyle, LV_LAYOUT_FLEX);
        lv_obj_add_style(root, &tabStyle, 0);

        auto *label = lv_label_create(root);
        lv_label_set_text(label, "Battery Status");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);

        static lv_style_t smallLayoutStyle;
        lv_style_init(&smallLayoutStyle);
        lv_style_set_flex_main_place(&smallLayoutStyle, LV_FLEX_ALIGN_SPACE_BETWEEN);
        lv_style_set_flex_flow(&smallLayoutStyle, LV_FLEX_FLOW_ROW);
        lv_style_set_layout(&smallLayoutStyle, LV_LAYOUT_FLEX);

        auto battLevelLayout = lv_obj_create(root);
        lv_obj_remove_style_all(battLevelLayout);
        lv_obj_set_size(battLevelLayout, lv_pct(100), 30);
        lv_obj_add_style(battLevelLayout, &smallLayoutStyle, 0);
        batteryLevelBar = lv_bar_create(battLevelLayout);
        lv_obj_set_width(batteryLevelBar, lv_pct(60));
        lv_bar_set_range(batteryLevelBar, 3000, 4200);
        batteryLevelLabel = lv_label_create(battLevelLayout);
        lv_label_set_text(batteryLevelLabel, "0 %%");

        auto voltageLayout = lv_obj_create(root);
        lv_obj_remove_style_all(voltageLayout);
        lv_obj_set_size(voltageLayout, lv_pct(100), 30);
        lv_obj_add_style(voltageLayout, &smallLayoutStyle, 0);
        auto voltHint = lv_label_create(voltageLayout);
        lv_label_set_text(voltHint, "Voltage:");
        voltageLabel = lv_label_create(voltageLayout);
        lv_label_set_text(voltageLabel, "0 mV");

        auto currentLayout = lv_obj_create(root);
        lv_obj_remove_style_all(currentLayout);
        lv_obj_set_size(currentLayout, lv_pct(100), 30);
        lv_obj_add_style(currentLayout, &smallLayoutStyle, 0);
        auto currHint = lv_label_create(currentLayout);
        lv_label_set_text(currHint, "Current:");
        currentLabel = lv_label_create(currentLayout);
        lv_obj_set_width(currentLabel, lv_pct(35));
        lv_label_set_text(currentLabel, "0 mA");
    }

    void updateTab() override
    {
        std::tuple<int, int, uint8_t> batteryInfo;
        if (xQueueReceive(batteryInfoQueue, &batteryInfo, 0))
        {
            auto [voltage, current, level] = batteryInfo;

            lv_label_set_text_fmt(voltageLabel, "%d mV", voltage);

            lv_label_set_text_fmt(currentLabel, "%d mA", current);

            lv_bar_set_value(batteryLevelBar, current, LV_ANIM_ON);
            lv_label_set_text_fmt(batteryLevelLabel, "%d %%", level);
        }
    }
};
