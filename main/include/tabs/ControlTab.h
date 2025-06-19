#pragma once

#include "Tab.hpp"
#include "Device.hpp"
#include <vector>

class ControlTab : public Tab
{
private:
    // std::vector<Device> _deviceList; // List of saved devices for test purposes
    lv_obj_t *devListView;
    lv_obj_t *controlMessageBox;
    lv_obj_t *waitResultSpinner;
    lv_obj_t *resultMsgBox;
    
    static lv_style_t horizonItemStyle;

    static void configButtonHandler(lv_event_t *e);
    static void checkboxEventHandler(lv_event_t *e);
    static void modeDropdownEventHandler(lv_event_t *e);
    static void okButtonEventHandler(lv_event_t *e);
    static void viewButtonEventHandler(lv_event_t *e);

public:
    ControlTab(lv_obj_t *parent);
    void initTab() override;
    void updateTab() override;
};
