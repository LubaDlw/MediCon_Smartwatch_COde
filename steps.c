/*
 * MIT License
 *
 * Copyright (c) 2024 Vuyo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ______________  _____
 * ___  __/___  /_ ___(_)_____ _______ _______
 * __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
 * _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
 * /_/     /_.___/ /_/   \___/ _\__, /  \____/
 *                             /____/
 */

#include "steps.h"
#include "ui/ui.h"

REGISTER_APP("Steps", &ui_img_activity_l_png, ui_stepsScreen, ui_stepsScreen_screen_init);

// UI Objects
static lv_obj_t *ui_stepsScreen;
static lv_obj_t *ui_stepsLabel;
static lv_obj_t *ui_distanceLabel;
static lv_obj_t *ui_caloriesLabel;
static lv_obj_t *ui_stepsArc;
static lv_obj_t *ui_fallAlertPanel;
static lv_obj_t *ui_fallAlertLabel;

// Step counting parameters
#define STEP_THRESHOLD 1.15f
#define MIN_STEP_INTERVAL 250
#define MAX_STEP_INTERVAL 2000
#define MOVING_AVG_SIZE 5

// Fall detection parameters
#define FALL_THRESHOLD 2.5f
#define FREEFALL_THRESHOLD 0.5f
#define FALL_COOLDOWN_MS 5000

static int step_count = 0;
static float distance_km = 0.0f;
static int calories = 0;

static unsigned long last_step_time = 0;
static bool high_g_event = false;
static float acc_mag_avg[MOVING_AVG_SIZE] = {0};
static int avg_index = 0;

static bool fall_detected = false;
static unsigned long last_fall_time = 0;

static float calc_acc_magnitude(imu_data_t *imu) {
    return sqrt(imu->ax * imu->ax + imu->ay * imu->ay + imu->az * imu->az);
}

static float apply_moving_average(float new_val) {
    acc_mag_avg[avg_index] = new_val;
    avg_index = (avg_index + 1) % MOVING_AVG_SIZE;

    float sum = 0;
    for (int i = 0; i < MOVING_AVG_SIZE; i++) {
        sum += acc_mag_avg[i];
    }
    return sum / MOVING_AVG_SIZE;
}

static void detect_step_from_imu() {
    imu_data_t imu = get_imu_data();
    if (!imu.success) return;

    float mag = calc_acc_magnitude(&imu);
    float smoothed_mag = apply_moving_average(mag);

    unsigned long current_time = millis();

    if (smoothed_mag > STEP_THRESHOLD && !high_g_event) {
        if (current_time - last_step_time > MIN_STEP_INTERVAL) {
            high_g_event = true;
            last_step_time = current_time;
            step_count++;
            distance_km = step_count * 0.00075f; // 0.75m per step
            calories = step_count * 0.04f;
        }
    } else if (smoothed_mag < 1.0f) {
        high_g_event = false;
    }

    if (current_time - last_step_time > MAX_STEP_INTERVAL) {
        // Reset if no steps for a while
    }
}

static void show_fall_alert() {
    if (ui_fallAlertPanel) {
        lv_label_set_text(ui_fallAlertLabel, "FALL DETECTED!\nAre you OK?");
        lv_obj_clear_flag(ui_fallAlertPanel, LV_OBJ_FLAG_HIDDEN);
        // Auto-dismiss after some time
        lv_timer_t *timer = lv_timer_create( {
            lv_obj_add_flag(ui_fallAlertPanel, LV_OBJ_FLAG_HIDDEN);
            fall_detected = false;
        }, 5000, NULL);
        lv_timer_set_repeat_count(timer, 1);
    }
}

static void detect_fall_from_imu() {
    imu_data_t imu = get_imu_data();
    if (!imu.success) return;

    float mag = calc_acc_magnitude(&imu);
    unsigned long current_time = millis();

    if (current_time - last_fall_time < FALL_COOLDOWN_MS) {
        return; // Cooldown period
    }

    if (mag > FALL_THRESHOLD || mag < FREEFALL_THRESHOLD) {
        if (!fall_detected) {
            fall_detected = true;
            last_fall_time = current_time;
            show_fall_alert();
        }
    }
}

static void update_steps_display() {
    if (lv_screen_active() == ui_stepsScreen) {
        lv_label_set_text_fmt(ui_stepsLabel, "%d", step_count);
        lv_label_set_text_fmt(ui_distanceLabel, "%.2f km", distance_km);
        lv_label_set_text_fmt(ui_caloriesLabel, "%d kcal", calories);
        lv_arc_set_value(ui_stepsArc, (step_count % 10000) * 100 / 10000);
    }
}

