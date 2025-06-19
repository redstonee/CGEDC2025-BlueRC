#include "Tabs/ParingTab.h"
#include "Device.hpp"
#include "blue.h"

extern "C"
{
    LV_FONT_DECLARE(mode_logo_20)
}

// WARNING: Global variables, use with CAUTION!
extern QueueHandle_t bleScanUnsavedDeviceQueue;
extern QueueHandle_t blePairResultQueue;
extern SemaphoreHandle_t bleScanStartSemaphore;

constexpr auto LOGO_REFRESH = "\xEF\x80\xA1";

PairingTab::PairingTab(lv_obj_t *parent)
    : Tab(parent, "Paring") {};

/**
 * @brief Initialize the Paring tab.
 */
void PairingTab::initTab()
{
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
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
    lv_label_set_text(titleLabel, "Paring");
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_24, 0);

    // The list view for displaying devices
    devListView = lv_list_create(root);
    lv_obj_set_flex_grow(devListView, 1);
    lv_obj_set_style_pad_row(devListView, 3, 0);
}

void PairingTab::pairButtonHandler(lv_event_t *e)
{
    blue::pauseScan(); // Pause the scan when pairing starts
    auto button = lv_event_get_target_obj(e);
    auto pairingTab = static_cast<PairingTab *>(lv_event_get_user_data(e));
    auto device = static_cast<Device *>(lv_obj_get_user_data(button));

    pairingTab->pairingMsgBox = lv_msgbox_create(nullptr);
    lv_msgbox_add_title(pairingTab->pairingMsgBox, ("Paring " + String(device->name)).c_str());
    lv_obj_set_user_data(pairingTab->pairingMsgBox, device); // Store the device in the message box

    pairingTab->pairingSpinner = lv_spinner_create(pairingTab->pairingMsgBox);
    lv_obj_set_size(pairingTab->pairingSpinner, 40, 40);
    lv_obj_center(pairingTab->pairingSpinner);

    blue::tryToPairDevice(device->address);
}

void PairingTab::updateTab()
{
    // Update logic for the Paring tab can be implemented here.
    if ((!bleScanUnsavedDeviceQueue) || (!bleScanStartSemaphore) || (!blePairResultQueue))
    {
        // If the queues or semaphores are not initialized, do nothing
        return;
    }

    static std::vector<Device> deviceList;
    Device device;
    if (xQueueReceive(bleScanUnsavedDeviceQueue, &device, 0))
    {
        // Create a new list item for the device
        auto listItem = lv_obj_create(devListView);
        lv_obj_remove_style_all(listItem);
        lv_obj_set_size(listItem, lv_pct(100), 40);
        lv_obj_add_style(listItem, &horizonItemStyle, 0);

        // Add the information labels
        auto nameLabel = lv_label_create(listItem);
        lv_label_set_text(nameLabel, device.name);
        lv_obj_set_width(nameLabel, lv_pct(60));
        lv_label_set_long_mode(nameLabel, LV_LABEL_LONG_MODE_SCROLL);

        deviceList.push_back(device);

        // Add a button to the item for pairing
        auto pairButton = lv_button_create(listItem);
        lv_obj_set_user_data(pairButton, &deviceList.back()); // Store the device in the button's user data
        lv_obj_add_event_cb(pairButton, pairButtonHandler, LV_EVENT_CLICKED, this);
        auto buttonLabel = lv_label_create(pairButton);
        lv_label_set_text(buttonLabel, "Pair");
    }

    if (xSemaphoreTake(bleScanStartSemaphore, 0))
    {
        // If the semaphore is taken, the scan is complete
        lv_obj_clean(devListView); // Clear the existing list view
        // for (auto &dev : deviceList)
        // {
        //     delete[] dev.name; // Free the device name memory
        // }
        deviceList.clear(); // Clear the device list
    }

    bool pairingSuccess;
    if (xQueueReceive(blePairResultQueue, &pairingSuccess, 0))
    {
        // Create a button to close the pairing message box
        auto okButton = lv_msgbox_add_footer_button(pairingMsgBox, "OK");
        auto okButtonHandler = [](lv_event_t *e)
        {
            blue::resumeScan(); // Resume scanning after pairing is done
            auto okButton = lv_event_get_target_obj(e);
            lv_msgbox_close(lv_obj_get_parent(lv_obj_get_parent(okButton))); // Close the message box
        };
        lv_obj_add_event_cb(okButton, okButtonHandler, LV_EVENT_CLICKED, nullptr);

        // Remove the spinner and show the result
        lv_obj_delete(pairingSpinner); // Remove the spinner from the message box
        auto resultLabel = lv_label_create(pairingMsgBox);
        if (pairingSuccess)
        {
            lv_label_set_text(resultLabel, "Pairing successful!");
            lv_obj_set_style_text_color(resultLabel, lv_palette_main(LV_PALETTE_GREEN), 0);

            auto device = static_cast<Device *>(lv_obj_get_user_data(pairingMsgBox));
            blue::addDevice(device->name, device->address); // Save the device after successful pairing
        }
        else
        {
            lv_label_set_text(resultLabel, "Pairing failed.");
            lv_obj_set_style_text_color(resultLabel, lv_palette_main(LV_PALETTE_RED), 0);
        }
    }
}

lv_style_t PairingTab::horizonItemStyle;
