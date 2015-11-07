#include <pebble.h>

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