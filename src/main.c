#include <pebble.h>
#include <utils.h>

#define MAX_NUM_SLIDES 128
#define NUM_INVERTER_LAYERS 2
#define FPS 10
#define BUF_SIZE 32
#define NUM_MENU_SECTIONS 2


SimpleMenuSection menu_sections[NUM_MENU_SECTIONS];
SimpleMenuItem display_style_menu_items[1];
SimpleMenuItem pre_slide_buzz_time_menu_items[1];


settings_t settings = {
    .display_style = TIMER,
    .time_of_buzz_before_slide_ends = 10,
    .NUM_SETTINGS = 2
};

uint16_t *num_slides=NULL;
Window *main_window=NULL, *menu_window=NULL;
TextLayer *title_layer=NULL, *info_layer=NULL, *time_layer=NULL, *train_layer=NULL;
SimpleMenuLayer *menu=NULL;
BitmapLayer *draw_layer;
InverterLayer *invert_draw_layers[NUM_INVERTER_LAYERS];
uint16_t delays[MAX_NUM_SLIDES];
uint16_t current_slide;
AppTimer *seconds_timer=NULL;
volatile bool running;
ms_time_t last_time;
ms_time_t current_epoch;
info_t info;
mode_e current_mode;

// Buffers
char *info_buf=NULL, *time_buf=NULL;
char time_of_buzz_before_slide_ends_buf[BUF_SIZE];

void write_info() {
    if (!info_layer) {
        info_layer = text_layer_create(GRect(5, 40, 134, 30));
    
        // Set the font and text alignment
        text_layer_set_font(info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
        text_layer_set_text_alignment(info_layer, GTextAlignmentCenter);

        // Add the text layer to the window
        layer_add_child(window_get_root_layer(main_window), text_layer_get_layer(info_layer));

    }
	
	text_layer_set_text(info_layer, info_buf);
}

void write_time(ms_time_t time) {
    if (!time_layer) {
        time_layer = text_layer_create(GRect(5, 70, 134, 30));
        
        // Set the font and text alignment
        text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
        text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

        // Add the text layer to the window
        layer_add_child(window_get_root_layer(main_window), text_layer_get_layer(time_layer));

    }
	
	// Set the text, font, and text alignment
    char *buf1, *buf2;
    uint8_t bufsize=30;
    buf1 = malloc(bufsize);
    buf2 = malloc(bufsize);
    if (current_mode == TRAIN) {
        seconds_to_string(time, buf1, bufsize, "%li:%02li.%03u");
        text_layer_set_text(time_layer, buf1);
    } else {
        switch (settings.display_style) {
            case TIMER:
                seconds_to_string(time, buf1, bufsize, "%li:%02li");
                seconds_to_string(ms_time_from_sec_ms(delays[current_slide],0), buf2, bufsize, "%li:%02li");
        	    text_layer_set_text(time_layer, strcat(strcat(buf1, " / "), buf2));
                break;
            case COUNTDOWN:
                seconds_to_string(ms_time_subtract(ms_time_from_sec_ms(delays[current_slide], 0), time), buf1, bufsize, "%li:%02li.%03u");
                text_layer_set_text(time_layer, buf1);
                break;
        }
    }
    free(buf1);
    buf1 = NULL;
    free(buf2);
    buf2 = NULL;
}

void update() {
    if (info.show) {
        snprintf(info_buf, BUF_SIZE, "%s", info.text);
    } else {
        if (current_mode == TRAIN) {
            snprintf(info_buf, BUF_SIZE, "Slide: %u", current_slide);
        } else {
            snprintf(info_buf, BUF_SIZE, "Slide: %u / %u", current_slide, *num_slides);
        }
        write_info();
    }
    
    write_time(current_epoch);
    
    for (int i=0; i<NUM_INVERTER_LAYERS; ++i) {
        layer_set_hidden(inverter_layer_get_layer(invert_draw_layers[i]), running);
    }
    
    layer_set_hidden(bitmap_layer_get_layer(draw_layer), current_mode==TRAIN);
    layer_set_hidden(text_layer_get_layer(train_layer), current_mode!=TRAIN);
    
    clear_info(&info);
    
    // Force redraw
    layer_mark_dirty(window_get_root_layer(main_window));
}

void next_slide() {
    vibes_long_pulse();
    current_slide++;
    if (current_slide <= *num_slides) {
//         update();
//         buzz_timer = app_timer_register(delays[current_slide], &buzz_timer_handler, NULL);
    } else {
        running = false;
        current_slide = *num_slides;
        set_info(&info, "FINISHED!", 0);
        current_epoch = ms_time_from_sec_ms(delays[current_slide], 0);
    }
}

void seconds_timer_handler(void *data) {
    // Gets called every few msecs
    
    if (running) {
        ms_time_t current_time = ms_time_get_wall_time();
        
        current_epoch = ms_time_subtract(current_time, last_time);
        
        if (current_mode == PRESENT) {
            if (settings.time_of_buzz_before_slide_ends > 0 && delays[current_slide] - current_epoch.time_sec == settings.time_of_buzz_before_slide_ends) {
                vibes_short_pulse();
            }

            if (current_epoch.time_sec >= delays[current_slide]) {
                // Time for current slide is up
                next_slide();
                last_time = current_time;
            }
        }
        update();
        
        seconds_timer = app_timer_register(1000 / FPS, &seconds_timer_handler, NULL);
    }
}

void reset_time() {
    // Reset current time
    last_time = ms_time_get_wall_time();
    current_epoch.time_sec = 0;
    current_epoch.time_ms = 0;
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    // On click of select button
    
    if (running) {
        running = false;
             
        if (current_mode == TRAIN) {
            // Store current epoch if training
            delays[current_slide] = current_epoch.time_sec;
        }
    } else {
        last_time = ms_time_subtract(ms_time_get_wall_time(), current_epoch);
        running = true;
        // Start seconds timer
        seconds_timer = app_timer_register(0, &seconds_timer_handler, NULL);
    }
    
    update();
}

void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
    // On long click of select button
    
    if (current_mode == TRAIN) {
        // Remove all delays if in training
        // TODO: Confirmation
        for (int i=1; i<=*num_slides; ++i) {
            delays[i] = 0;
        }
        *num_slides = 1;
        set_info(&info, "ALL CLEARED!", 1);
    } else {
        info.show = false;
    }
    
    reset_time();
        
    update();
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    // On click of up button
    
    if (current_mode == PRESENT) {
        // Advance one slide
        if (current_slide == *num_slides) {
            current_slide = 1;
        } else {
            current_slide++;
        }
    } else {
        if (running) {
            // Store the current epoch in the delays if running
            delays[current_slide] = current_epoch.time_sec;
        }
        if (current_slide == MAX_NUM_SLIDES) {
            //TODO: Handle
            set_info(&info, "MAX SLIDES!", 1);
        } else {
            current_slide++;
            if (*num_slides < current_slide) {
                *num_slides = current_slide;
            }
        }
    }
    
    reset_time();
    
    update();
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    // On click of down button
    
    // Go back one slide
    if (current_slide == 1) {
        current_slide = *num_slides;
    } else {
        current_slide--;
    }
    reset_time();
    update();
}

