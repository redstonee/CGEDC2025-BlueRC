#pragma once

#include <tuple>
#include "Tab.hpp"

// WARNING: This is a global variable, use with CAUTION!
extern QueueHandle_t batteryInfoQueue;

class BatteryTab : public Tab
{
private:
    static constexpr auto TAG = "BatteryTab";

    lv_obj_t *batteryLevelBar;   // Bar to show battery level
    lv_obj_t *batteryLevelLabel; // Label to show battery level percentage
    lv_obj_t *remainTimeLabel;   // Label to show estimated remaining time
    lv_obj_t *voltageLabel;      // Label to show battery voltage
    lv_obj_t *currentLabel;      // Label to show battery current, negative if discharging

public:
    BatteryTab(lv_obj_t *parent)
        : Tab(parent, "Battery") {}

    /**
     * @brief Initialize the Battery tab.
     */
    void initTab() override
    {
        // style the tab
        static lv_style_t tabStyle;
        lv_style_init(&tabStyle);
        lv_style_set_flex_main_place(&tabStyle, LV_FLEX_ALIGN_SPACE_EVENLY);
        lv_style_set_flex_flow(&tabStyle, LV_FLEX_FLOW_COLUMN);
        lv_style_set_layout(&tabStyle, LV_LAYOUT_FLEX);
        lv_obj_add_style(root, &tabStyle, 0);

        auto *titleLabel = lv_label_create(root);
        lv_label_set_text(titleLabel, "Battery Status");
        lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_24, 0);

        // style for the small horizontal layout
        static lv_style_t smallLayoutStyle;
        lv_style_init(&smallLayoutStyle);
        lv_style_set_flex_main_place(&smallLayoutStyle, LV_FLEX_ALIGN_SPACE_BETWEEN);
        lv_style_set_flex_flow(&smallLayoutStyle, LV_FLEX_FLOW_ROW);
        lv_style_set_layout(&smallLayoutStyle, LV_LAYOUT_FLEX);

        // Battery level information layout
        auto battLevelLayout = lv_obj_create(root);
        lv_obj_remove_style_all(battLevelLayout);
        lv_obj_set_size(battLevelLayout, lv_pct(100), 30);
        lv_obj_add_style(battLevelLayout, &smallLayoutStyle, 0);
        batteryLevelBar = lv_bar_create(battLevelLayout);
        lv_obj_set_width(batteryLevelBar, lv_pct(70));
        lv_bar_set_range(batteryLevelBar, 0, 100);
        batteryLevelLabel = lv_label_create(battLevelLayout);
        lv_label_set_text(batteryLevelLabel, "69 %%");

        auto remainTimeLayout = lv_obj_create(root);
        lv_obj_remove_style_all(remainTimeLayout);
        lv_obj_set_size(remainTimeLayout, lv_pct(100), 30);
        lv_obj_add_style(remainTimeLayout, &smallLayoutStyle, 0);
        auto remainTimeHint = lv_label_create(remainTimeLayout);
        lv_label_set_text(remainTimeHint, "Remain Time:");
        remainTimeLabel = lv_label_create(remainTimeLayout);
        lv_label_set_text(remainTimeLabel, "114 d 514 h");

        // Voltage information layout
        auto voltageLayout = lv_obj_create(root);
        lv_obj_remove_style_all(voltageLayout);
        lv_obj_set_size(voltageLayout, lv_pct(100), 30);
        lv_obj_add_style(voltageLayout, &smallLayoutStyle, 0);
        auto voltHint = lv_label_create(voltageLayout);
        lv_label_set_text(voltHint, "Voltage:");
        voltageLabel = lv_label_create(voltageLayout);
        lv_label_set_text(voltageLabel, "1919 mV");

        // Current information layout
        auto currentLayout = lv_obj_create(root);
        lv_obj_remove_style_all(currentLayout);
        lv_obj_set_size(currentLayout, lv_pct(100), 30);
        lv_obj_add_style(currentLayout, &smallLayoutStyle, 0);
        auto currHint = lv_label_create(currentLayout);
        lv_label_set_text(currHint, "Current:");
        currentLabel = lv_label_create(currentLayout);
        lv_label_set_text(currentLabel, "810 mA");
    }

    void updateTab() override
    {
        std::tuple<int, int, uint8_t> batteryInfo;
        if (xQueueReceive(batteryInfoQueue, &batteryInfo, 0))
        {
            auto [voltage, current, level] = batteryInfo;
            lv_label_set_text_fmt(voltageLabel, "%d mV", voltage);
            lv_label_set_text_fmt(currentLabel, "%d mA", current);
            lv_bar_set_value(batteryLevelBar, level, LV_ANIM_ON);
            lv_label_set_text_fmt(batteryLevelLabel, "%d %%", level);

            auto remainPower = BATT_CAP * level / 100.0f;                         // Remaining power in mAh
            auto remainHours = static_cast<uint32_t>(remainPower / TYP_PWR_CONS); // Remaining hours based on typical power consumption
            if (remainHours > 24)
            {
                lv_label_set_text_fmt(remainTimeLabel, "%d d %d h", remainHours / 24, remainHours % 24);
            }
            else
            {
                lv_label_set_text_fmt(remainTimeLabel, "%d h", remainHours);
            }
        }
    }
};
