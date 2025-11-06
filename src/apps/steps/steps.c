/**
 * @file steps.c
 * @brief Steps Counter App for C3 Watch with real-time sensor integration
 */

#include "steps.h"

#ifdef ENABLE_APP_STEPS

REGISTER_APP("Steps", &ui_img_activity_l_png, ui_stepsScreen, ui_stepsScreen_screen_init);

// UI Elements
lv_obj_t *ui_stepsPanel;
lv_obj_t *ui_stepsTitle;
lv_obj_t *ui_stepsCountLabel;
lv_obj_t *ui_stepsCountValue;
lv_obj_t *ui_distanceLabel;
lv_obj_t *ui_distanceValue;
lv_obj_t *ui_caloriesLabel;
lv_obj_t *ui_caloriesValue;
lv_obj_t *ui_stepsArc;
lv_obj_t *ui_resetButton;
lv_obj_t *ui_resetButtonLabel;

// Step counting variables
static int total_steps = 0;
static float total_distance = 0.0; // in km
static int total_calories = 0;
static const float STEP_LENGTH_M = 0.75; // Average step length in meters
static const float CALORIES_PER_STEP = 0.04; // Average calories per step

// Accelerometer threshold and tracking
static float prev_acc_mag = 0.0;
static bool step_detected = false;
static const float STEP_THRESHOLD = 1.2; // Acceleration threshold for step detection
static uint32_t last_step_time = 0;
static const uint32_t MIN_STEP_INTERVAL_MS = 200; // Minimum time between steps (ms)

lv_timer_t *steps_timer = NULL;

// Calculate magnitude of acceleration vector
float calc_acc_magnitude(float x, float y, float z)
{
    return sqrtf(x * x + y * y + z * z);
}

// Detect step from accelerometer data
bool detect_step_from_imu(void)
{
    imu_data_t imu = get_imu_data();
    
    if (!imu.success)
    {
        return false;
    }
    
    // Calculate acceleration magnitude
    float acc_mag = calc_acc_magnitude(imu.ax, imu.ay, imu.az);
    
    // Get current time
    uint32_t current_time = lv_tick_get();
    
    // Check if enough time has passed since last step
    if (current_time - last_step_time < MIN_STEP_INTERVAL_MS)
    {
        prev_acc_mag = acc_mag;
        return false;
    }
    
    // Detect peak in acceleration (step detection)
    // A step is detected when acceleration crosses threshold going up then down
    if (prev_acc_mag < STEP_THRESHOLD && acc_mag >= STEP_THRESHOLD && !step_detected)
    {
        step_detected = true;
    }
    else if (prev_acc_mag >= STEP_THRESHOLD && acc_mag < STEP_THRESHOLD && step_detected)
    {
        step_detected = false;
        last_step_time = current_time;
        prev_acc_mag = acc_mag;
        return true; // Step detected!
    }
    
    prev_acc_mag = acc_mag;
    return false;
}

// Update step count and related metrics
void update_step_metrics(void)
{
    // Update distance (steps * step_length)
    total_distance = (total_steps * STEP_LENGTH_M) / 1000.0; // Convert to km
    
    // Update calories
    total_calories = (int)(total_steps * CALORIES_PER_STEP);
}

// Update UI display
void update_steps_display(void)
{
    if (ui_stepsScreen == NULL)
    {
        return;
    }
    
    // Update step count
    lv_label_set_text_fmt(ui_stepsCountValue, "%d", total_steps);
    
    // Update distance
    lv_label_set_text_fmt(ui_distanceValue, "%.2f km", total_distance);
    
    // Update calories
    lv_label_set_text_fmt(ui_caloriesValue, "%d kcal", total_calories);
    
    // Update arc (goal: 10,000 steps)
    int percentage = (total_steps * 100) / 10000;
    if (percentage > 100) percentage = 100;
    lv_arc_set_value(ui_stepsArc, percentage);
}

// Timer callback for step detection
void steps_timer_cb(lv_timer_t *timer)
{
    // Check for step using IMU
    if (detect_step_from_imu())
    {
        total_steps++;
        update_step_metrics();
        update_steps_display();
    }
}

// Reset button callback
void ui_event_resetButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        total_steps = 0;
        total_distance = 0.0;
        total_calories = 0;
        update_steps_display();
    }
}

// Screen event callback
void ui_event_stepsScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if (event_code == LV_EVENT_SCREEN_LOADED)
    {
        onGameOpened();
        
        // Start step detection timer (check every 50ms)
        steps_timer = lv_timer_create(steps_timer_cb, 50, NULL);
        lv_timer_set_repeat_count(steps_timer, -1);
        
        // Initialize display
        update_steps_display();
    }
    
    if (event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {
        if (steps_timer != NULL)
        {
            lv_timer_delete(steps_timer);
            steps_timer = NULL;
        }
    }
    
    if (event_code == LV_EVENT_SCREEN_UNLOADED)
    {
        onGameClosed();
    }
    
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_RIGHT)
    {
        ui_app_exit();
    }
}

