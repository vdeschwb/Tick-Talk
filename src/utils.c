#include <pebble.h>
#include <utils.h>

ms_time_t ms_time_subtract(ms_time_t minuend, ms_time_t subtrahend) {
    ms_time_t result;
    
    result.time_sec = minuend.time_sec - subtrahend.time_sec;
    if (minuend.time_ms < subtrahend.time_ms) {
        result.time_sec--;
        result.time_ms = 1000 - (subtrahend.time_ms - minuend.time_ms);
    } else {
        result.time_ms = minuend.time_ms - subtrahend.time_ms;
    }
    return result;
}

ms_time_t ms_time_get_wall_time() {
    ms_time_t result;
    time_ms(&(result.time_sec), &(result.time_ms));
    return result;
}

void seconds_to_string(ms_time_t time, char *buf, uint8_t bufsize, char* format) {
    snprintf(buf, bufsize, format, time.time_sec / 60, time.time_sec % 60, time.time_ms);
}

ms_time_t ms_time_from_sec_ms(uint16_t sec, uint16_t ms) {
    ms_time_t result;
    result.time_sec = sec;
    result.time_ms = ms;
    return result;
}

double ms_time_to_sec(ms_time_t time) {
    return time.time_sec + ((double) time.time_ms) / 1000.0;
}

void set_info(info_t *info, char *text, uint8_t clearable_after) {
    info -> text = text;
    if (clearable_after == 0) {
        info -> clearable_counter = 0;
    } else {
        info -> clearable_counter = clearable_after + 1;
    }
    info -> show = true;
}

void clear_info(info_t *info) {
    if (info -> clearable_counter == 1) {
        info -> show = false;
    } else {
        info -> clearable_counter--;
    }
}

bool persist_data(uint16_t *data, uint16_t offset) {
    LOGI("Persist data...");
    bool succ=true;
    for (int i=0; i<data[0]+1; i+=2) {
        int code_1 = persist_write_int(STORAGE_BASE_KEY + i + offset, data[i] << 16 | data[i+1]);
        succ &= (code_1 == 0);
        LOGI("%d", code_1);    
    }
    return succ;
}

void load_data(uint16_t *data, uint16_t offset) {
    LOGI("Loading data...");
    uint16_t len = persist_read_int(STORAGE_BASE_KEY + offset) >> 16;
    int32_t tmp;
    for (int i=0; i<len+1; i+=2) {
        tmp = persist_read_int(STORAGE_BASE_KEY + i + offset);
        data[i] = tmp >> 16;
        data[i+1] = tmp & 0xFFFF;
    }
    LOGI("Done.");
}

void load_settings(settings_t *settings) {
    LOGI("Loading settings...");
    settings -> display_style = persist_read_int(STORAGE_BASE_KEY);
    settings -> time_of_buzz_before_slide_ends = persist_read_int(STORAGE_BASE_KEY + 1);
    settings -> fps = persist_read_int(STORAGE_BASE_KEY + 2);
    LOGI("Done.");
}

bool persist_settings(settings_t *settings) {
    LOGI("Persist settings...");
    bool succ=true;
    int code_1 = persist_write_int(STORAGE_BASE_KEY, (int) (settings -> display_style));
    int code_2 = persist_write_int(STORAGE_BASE_KEY + 1, (int) (settings -> time_of_buzz_before_slide_ends));
    int code_3 = persist_write_int(STORAGE_BASE_KEY + 2, (int) (settings -> fps));
    succ &= (code_1 == 0);
    succ &= (code_2 == 0);
    succ &= (code_3 == 0);
    LOGI("%d", code_1);
    LOGI("%d", code_2);
    LOGI("%d", code_3);
    return succ;
}

char* enum_to_string(enum_e enum_, int value) {
    switch(enum_) {
        case MODE:
            switch(value) {
                case TRAIN: return "Train"; break;
                case PRESENT: return "Present"; break;
            }
            break;
        case DISPLAY_STYLE:
            switch(value) {
                case TIMER: return "Timer"; break;
                case COUNTDOWN: return "Countdown"; break;
            }
            break;
    }
    return "[UNDEFINED]";
}