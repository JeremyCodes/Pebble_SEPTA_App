/*
 * main.c
 * Sets up a Window and a TextLayer.
 */

#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_text_layer;

enum {
  KEY_STATION = 0,
  KEY_TIME = 1,
  KEY_TARDINESS = 2
};

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_text_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 5, window_bounds.size.h));
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_text_layer, "Updating Train Info...");
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_text_layer);
}


// Incoming messages arrive here for processing.
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Received Inbox message!");
  
  // Store incoming information
  static char station_buffer[32];
  static char time_buffer[32];
  static char tardiness_buffer[32];
  static char merged_buffer[256];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_STATION:
        snprintf(station_buffer, sizeof(station_buffer), "%s", t->value->cstring);
//        s_weather_updating = S_FALSE;
        break;
      case KEY_TIME:
        snprintf(time_buffer, sizeof(time_buffer), "%s", t->value->cstring);
//        s_weather_updating = S_FALSE;
        break;
      case KEY_TARDINESS:
        snprintf(tardiness_buffer, sizeof(tardiness_buffer), "%s", t->value->cstring);
//        s_weather_updating = S_FALSE;
        break;      
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }
    
    // Assemble full string and display
    snprintf(merged_buffer, sizeof(merged_buffer), "%s\n%s    %s", station_buffer, time_buffer, tardiness_buffer);
    text_layer_set_text(s_text_layer, merged_buffer);
   
    // Look for next item
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) 
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void init() {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  // Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
  
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}