#include <pebble.h>

Window *window;
TextLayer *hhmm_layer, *ss_layer, *ddmm_layer;
BitmapLayer *background_layer, *radio_layer;

static GBitmap *background, *radio;
static GFont digitS, digitL;

char hhmmBuffer[] = "00:00";
char ssBuffer[] = "00";
char ddmmBuffer[] = "00~00";

void bluetooth_connection_handler(bool connected)
{
	//layer_set_hidden(bitmap_layer_get_layer(radio_layer), connected != true);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	int seconds = tick_time->tm_sec;

	strftime(ssBuffer, sizeof(ssBuffer), "%S", tick_time);
	text_layer_set_text(ss_layer, ssBuffer);

	if(seconds == 0 || units_changed == MINUTE_UNIT)
	{
		if(clock_is_24h_style())
			strftime(hhmmBuffer, sizeof(hhmmBuffer), "%H:%M", tick_time);
		else
			strftime(hhmmBuffer, sizeof(hhmmBuffer), "%I:%M", tick_time);
		
		text_layer_set_text(hhmm_layer, hhmmBuffer);
		
		strftime(ddmmBuffer, sizeof(ddmmBuffer), "%d %m", tick_time);
		text_layer_set_text(ddmm_layer, ddmmBuffer);
	}
}

void window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);

	//Init Background
	background = gbitmap_create_with_resource(RESOURCE_ID_RESOURCE_ID_IMAGE_BACKGROUND);
	background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_background_color(background_layer, GColorClear);
	bitmap_layer_set_bitmap(background_layer, background);
	layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
	
	digitS = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_RESOURCE_ID_FONT_DIGITAL_25));
	digitL = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_RESOURCE_ID_FONT_DIGITALTHIN_55));
  
	//DAY+MONTH layer
	ddmm_layer = text_layer_create(GRect(3, 5, 35, 37));
	text_layer_set_background_color(ddmm_layer, GColorClear);
	text_layer_set_text_color(ddmm_layer, GColorBlack);
	text_layer_set_text_alignment(ddmm_layer, GTextAlignmentCenter);
	text_layer_set_font(ddmm_layer, digitS);
	layer_add_child(window_layer, text_layer_get_layer(ddmm_layer));
        
	//HOUR+MINUTE layer
	hhmm_layer = text_layer_create(GRect(0, 50, 100, 125));
	text_layer_set_background_color(hhmm_layer, GColorBlack);
	text_layer_set_text_color(hhmm_layer, GColorWhite);
	text_layer_set_text_alignment(hhmm_layer, GTextAlignmentCenter);
	text_layer_set_font(hhmm_layer, digitL);
	//layer_add_child(window_layer, text_layer_get_layer(hhmm_layer));
        
	//SECOND layer
	ss_layer = text_layer_create(GRect(100, 5, 140, 37));
	text_layer_set_background_color(ss_layer, GColorBlack);
	text_layer_set_text_color(ss_layer, GColorWhite);
	text_layer_set_text_alignment(ss_layer, GTextAlignmentCenter);
	text_layer_set_font(ss_layer, digitS);
	layer_add_child(window_layer, text_layer_get_layer(ss_layer));
        
	//Init bluetooth radio
	radio = gbitmap_create_with_resource(RESOURCE_ID_RESOURCE_ID_IMAGE_RADIO);
	radio_layer = bitmap_layer_create(GRect(111, 133, 144, 168)); //31x33
	bitmap_layer_set_background_color(radio_layer, GColorClear);
	bitmap_layer_set_bitmap(radio_layer, radio);
	//bitmap_layer_set_compositing_mode(radio_layer, GCompOpAnd);
	layer_add_child(window_layer, bitmap_layer_get_layer(radio_layer));

	//Get a time structure so that it doesn't start blank
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);

	//Manually call the tick handler when the window is loading
	tick_handler(t, MINUTE_UNIT);
	
	bool connected = bluetooth_connection_service_peek();
	bluetooth_connection_handler(connected);
}
 
void window_unload(Window *window)
{
	//Destroy text layers
	text_layer_destroy(ddmm_layer);
	text_layer_destroy(ss_layer);
	text_layer_destroy(hhmm_layer);
	
	//Unload Fonts
	fonts_unload_custom_font(digitS);
	fonts_unload_custom_font(digitL);
	
	//Destroy GBitmaps
	gbitmap_destroy(radio);
	gbitmap_destroy(background);

	//Destroy BitmapLayers
	bitmap_layer_destroy(radio_layer);
	bitmap_layer_destroy(background_layer);
}

void handle_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
    window_stack_push(window, true);
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler)tick_handler);
	bluetooth_connection_service_subscribe(&bluetooth_connection_handler);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}