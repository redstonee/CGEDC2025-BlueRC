#include <lvgl.h>
#include <freertos/semphr.h>

#pragma once

class Tab
{
protected:
    lv_obj_t *root;

public:
    /**
     * @brief Constructor for the Tab class.
     *
     * @param parent The parent object(must be a Tabview) to which this tab will be added.
     * @param name The name of the tab.
     */
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

    /**
     * @brief Get the pointer of the tab.
     *
     * @return Pointer to the tab object.
     */
    inline lv_obj_t *getRoot() const
    {
        return root;
    }
};
