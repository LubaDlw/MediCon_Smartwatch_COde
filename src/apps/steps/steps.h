/**
 * @file steps.h
 * @brief Steps Counter App for C3 Watch
 */

#ifndef STEPS_H
#define STEPS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "../../common/api.h"
#include "../../common/app_manager.h"

// Forward declarations for app manager functions
void onGameOpened(void);
void onGameClosed(void);
void ui_app_exit(void);
void ui_app_load(lv_obj_t **screen, void (*screen_init)(void));

#ifdef ENABLE_APP_STEPS

    void ui_stepsScreen_screen_init(void);

#endif

// Declare icon
LV_IMG_DECLARE(ui_img_activity_l_png);

// Public API functions (available even without ENABLE_APP_STEPS)
int get_step_count(void);
float get_distance_km(void);
int get_calories(void);

// Background step counting functions
void steps_init_background(void);
void steps_update_background(void);

#ifdef __cplusplus
}
#endif

#endif // STEPS_H