void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (!menu) {
        // Create menu layer
        menu = simple_menu_layer_create(layer_get_frame(window_get_root_layer(menu_window)), menu_window, menu_sections, NUM_MENU_SECTIONS, NULL);
        layer_add_child(window_get_root_layer(menu_window), simple_menu_layer_get_layer(menu));
    }
    window_stack_push(menu_window, true);
}

void back_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
    switch(click_number_of_clicks_counted(recognizer)) {
        case 2:
            running = false;
            info.show = false;
            reset_time();
            switch(current_mode) {
                case PRESENT: current_mode = TRAIN; break;
                case TRAIN: current_mode = PRESENT; break;
            }

            update();
            break;
        case 3:
            persist_data(delays, settings.NUM_SETTINGS);
            window_stack_pop(true);
            break;
    }
}

void config_provider(Window *window) {
    // Set the click configurations

    window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);

    window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);

    window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);

    window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
    window_multi_click_subscribe(BUTTON_ID_BACK, 2, 3, 750, true, back_multi_click_handler);
}

double get_progress() {
    double progress;
    progress = ((double) ms_time_to_sec(current_epoch)) / delays[current_slide];
    if (progress > 1) {
        progress = 1;
    }
    return progress;
}

void draw(Layer *my_layer, GContext* ctx) {
    // Set stroke and fill colour
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    
    // Draw progress bar border
    graphics_draw_rect(ctx, GRect(5, 0, 134, 20));

    // Draw progress bar progress
    graphics_fill_rect(ctx, GRect(5, 0, ((double) 134) * get_progress(), 20), 0, GCornerNone);
}

void refresh_menu() {
    display_style_menu_items[0].title = enum_to_string(DISPLAY_STYLE, settings.display_style);
    
    if (settings.time_of_buzz_before_slide_ends == 0) {
        snprintf(time_of_buzz_before_slide_ends_buf, BUF_SIZE, "Off");
    } else {
        snprintf(time_of_buzz_before_slide_ends_buf, BUF_SIZE, "%u Seconds", settings.time_of_buzz_before_slide_ends);   
    }
    pre_slide_buzz_time_menu_items[0].title = time_of_buzz_before_slide_ends_buf;
}

