#pragma once

#include "Tab.hpp"

class PairingTab : public Tab
{
private:
    lv_obj_t *devListView;
    lv_obj_t *scanButton;
    lv_obj_t *scanSpinner;

    lv_obj_t *pairingMsgBox;
    lv_obj_t *pairingSpinner;

    static lv_style_t horizonItemStyle;

    static void scanButtonHandler(lv_event_t *e);
    static void pairButtonHandler(lv_event_t *e);

public:
    PairingTab(lv_obj_t *parent);
    void initTab() override;
    void updateTab() override;
};