static void steps_app_loop() {
    detect_step_from_imu();
    detect_fall_from_imu();
    update_steps_display();
}

static void reset_steps_cb(lv_event_t *e) {
    step_count = 0;
    distance_km = 0.0f;
    calories = 0;
    update_steps_display();
}

static void ui_event_stepsScreen(lv_event_t *e) {
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_SCREEN_LOAD_START) {
        onGameOpened();
        app_loop_override(steps_app_loop);
    }

    if (event_code == LV_EVENT_SCREEN_UNLOAD_START) {
        onGameClosed();
        app_loop_override(NULL);
    }

    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_RIGHT) {
        ui_app_exit();
    }
}

void ui_stepsScreen_screen_init(void) {
    ui_stepsScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_stepsScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_stepsScreen, lv_color_hex(0x1A3329), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_stepsScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_stepsScreen, ui_event_stepsScreen, LV_EVENT_ALL, NULL);

    // Title
    lv_obj_t *title = lv_label_create(ui_stepsScreen);
    lv_label_set_text(title, "Step Counter");
    lv_obj_set_style_text_color(title, lv_color_hex(0xDCF5EA), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // Progress Arc
    ui_stepsArc = lv_arc_create(ui_stepsScreen);
    lv_obj_set_size(ui_stepsArc, 150, 150);
    lv_arc_set_rotation(ui_stepsArc, 135);
    lv_arc_set_bg_angles(ui_stepsArc, 0, 270);
    lv_obj_remove_style(ui_stepsArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(ui_stepsArc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(ui_stepsArc, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_arc_color(ui_stepsArc, lv_color_hex(0x27AE60), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(ui_stepsArc, lv_color_hex(0x444), LV_PART_MAIN);

    // Step Count Label
    ui_stepsLabel = lv_label_create(ui_stepsArc);
    lv_label_set_text(ui_stepsLabel, "0");
    lv_obj_set_style_text_color(ui_stepsLabel, lv_color_hex(0x2ECC71), 0);
    lv_obj_set_style_text_font(ui_stepsLabel, &lv_font_montserrat_48, 0);
    lv_obj_center(ui_stepsLabel);

    // Distance Label
    ui_distanceLabel = lv_label_create(ui_stepsScreen);
    lv_label_set_text(ui_distanceLabel, "0.00 km");
    lv_obj_set_style_text_color(ui_distanceLabel, lv_color_hex(0xDCF5EA), 0);
    lv_obj_align(ui_distanceLabel, LV_ALIGN_BOTTOM_LEFT, 20, -50);

    // Calories Label
    ui_caloriesLabel = lv_label_create(ui_stepsScreen);
    lv_label_set_text(ui_caloriesLabel, "0 kcal");
    lv_obj_set_style_text_color(ui_caloriesLabel, lv_color_hex(0xDCF5EA), 0);
    lv_obj_align(ui_caloriesLabel, LV_ALIGN_BOTTOM_RIGHT, -20, -50);

    // Reset Button
    lv_obj_t *resetBtn = lv_button_create(ui_stepsScreen);
    lv_obj_align(resetBtn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(resetBtn, reset_steps_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *resetLabel = lv_label_create(resetBtn);
    lv_label_set_text(resetLabel, "Reset");
    lv_obj_center(resetLabel);

    // Fall Alert Panel
    ui_fallAlertPanel = lv_obj_create(ui_stepsScreen);
    lv_obj_set_size(ui_fallAlertPanel, 200, 100);
    lv_obj_align(ui_fallAlertPanel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(ui_fallAlertPanel, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_border_color(ui_fallAlertPanel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(ui_fallAlertPanel, 2, 0);
    lv_obj_add_flag(ui_fallAlertPanel, LV_OBJ_FLAG_HIDDEN);

    ui_fallAlertLabel = lv_label_create(ui_fallAlertPanel);
    lv_label_set_text(ui_fallAlertLabel, "");
    lv_obj_set_style_text_color(ui_fallAlertLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(ui_fallAlertLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(ui_fallAlertLabel);
}

int get_step_count(void) {
    return step_count;
}

float get_distance_km(void) {
    return distance_km;
}

int get_calories(void) {
    return calories;
}