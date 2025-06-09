#include <lvgl.h>
#include <freertos/semphr.h>

#pragma once

class Tab
{
protected:
    lv_obj_t *root;

public:
    Tab(lv_obj_t *parent, const char *name)
    {
        LV_ASSERT_OBJ(parent, lv_tabview_class);

        root = lv_tabview_add_tab(parent, name);
    }

    ~Tab()
    {
        lv_obj_delete(root);
    }

    virtual void initTab() = 0;   // Pure virtual function to be implemented by derived classes
    virtual void updateTab() = 0; // Pure virtual function to be implemented by derived classes

    inline lv_obj_t *getRoot() const
    {
        return root;
    }
};
