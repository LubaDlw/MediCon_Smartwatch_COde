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
#include "../common/api.h"

#ifdef ENABLE_APP_STEPS

    extern lv_obj_t *ui_stepsScreen;

    void ui_stepsScreen_screen_init(void);

#endif

#ifdef __cplusplus
}
#endif

#endif // STEPS_H
