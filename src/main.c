#include <pebble.h>
#include <string.h>

#define WEATHER_LAYER_BUFFER_SIZE 32

#define DEBUG S_TRUE

#define WEATHER_UPDATE_INTERVAL  30
#define SEPTA_UPDATE_INTERVAL    10
  
enum {
  KEY_TEMPERATURE = 2,
  KEY_CONDITIONS = 1
};

static Window *s_main_window;
static Layer *s_canvas_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_septa_layer1;
static TextLayer *s_septa_layer2;

static int s_weather_updating;

static void update_time()
{
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

  // Create two long-lived buffer and a pointer to reference;
  static char date_buffer_1_digit[] = "0";
  static char date_buffer_2_digit[] = "00";
  static char *date_buffer[2];
  static char long_buffer[] = "00:00";
  static char short_buffer[] = "0:00";
  static char *buffer[6]; //[5];
  *buffer = long_buffer;
  *date_buffer = date_buffer_2_digit;

	// Write the current hours and minutes into the buffer
	if(clock_is_24h_style() == true) 
	{
	  // Use 24 hour format
		strftime(long_buffer, sizeof("00:00"), "%H:%M", tick_time);
		*buffer = long_buffer;
	} 
	else // Use 12 hour format
	{
		strftime(long_buffer, sizeof("00:00"), "%I:%M", tick_time);
		
		*buffer = long_buffer;
		
		if (long_buffer[0] == '0')
		{
			short_buffer[0] = long_buffer[1];
			short_buffer[2] = long_buffer[3];
			short_buffer[3] = long_buffer[4];
			*buffer = short_buffer;
		}
	}	 
	// Display this time on the TextLayer
	text_layer_set_text(s_time_layer, *buffer);
  
  strftime(date_buffer_2_digit, sizeof(date_buffer_2_digit), "%d", tick_time);
  if (date_buffer_2_digit[0] == '0')
  {
    date_buffer_1_digit[0] = date_buffer_2_digit[1];
    *date_buffer = date_buffer_1_digit;
  }
  text_layer_set_text(s_date_layer, *date_buffer);
}

static void canvas_update_proc(Layer *this_layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(this_layer);

  // get a background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
  
  // Draw Bisecting line
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, ((bounds.size.h/2) -1), 144, 2), 0, GCornerNone);  
  
  // Draw the 'border'
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(8, 53, 128, 59), 0, GCornerNone);
}

static void main_window_load(Window *window) 
{
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create Layer
  s_canvas_layer = layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  layer_add_child(window_layer, s_canvas_layer);

  // Set the update_proc
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  
  // Add Time layer
	s_time_layer = text_layer_create(GRect(10, 55, 124, 55));
	text_layer_set_background_color(s_time_layer, GColorBlack);
	text_layer_set_text_color(s_time_layer, GColorClear);
	
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // Add Date Layer
	s_date_layer = text_layer_create(GRect(119, 95, 15, 15));
	text_layer_set_background_color(s_date_layer, GColorBlack);
	text_layer_set_text_color(s_date_layer, GColorClear);
	
	text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
	// Add weather text layer
	s_weather_layer = text_layer_create(GRect(0, 12, 144, 30));
	text_layer_set_background_color(s_weather_layer, GColorClear);
	text_layer_set_text_color(s_weather_layer, GColorWhite);
	text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
	text_layer_set_text(s_weather_layer, "Updating...");
	
	text_layer_set_font(s_weather_layer,  fonts_get_system_font(FONT_KEY_GOTHIC_24));
	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
	// Add SEPTA text layers
	s_septa_layer1 = text_layer_create(GRect(0, 118, 144, 30));
	text_layer_set_background_color(s_septa_layer1, GColorClear);
	text_layer_set_text_color(s_septa_layer1, GColorWhite);
	text_layer_set_text_alignment(s_septa_layer1, GTextAlignmentLeft);
	text_layer_set_text(s_septa_layer1, " JEF> 00:00a On Time");
	
	text_layer_set_font(s_septa_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_septa_layer1));
  
  s_septa_layer2 = text_layer_create(GRect(0, 135, 144, 30));
	text_layer_set_background_color(s_septa_layer2, GColorClear);
	text_layer_set_text_color(s_septa_layer2, GColorWhite);
	text_layer_set_text_alignment(s_septa_layer2, GTextAlignmentLeft);
	text_layer_set_text(s_septa_layer2, " JEF>   6:00a On Time");
	
	text_layer_set_font(s_septa_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_septa_layer2));
}

static void main_window_unload(Window *window) 
{
	text_layer_destroy(s_septa_layer1);
  text_layer_destroy(s_septa_layer2);
	text_layer_destroy(s_weather_layer);
	text_layer_destroy(s_time_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	update_time();
//  char new_text_buffer[32];
  
  // Get weather update every 30 minutes
  if ( ((tick_time->tm_min % WEATHER_UPDATE_INTERVAL) == 0) || 
       (s_weather_updating) )
  {
    APP_LOG(APP_LOG_LEVEL_INFO, "Weather updating...");
    // Mark for update
    //snprintf(new_text_buffer, (3 + sizeof(text_layer_get_text(s_weather_layer))), "%s(u)", text_layer_get_text(s_weather_layer)) ;
//    text_layer_set_text(s_weather_layer, "Updating...");
    
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    s_weather_updating = S_TRUE;
    app_message_outbox_send();
  }
}


// Incoming messages arrive here for processing.
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Received Inbox message!");
  
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[WEATHER_LAYER_BUFFER_SIZE];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_TEMPERATURE:
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)t->value->int32);
        s_weather_updating = S_FALSE;
        break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        s_weather_updating = S_FALSE;
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }
    
    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
    
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


static void init() 
{
  s_weather_updating = S_TRUE;
	s_main_window = window_create();
	
	window_set_window_handlers(s_main_window, 
												(WindowHandlers) 
												{
													.load  = main_window_load,
													.unload = main_window_unload
												});
	window_stack_push(s_main_window, true);
	update_time();
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

}

static void deinit() 
{
	window_destroy(s_main_window);
}

int main(void) 
{
	init();
	app_event_loop();
	deinit();
}
