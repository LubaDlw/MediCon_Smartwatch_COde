/**
 * @file medicon.h
 * @brief MediCon Health & Fitness Watch Face
 */

#ifndef MEDICON_H
#define MEDICON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

extern lv_obj_t *face_medicon_root;

void onFaceEvent(lv_event_t * e);

void init_face_medicon(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **));
void update_check_medicon(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                          int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);

#ifdef __cplusplus
}
#endif

#endif // MEDICON_H
