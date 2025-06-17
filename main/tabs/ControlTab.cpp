#include "tabs/ControlTab.h"
#include <vector>
#include <utility>

#include "esp_log.h"

extern "C"
{
    LV_FONT_DECLARE(mode_logo_20)
}

constexpr auto LOGO_SUN = "\xEF\x86\x85";
constexpr auto LOGO_ICE = "\xEF\x8B\x9C";
constexpr auto LOGO_DRY = "\xEF\x81\x83";
constexpr auto LOGO_WIND = "\xEF\x9C\xAE";
constexpr auto LOGO_OFF = "\xEF\x80\x91";

constexpr const char *logos[] = {
    LOGO_OFF,
    LOGO_SUN,
    LOGO_ICE,
    LOGO_DRY,
    LOGO_WIND,
};

constexpr auto LOGO_CFG = "\xEF\x87\x9E";

// WARNING: Global variable, use with CAUTION!
extern QueueHandle_t deviceControlQueue;

ControlTab::ControlTab(lv_obj_t *parent)
    : Tab(parent, "Devices") {};

/**
 * @brief Initialize the Control tab.
 */
void ControlTab::initTab()
{
    // Test data for the device list
    for (auto i = 0; i < 5; i++)
    {
        Device dev;
        dev.name = new char[8];
        snprintf(dev.name, 8, "AC %d", i + 1);
        dev.temperature = 20 + i;                          // Example temperature
        dev.mode = static_cast<Device::DeviceMode>(i % 5); // Example mode
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
    lv_style_init(&horizonItemStyle);
    lv_style_set_flex_main_place(&horizonItemStyle, LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_style_set_flex_cross_place(&horizonItemStyle, LV_FLEX_ALIGN_CENTER);
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
    lv_obj_add_event_cb(configButton, configButtonHandler, LV_EVENT_CLICKED, this);

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

        // Add the information labels and checkbox to the item
        auto nameLabel = lv_label_create(listItem);
        lv_label_set_text(nameLabel, dev.name);
        lv_obj_set_width(nameLabel, lv_pct(20));
        lv_label_set_long_mode(nameLabel, LV_LABEL_LONG_MODE_SCROLL);
        auto modeLabel = lv_label_create(listItem);
        lv_label_set_text(modeLabel, logos[static_cast<int>(dev.mode)]);
        lv_obj_set_style_text_font(modeLabel, &mode_logo_20, 0);
        auto tempLabel = lv_label_create(listItem);
        if (dev.mode == Device::MODE_OFF)
            lv_label_set_text(tempLabel, " Off ");
        else
            lv_label_set_text_fmt(tempLabel, "%u °C", dev.temperature);

        auto checkbox = lv_checkbox_create(listItem);
        lv_obj_add_event_cb(checkbox, checkboxEventHandler, LV_EVENT_VALUE_CHANGED, &dev);
        lv_checkbox_set_text(checkbox, "");
    }
}

/**
 * @brief Event handler for the mode dropdown change event.
 *
 * This function updates the `selected` state of the device based on the selected mode.
 *
 * @param e The LVGL event data
 */
void ControlTab::checkboxEventHandler(lv_event_t *e)
{
    auto checkbox = lv_event_get_target_obj(e);
    LV_ASSERT_OBJ(checkbox, &lv_checkbox_class);

    auto deviceTouched = static_cast<Device *>(lv_event_get_user_data(e));
    deviceTouched->selected = (lv_obj_get_state(checkbox) & LV_STATE_CHECKED) ? true : false;
}

/**
 * @brief Event handler for the configuration button click event.
 *
 * This function creates a message box for configuring selected devices.
 *
 * @param e The LVGL event data
 */
void ControlTab::configButtonHandler(lv_event_t *e)
{
    auto controlTab = static_cast<ControlTab *>(lv_event_get_user_data(e));

    auto controlMessageBox = lv_msgbox_create(nullptr);
    lv_msgbox_add_title(controlMessageBox, "Configuration");
    lv_msgbox_add_close_button(controlMessageBox);

    std::vector<Device> selectedDevices;
    for (auto &dev : controlTab->_deviceList)
    {
        if (dev.selected)
        {
            selectedDevices.push_back(dev);
        }
    }
    if (selectedDevices.empty())
    {
        lv_msgbox_add_text(controlMessageBox, "No devices selected for configuration.");
        return;
    }

    lv_obj_set_size(controlMessageBox, lv_pct(85), lv_pct(85));
    auto content = lv_msgbox_get_content(controlMessageBox);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Mode selection
    auto modeLayout = lv_obj_create(content);
    lv_obj_set_size(modeLayout, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_add_style(modeLayout, &horizonItemStyle, 0);
    auto modeHintLabel = lv_label_create(modeLayout);
    lv_label_set_text(modeHintLabel, "Mode:");
    static const char *modeDropdownOptions = "Off\nHeat\nCool\nDry\nFan";
    auto modeDropdown = lv_dropdown_create(modeLayout);
    lv_dropdown_set_options_static(modeDropdown, modeDropdownOptions);

    // Temperature input
    auto tempLayout = lv_obj_create(content);
    lv_obj_set_size(tempLayout, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_add_style(tempLayout, &horizonItemStyle, 0);
    auto tempHintLabel = lv_label_create(tempLayout);
    lv_label_set_text(tempHintLabel, "Temp:");
    auto tempSlider = lv_slider_create(tempLayout);
    lv_obj_set_width(tempSlider, lv_pct(50));
    lv_slider_set_range(tempSlider, 15, 35);          // Temperature range
    lv_slider_set_value(tempSlider, 25, LV_ANIM_OFF); // Default temperature
    auto tempValueLabel = lv_label_create(tempLayout);
    lv_label_set_text(tempValueLabel, "25 °C");
    lv_obj_set_width(tempValueLabel, 48);
    auto sliderEventHandler = [](lv_event_t *e)
    {
        auto slider = lv_event_get_target_obj(e);
        auto label = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
        LV_ASSERT_OBJ(slider, &lv_slider_class);
        LV_ASSERT_OBJ(label, &lv_label_class);

        int value = lv_slider_get_value(slider);
        lv_label_set_text_fmt(label, "%d °C", value);
    };
    lv_obj_add_event_cb(tempSlider, sliderEventHandler, LV_EVENT_VALUE_CHANGED, tempValueLabel);

    // Direction selection
    auto dirLayout = lv_obj_create(content);
    lv_obj_set_size(dirLayout, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_add_style(dirLayout, &horizonItemStyle, 0);
    auto dirHintLabel = lv_label_create(dirLayout);
    lv_label_set_text(dirHintLabel, "Direcion:");
    static const char *dirDropdownOptions = "Swing\nUp\nMiddle\nDown";
    auto dirDropdown = lv_dropdown_create(dirLayout);
    lv_dropdown_set_options_static(dirDropdown, dirDropdownOptions);

    // Fan speed selection
    auto speedLayout = lv_obj_create(content);
    lv_obj_set_size(speedLayout, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_add_style(speedLayout, &horizonItemStyle, 0);
    auto speedHintLabel = lv_label_create(speedLayout);
    lv_label_set_text(speedHintLabel, "Fan Speed:");
    static const char *speedDropdownOptions = "Auto\nLow\nMedium\nHigh";
    auto speedDropdown = lv_dropdown_create(speedLayout);
    lv_dropdown_set_options_static(speedDropdown, speedDropdownOptions);

    // We're gonna hide the other layouts if the mode is Off
    static std::vector<lv_obj_t *> layoutsBesidesMode;
    layoutsBesidesMode = {tempLayout, dirLayout, speedLayout};
    for (auto &obj : layoutsBesidesMode)
    {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN); // Hide the other options
    }
    lv_obj_add_event_cb(modeDropdown, ControlTab::modeDropdownEventHandler,
                        LV_EVENT_VALUE_CHANGED, &layoutsBesidesMode);

    // Footer buttons
    auto okButton = lv_msgbox_add_footer_button(controlMessageBox, "OK");
    std::vector<lv_obj_t *> widgets{modeDropdown, tempSlider, dirDropdown, speedDropdown};
    static std::pair<std::vector<Device>, std::vector<lv_obj_t *>> okButtonParams;
    okButtonParams = std::make_pair(selectedDevices, widgets);
    lv_obj_add_event_cb(okButton, ControlTab::okButtonEventHandler, LV_EVENT_CLICKED, &okButtonParams);
}

/**
 * @brief Event handler for the mode dropdown selection change.
 *
 * This function hides or shows the options besides the mode dropdown based on the selected mode.
 *
 * @param e The LVGL event data
 */
void ControlTab::modeDropdownEventHandler(lv_event_t *e)
{
    auto dropdown = lv_event_get_target_obj(e);
    LV_ASSERT_OBJ(dropdown, &lv_dropdown_class);
    auto objs = static_cast<std::vector<lv_obj_t *> *>(lv_event_get_user_data(e));

    auto modeIsOff = lv_dropdown_get_selected(dropdown) == 0;
    for (auto &obj : *objs) // Skip the first one, which is the mode dropdown
    {
        if (modeIsOff)
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN); // Hide the other options
        else
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN); // Show the other options
    }
}

/**
 * @brief Event handler for the OK button click event in the configuration message box.
 *
 * This function retrieves the configuration values from the message box and sends them to the deviceControlQueue.
 *
 * @param e The LVGL event data
 */
void ControlTab::okButtonEventHandler(lv_event_t *e)
{
    // The first parameter is a pointer to a vector of selected devices
    // The second parameter is a pointer to an array of objects in the message box
    auto params = static_cast<std::pair<std::vector<Device>, std::vector<lv_obj_t *>> *>(lv_event_get_user_data(e));

    auto selectedDevices = params->first;
    // An vector containing the layouts of the message box
    // They are: modeDropdown, tempSlider, dirDropdown, speedDropdown
    auto objs = params->second;

    LV_ASSERT_OBJ(objs[0], &lv_dropdown_class);
    LV_ASSERT_OBJ(objs[1], &lv_slider_class);
    LV_ASSERT_OBJ(objs[2], &lv_dropdown_class);
    LV_ASSERT_OBJ(objs[3], &lv_dropdown_class);

    auto mode = static_cast<Device::DeviceMode>(lv_dropdown_get_selected(objs[0]));
    auto temp = lv_slider_get_value(objs[1]);
    auto direction = static_cast<Device::Direction>(lv_dropdown_get_selected(objs[2]));
    auto speed = static_cast<Device::FanSpeed>(lv_dropdown_get_selected(objs[3]));

    for (auto &device : selectedDevices)
    {
        device.mode = mode;
        device.temperature = temp;
        device.direction = direction;
        device.speed = speed;
        // Send the updated device to the queue
        xQueueSend(deviceControlQueue, &device, portMAX_DELAY);
    }

    auto button = lv_event_get_target_obj(e);
    auto msgBox = lv_obj_get_parent(lv_obj_get_parent(button));
    lv_msgbox_close(msgBox); // Close the message box asynchronously
}

void ControlTab::updateTab()
{
    // Update device list or UI elements if needed
}

lv_style_t ControlTab::horizonItemStyle;
