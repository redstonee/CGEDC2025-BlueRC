#pragma once

#include "tabs/ControlTab.h"
#include <vector>

extern "C"
{
    LV_FONT_DECLARE(mode_logo_20)
}

static constexpr auto LOGO_SUN = "\xEF\x86\x85";
static constexpr auto LOGO_ICE = "\xEF\x8B\x9C";
static constexpr auto LOGO_DRY = "\xEF\x81\x83";
static constexpr auto LOGO_WIND = "\xEF\x9C\xAE";
static constexpr auto LOGO_OFF = "\xEF\x80\x91";

static constexpr const char *logos[] = {
    LOGO_OFF,
    LOGO_SUN,
    LOGO_ICE,
    LOGO_DRY,
    LOGO_WIND,
};

constexpr auto LOGO_CFG = "\xEF\x87\x9E";

static void checkboxEventHandler(lv_event_t *e)
{
    auto checkbox = lv_event_get_target_obj(e);
    auto params = static_cast<ControlTab::Device *>(lv_event_get_user_data(e));

    params->selected = (lv_obj_get_state(checkbox) == LV_STATE_CHECKED);
}

ControlTab::ControlTab(lv_obj_t *parent)
    : Tab(parent, "Devices") {};

void ControlTab::initTab()
{
    // Test data for the device list
    for (auto i = 0; i < 5; i++)
    {
        Device dev;
        dev.name = "AC " + String(i + 1);
        dev.temperature = 20 + i;                  // Example temperature
        dev.mode = static_cast<DeviceMode>(i % 5); // Example mode
        _deviceList.push_back(dev);
    }

    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    // The style for the tab
    static lv_style_t tabStyle;
    lv_style_init(&tabStyle);
    lv_style_set_flex_main_place(&tabStyle, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_style_set_flex_flow(&tabStyle, LV_FLEX_FLOW_COLUMN);
    lv_style_set_layout(&tabStyle, LV_LAYOUT_FLEX);
    lv_obj_add_style(root, &tabStyle, 0);

    // The style for horizon layout items
    static lv_style_t horizonItemStyle;
    lv_style_init(&horizonItemStyle);
    lv_style_set_flex_main_place(&horizonItemStyle, LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_style_set_flex_flow(&horizonItemStyle, LV_FLEX_FLOW_ROW);
    lv_style_set_layout(&horizonItemStyle, LV_LAYOUT_FLEX);

    // Create a flex layout for the title and configuration button
    auto appBar = lv_obj_create(root);
    lv_obj_remove_style_all(appBar);
    lv_obj_set_size(appBar, lv_pct(100), 28);
    lv_obj_add_style(appBar, &horizonItemStyle, 0);

    // Layout title label
    auto *titleLabel = lv_label_create(appBar);
    lv_label_set_text(titleLabel, "Devices");
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_24, 0);

    // Configuration button
    auto configButton = lv_button_create(appBar);
    lv_obj_set_size(configButton, 26, 26);
    auto buttonLabel = lv_label_create(configButton);
    lv_obj_set_style_text_font(buttonLabel, &mode_logo_20, 0);
    lv_label_set_text(buttonLabel, LOGO_CFG);
    lv_obj_center(buttonLabel);

    // The list view for displaying devices
    auto devListView = lv_list_create(root);
    lv_obj_set_size(devListView, lv_pct(100), lv_pct(80));
    lv_obj_set_style_pad_row(devListView, 3, 0);

    // Add list items into the list view
    for (auto &dev : _deviceList)
    {
        // Create a flex layout for each device item
        auto listItem = lv_obj_create(devListView);
        lv_obj_remove_style_all(listItem);
        lv_obj_set_size(listItem, lv_pct(100), 40);
        lv_obj_add_style(listItem, &horizonItemStyle, 0);

        auto nameLabel = lv_label_create(listItem);
        lv_label_set_text(nameLabel, dev.name.c_str());
        lv_obj_set_width(nameLabel, lv_pct(20));
        lv_label_set_long_mode(nameLabel, LV_LABEL_LONG_MODE_SCROLL);

        auto modeLabel = lv_label_create(listItem);
        lv_label_set_text(modeLabel, logos[static_cast<int>(dev.mode)]);
        lv_obj_set_style_text_font(modeLabel, &mode_logo_20, 0);

        auto tempLabel = lv_label_create(listItem);
        if (dev.mode == DeviceMode::OFF)
            lv_label_set_text(tempLabel, " Off ");
        else
            lv_label_set_text_fmt(tempLabel, "%u °C", dev.temperature);

        auto checkbox = lv_checkbox_create(listItem);
        lv_obj_add_event_cb(checkbox, checkboxEventHandler, LV_EVENT_VALUE_CHANGED, &dev);
        lv_checkbox_set_text(checkbox, "");
    }
}

void ControlTab::updateTab()
{
    // Update device list or UI elements if needed
}
