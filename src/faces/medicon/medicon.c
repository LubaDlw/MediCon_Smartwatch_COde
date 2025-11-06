/**
 * @file medicon.c
 * @brief MediCon Health & Fitness Watch Face
 * Design: Health-focused watch face with large time display, steps, calories, and heart rate
 */

#include "medicon.h"
#include "../../common/api.h"

lv_obj_t *face_medicon_root = NULL;

// UI Elements
static lv_obj_t *time_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *day_label = NULL;
static lv_obj_t *steps_value = NULL;
static lv_obj_t *steps_icon = NULL;
static lv_obj_t *calories_value = NULL;
static lv_obj_t *calories_icon = NULL;
static lv_obj_t *heart_value = NULL;
static lv_obj_t *heart_icon = NULL;

// MediCon brand colors - Health-focused green theme
#define MEDICON_PRIMARY 0x2ECC71    // Medical green
#define MEDICON_ACCENT 0x27AE60     // Darker green
#define MEDICON_TEXT 0xDCF5EA       // Light mint
#define MEDICON_BG 0x1A3329         // Dark green background

void face_medicon_init(void)
{
    if (face_medicon_root != NULL)
    {
        return;
    }

    // Create root container
    face_medicon_root = lv_obj_create(NULL);
    lv_obj_remove_flag(face_medicon_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(face_medicon_root, lv_color_hex(MEDICON_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_medicon_root, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add event callback for touch gestures
    lv_obj_add_event_cb(face_medicon_root, onFaceEvent, LV_EVENT_ALL, NULL);

    // Day label (THU)
    day_label = lv_label_create(face_medicon_root);
    lv_obj_set_width(day_label, LV_SIZE_CONTENT);
    lv_obj_set_height(day_label, LV_SIZE_CONTENT);
    lv_obj_set_x(day_label, 0);
    lv_obj_set_y(day_label, -85);
    lv_obj_set_align(day_label, LV_ALIGN_CENTER);
    lv_label_set_text(day_label, "THU 23");
    lv_obj_set_style_text_font(day_label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(day_label, lv_color_hex(MEDICON_TEXT), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Time label (large)
    time_label = lv_label_create(face_medicon_root);
    lv_obj_set_width(time_label, LV_SIZE_CONTENT);
    lv_obj_set_height(time_label, LV_SIZE_CONTENT);
    lv_obj_set_x(time_label, 0);
    lv_obj_set_y(time_label, -35);
    lv_obj_set_align(time_label, LV_ALIGN_CENTER);
    lv_label_set_text(time_label, "10:08");
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(time_label, lv_color_hex(MEDICON_PRIMARY), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Calories icon and value (flame icon)
    calories_icon = lv_label_create(face_medicon_root);
    lv_obj_set_width(calories_icon, LV_SIZE_CONTENT);
    lv_obj_set_height(calories_icon, LV_SIZE_CONTENT);
    lv_obj_set_x(calories_icon, -80);
    lv_obj_set_y(calories_icon, 30);
    lv_obj_set_align(calories_icon, LV_ALIGN_CENTER);
    lv_label_set_text(calories_icon, LV_SYMBOL_CHARGE);
    lv_obj_set_style_text_font(calories_icon, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(calories_icon, lv_color_hex(0xFF6B35), LV_PART_MAIN | LV_STATE_DEFAULT);

    calories_value = lv_label_create(face_medicon_root);
    lv_obj_set_width(calories_value, LV_SIZE_CONTENT);
    lv_obj_set_height(calories_value, LV_SIZE_CONTENT);
    lv_obj_set_x(calories_value, -45);
    lv_obj_set_y(calories_value, 30);
    lv_obj_set_align(calories_value, LV_ALIGN_CENTER);
    lv_label_set_text(calories_value, "316");
    lv_obj_set_style_text_font(calories_value, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(calories_value, lv_color_hex(MEDICON_TEXT), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Heart rate icon and value
    heart_icon = lv_label_create(face_medicon_root);
    lv_obj_set_width(heart_icon, LV_SIZE_CONTENT);
    lv_obj_set_height(heart_icon, LV_SIZE_CONTENT);
    lv_obj_set_x(heart_icon, 15);
    lv_obj_set_y(heart_icon, 30);
    lv_obj_set_align(heart_icon, LV_ALIGN_CENTER);
    lv_label_set_text(heart_icon, LV_SYMBOL_DUMMY "\xEF\x80\x84"); // Heart symbol
    lv_obj_set_style_text_font(heart_icon, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(heart_icon, lv_color_hex(0xE74C3C), LV_PART_MAIN | LV_STATE_DEFAULT);

    heart_value = lv_label_create(face_medicon_root);
    lv_obj_set_width(heart_value, LV_SIZE_CONTENT);
    lv_obj_set_height(heart_value, LV_SIZE_CONTENT);
    lv_obj_set_x(heart_value, 50);
    lv_obj_set_y(heart_value, 30);
    lv_obj_set_align(heart_value, LV_ALIGN_CENTER);
    lv_label_set_text(heart_value, "82");
    lv_obj_set_style_text_font(heart_value, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(heart_value, lv_color_hex(MEDICON_TEXT), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Steps icon and value (footsteps)
    steps_icon = lv_label_create(face_medicon_root);
    lv_obj_set_width(steps_icon, LV_SIZE_CONTENT);
    lv_obj_set_height(steps_icon, LV_SIZE_CONTENT);
    lv_obj_set_x(steps_icon, -70);
    lv_obj_set_y(steps_icon, 75);
    lv_obj_set_align(steps_icon, LV_ALIGN_CENTER);
    lv_label_set_text(steps_icon, LV_SYMBOL_SHUFFLE);
    lv_obj_set_style_text_font(steps_icon, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(steps_icon, lv_color_hex(MEDICON_PRIMARY), LV_PART_MAIN | LV_STATE_DEFAULT);

    steps_value = lv_label_create(face_medicon_root);
    lv_obj_set_width(steps_value, LV_SIZE_CONTENT);
    lv_obj_set_height(steps_value, LV_SIZE_CONTENT);
    lv_obj_set_x(steps_value, -10);
    lv_obj_set_y(steps_value, 75);
    lv_obj_set_align(steps_value, LV_ALIGN_CENTER);
    lv_label_set_text(steps_value, "7,532");
    lv_obj_set_style_text_font(steps_value, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(steps_value, lv_color_hex(MEDICON_PRIMARY), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void face_medicon_update(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                         int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
    if (face_medicon_root == NULL)
    {
        return;
    }

    // Update time (HH:MM format)
    lv_label_set_text_fmt(time_label, "%02d:%02d", hour, minute);

    // Update day and date
    const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    lv_label_set_text_fmt(day_label, "%s %d", days[weekday], day);

    // Update steps
    if (steps >= 10000)
    {
        lv_label_set_text_fmt(steps_value, "%d,%03d", steps / 1000, steps % 1000);
    }
    else if (steps >= 1000)
    {
        lv_label_set_text_fmt(steps_value, "%d,%03d", steps / 1000, steps % 1000);
    }
    else
    {
        lv_label_set_text_fmt(steps_value, "%d", steps);
    }

    // Update calories
    lv_label_set_text_fmt(calories_value, "%d", kcal);

    // Update heart rate (BPM)
    lv_label_set_text_fmt(heart_value, "%d", bpm);
}

// Wrapper functions to match expected naming convention
void init_face_medicon(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **))
{
    face_medicon_init();
    // Register this watchface with the UI system
    callback("MediCon", NULL, &face_medicon_root, NULL);
}

void update_check_medicon(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
                          int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
    if (root != face_medicon_root)
    {
        return;
    }
    face_medicon_update(second, minute, hour, mode, am, day, month, year, weekday, temp, icon, battery, connection, steps, distance, kcal, bpm, oxygen);
}
