#include <pebble.h>

#define STORAGE_BASE_KEY 22456709
// #undef RELEASE

#ifdef RELEASE
    #pragma message "---- COMPILING IN RELEASE MODE - NO LOGGING WILL BE AVAILABLE ----"
    #define LOGI(...)
    #define LOG(...)
    #define WARN(...)
    #define ERROR(...)
    #undef APP_LOG
    #define APP_LOG(...)
#else
    #define LOGI(...) ((void) APP_LOG(APP_LOG_LEVEL_DEBUG, __VA_ARGS__))
    #define LOG(...) app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
    #define WARN(...) app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
    #define ERROR(...) app_log(APP_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#endif

typedef enum {TRAIN, PRESENT} mode_e;

typedef enum {COUNTDOWN, TIMER} display_style_e;

typedef enum {MODE, DISPLAY_STYLE} enum_e;

typedef struct {
    time_t time_sec;
    uint16_t time_ms;
} ms_time_t;

typedef struct {
    char* text;
    uint8_t clearable_counter;
    bool show;
} info_t;

typedef struct {
    display_style_e display_style;
    uint16_t time_of_buzz_before_slide_ends;
    uint16_t NUM_SETTINGS;
} settings_t;

double ms_time_to_sec(ms_time_t time);

ms_time_t ms_time_subtract(ms_time_t, ms_time_t);

ms_time_t ms_time_get_wall_time();

ms_time_t ms_time_from_sec_ms(uint16_t, uint16_t);

void seconds_to_string(ms_time_t, char *, uint8_t, char *);

void set_info(info_t *, char *, uint8_t);

void clear_info(info_t *);

bool persist_data(uint16_t *, uint16_t);

void load_data(uint16_t *, uint16_t);

void load_settings(settings_t *);

bool persist_settings(settings_t *);

char* enum_to_string(enum_e, int);

#ifndef PBL_PLATFORM_APLITE
    static void battery_proc(Layer *layer, GContext *ctx) {
      // Emulator battery meter on Aplite
      graphics_context_set_stroke_color(ctx, GColorWhite);
      graphics_draw_rect(ctx, GRect(126, 4, 14, 8));
      graphics_draw_line(ctx, GPoint(140, 6), GPoint(140, 9));

      BatteryChargeState state = battery_state_service_peek();
      int width = (int)(float)(((float)state.charge_percent / 100.0F) * 10.0F);
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, GRect(128, 6, width, 4), 0, GCornerNone);
    }
#endif