void display_style_click_handler(int index, void *context) {
    settings.display_style = (settings.display_style + 1) % 2;
    refresh_menu();
    persist_settings(&settings);
    layer_mark_dirty(simple_menu_layer_get_layer(menu));
}

void pre_slide_buzz_time_click_handler(int index, void *context) {
    settings.time_of_buzz_before_slide_ends = (settings.time_of_buzz_before_slide_ends + 10) % 70;
    refresh_menu();
    persist_settings(&settings);
    layer_mark_dirty(simple_menu_layer_get_layer(menu));
}

void init_menus() {
    display_style_menu_items[0] = (SimpleMenuItem) {
        .callback = display_style_click_handler 
    };
    
    menu_sections[0] = (SimpleMenuSection) {
        .num_items = 1,
        .title = "Display Style",
        .items = display_style_menu_items
    };
    
    pre_slide_buzz_time_menu_items[0] = (SimpleMenuItem) {
        .callback = pre_slide_buzz_time_click_handler 
    };
    
    menu_sections[1] = (SimpleMenuSection) {
        .num_items = 1,
        .title = "Buzz before next",
        .items = pre_slide_buzz_time_menu_items
    };
    
    refresh_menu();
}

void main_window_load() {
    // Create the titel layer
	title_layer = text_layer_create(GRect(0, 0, 144, 40));
	// Set the text, font, and text alignment for title layer
	text_layer_set_text(title_layer, "Tick Talk");
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(title_layer, GTextAlignmentCenter);
	// Add the title layer to the window
	layer_add_child(window_get_root_layer(main_window), text_layer_get_layer(title_layer));
    
    // Create the train layer
	train_layer = text_layer_create(GRect(0, 110, 144, 40));
	// Set the text, font, and text alignment for title layer
	text_layer_set_text(train_layer, "TRAINING");
	text_layer_set_font(train_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(train_layer, GTextAlignmentCenter);
	// Add the title layer to the window
	layer_add_child(window_get_root_layer(main_window), text_layer_get_layer(train_layer));
    
    // Create the draw layer
    draw_layer = bitmap_layer_create(GRect(0, 110, 144, 20));    
    // Set the draw layer's update routine
    layer_set_update_proc(bitmap_layer_get_layer(draw_layer), &draw);
    // Add the draw layer to the window
    layer_add_child(window_get_root_layer(main_window), bitmap_layer_get_layer(draw_layer));
    
    // Create inverter layers inside the draw layer
    invert_draw_layers[0] = inverter_layer_create(GRect(67, 5, 4, 10));
    invert_draw_layers[1] = inverter_layer_create(GRect(73, 5, 4, 10));
    // Add the inverter draw layers to the draw layer
    for (int i=0; i<NUM_INVERTER_LAYERS; ++i) {
        layer_add_child(bitmap_layer_get_layer(draw_layer), inverter_layer_get_layer(invert_draw_layers[i]));
    }
}

void main_window_appear() {
    update();
}

void handle_init(void) {
    
    // Load settings
    if (persist_exists(0)) {
        load_settings(&settings);
    }
    
	// Create the main window and set handlers
	main_window = window_create();
    window_set_window_handlers(main_window, (WindowHandlers) {
        .load = main_window_load,
        .appear = main_window_appear,
    });
    
    // Create the menu window
    menu_window = window_create();
    
	// Push the window
	window_stack_push(main_window, true);
    
    init_menus();
    
    // Init buffers
    info_buf = malloc(BUF_SIZE);
    time_buf = malloc(BUF_SIZE);
	    
    // Init global vars
    delays[0] = 1;
    current_slide = 1;
    current_mode=PRESENT;
    running = false;
    num_slides = &(delays[0]);
    
    if (persist_exists(settings.NUM_SETTINGS)) {
        // Load data
        load_data(delays, settings.NUM_SETTINGS);
    }
      
    // Set the click configuration provider
    window_set_click_config_provider(main_window, (ClickConfigProvider) config_provider);
    
    update();
}

void handle_deinit(void) {
    running = false;
    
    // Stop timer
    app_timer_cancel(seconds_timer);
    seconds_timer = NULL;
    
	// Destroy the layers
	text_layer_destroy(title_layer);
  	text_layer_destroy(info_layer);
   	text_layer_destroy(time_layer);
   	text_layer_destroy(train_layer);
    bitmap_layer_destroy(draw_layer);
    for (int i=0; i<NUM_INVERTER_LAYERS; ++i) {
        inverter_layer_destroy(invert_draw_layers[i]);
    }
    
    // Destroy buffers
    free(info_buf);
    info_buf = NULL;
    free(time_buf);
    time_buf = NULL;
	
	// Destroy the windows
    window_destroy(menu_window);
	window_destroy(main_window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