// Initialize the steps screen
void ui_stepsScreen_screen_init(void)
{
    ui_stepsScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_stepsScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_stepsScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_stepsScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Main panel
    ui_stepsPanel = lv_obj_create(ui_stepsScreen);
    lv_obj_set_width(ui_stepsPanel, lv_pct(100));
    lv_obj_set_height(ui_stepsPanel, lv_pct(100));
    lv_obj_set_align(ui_stepsPanel, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_stepsPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_stepsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_stepsPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_stepsPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_stepsPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Title
    ui_stepsTitle = lv_label_create(ui_stepsPanel);
    lv_obj_set_width(ui_stepsTitle, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_stepsTitle, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_stepsTitle, 0);
    lv_obj_set_y(ui_stepsTitle, -110);
    lv_obj_set_align(ui_stepsTitle, LV_ALIGN_CENTER);
    lv_label_set_text(ui_stepsTitle, "Steps Counter");
    lv_obj_set_style_text_font(ui_stepsTitle, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_stepsTitle, lv_color_hex(0x00A8FF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Steps Arc (progress circle)
    ui_stepsArc = lv_arc_create(ui_stepsPanel);
    lv_obj_set_width(ui_stepsArc, 180);
    lv_obj_set_height(ui_stepsArc, 180);
    lv_obj_set_x(ui_stepsArc, 0);
    lv_obj_set_y(ui_stepsArc, -20);
    lv_obj_set_align(ui_stepsArc, LV_ALIGN_CENTER);
    lv_arc_set_range(ui_stepsArc, 0, 100);
    lv_arc_set_value(ui_stepsArc, 0);
    lv_arc_set_bg_angles(ui_stepsArc, 0, 360);
    lv_obj_remove_flag(ui_stepsArc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(ui_stepsArc, lv_color_hex(0x00A8FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_stepsArc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_stepsArc, 12, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_stepsArc, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Steps count label
    ui_stepsCountLabel = lv_label_create(ui_stepsPanel);
    lv_obj_set_width(ui_stepsCountLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_stepsCountLabel, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_stepsCountLabel, 0);
    lv_obj_set_y(ui_stepsCountLabel, -35);
    lv_obj_set_align(ui_stepsCountLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_stepsCountLabel, "STEPS");
    lv_obj_set_style_text_font(ui_stepsCountLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_stepsCountLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Steps count value
    ui_stepsCountValue = lv_label_create(ui_stepsPanel);
    lv_obj_set_width(ui_stepsCountValue, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_stepsCountValue, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_stepsCountValue, 0);
    lv_obj_set_y(ui_stepsCountValue, -10);
    lv_obj_set_align(ui_stepsCountValue, LV_ALIGN_CENTER);
    lv_label_set_text(ui_stepsCountValue, "0");
    lv_obj_set_style_text_font(ui_stepsCountValue, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_stepsCountValue, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Distance label
    ui_distanceLabel = lv_label_create(ui_stepsPanel);
    lv_obj_set_width(ui_distanceLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_distanceLabel, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_distanceLabel, -60);
    lv_obj_set_y(ui_distanceLabel, 80);
    lv_obj_set_align(ui_distanceLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_distanceLabel, "Distance");
    lv_obj_set_style_text_font(ui_distanceLabel, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_distanceLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Distance value
    ui_distanceValue = lv_label_create(ui_stepsPanel);
    lv_obj_set_width(ui_distanceValue, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_distanceValue, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_distanceValue, -60);
    lv_obj_set_y(ui_distanceValue, 95);
    lv_obj_set_align(ui_distanceValue, LV_ALIGN_CENTER);
    lv_label_set_text(ui_distanceValue, "0.00 km");
    lv_obj_set_style_text_font(ui_distanceValue, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_distanceValue, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Calories label
    ui_caloriesLabel = lv_label_create(ui_stepsPanel);
    lv_obj_set_width(ui_caloriesLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_caloriesLabel, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_caloriesLabel, 60);
    lv_obj_set_y(ui_caloriesLabel, 80);
    lv_obj_set_align(ui_caloriesLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_caloriesLabel, "Calories");
    lv_obj_set_style_text_font(ui_caloriesLabel, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_caloriesLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Calories value
    ui_caloriesValue = lv_label_create(ui_stepsPanel);
    lv_obj_set_width(ui_caloriesValue, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_caloriesValue, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_caloriesValue, 60);
    lv_obj_set_y(ui_caloriesValue, 95);
    lv_obj_set_align(ui_caloriesValue, LV_ALIGN_CENTER);
    lv_label_set_text(ui_caloriesValue, "0 kcal");
    lv_obj_set_style_text_font(ui_caloriesValue, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_caloriesValue, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Reset button
    ui_resetButton = lv_btn_create(ui_stepsPanel);
    lv_obj_set_width(ui_resetButton, 100);
    lv_obj_set_height(ui_resetButton, 40);
    lv_obj_set_x(ui_resetButton, 0);
    lv_obj_set_y(ui_resetButton, 125);
    lv_obj_set_align(ui_resetButton, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(ui_resetButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_resetButton, lv_color_hex(0xFF4444), LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_resetButtonLabel = lv_label_create(ui_resetButton);
    lv_obj_set_align(ui_resetButtonLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_resetButtonLabel, "Reset");
    lv_obj_set_style_text_font(ui_resetButtonLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add event callbacks
    lv_obj_add_event_cb(ui_stepsScreen, ui_event_stepsScreen, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_resetButton, ui_event_resetButton, LV_EVENT_ALL, NULL);
}

// Public function to get current step count (for watchfaces)
int get_step_count(void)
{
    return total_steps;
}

// Public function to get distance (for watchfaces)
float get_distance_km(void)
{
    return total_distance;
}

// Public function to get calories (for watchfaces)
int get_calories(void)
{
    return total_calories;
}

#endif
