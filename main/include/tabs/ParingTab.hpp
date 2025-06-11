#pragma once

#include "Tab.hpp"

class Pairing : public Tab
{
private:
    uint32_t lastUpdateTime = 0;

public:
    Pairing(lv_obj_t *parent)
        : Tab(parent, "Paring") {};

    void initTab() override
    {
        lv_obj_set_size(root, lv_pct(100), lv_pct(100));
        static lv_style_t tabStyle;
        lv_style_init(&tabStyle);
        lv_style_set_flex_main_place(&tabStyle, LV_FLEX_ALIGN_SPACE_EVENLY);
        lv_style_set_flex_flow(&tabStyle, LV_FLEX_FLOW_COLUMN);
        lv_style_set_layout(&tabStyle, LV_LAYOUT_FLEX);
        lv_obj_add_style(root, &tabStyle, 0);

        auto *titleLabel = lv_label_create(root);
        lv_label_set_text(titleLabel, "Paring");
        lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_24, 0);

        auto listLayout = lv_obj_create(root);
        lv_obj_remove_style_all(listLayout);

        static lv_style_t layoutStyle;
        lv_style_init(&layoutStyle);
        lv_style_set_flex_main_place(&layoutStyle, LV_FLEX_ALIGN_SPACE_BETWEEN);
        lv_style_set_flex_flow(&layoutStyle, LV_FLEX_FLOW_ROW);
        lv_style_set_layout(&layoutStyle, LV_LAYOUT_FLEX);
    }

    void updateTab() override
    {
        if (millis() - lastUpdateTime < 1000)
            return;

        // Update battery information here

        lastUpdateTime = millis();
    }
};
