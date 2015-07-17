#include <pebble.h>
	
static Window *s_main_window;
static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_connection_layer;
static TextLayer *s_connection_bg_layer;
static TextLayer *s_hacking_layer;
static TextLayer *s_date_layer;

static GFont s_time_font;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static BitmapLayer *s_bluetooth_layer;
static GBitmap *s_bluetooth_bitmap;

static BitmapLayer *s_battery_layer;
static GBitmap *s_battery_bitmap;

static const char CONNECTION_TEXT[] = " CONNECTION IS POWER";
static GColor s_connection_bar_color;

static void bt_handler(bool connected) {
	layer_set_hidden(bitmap_layer_get_layer(s_bluetooth_layer), !connected); 
	if (!connected){
		vibes_short_pulse();
	}
}

static void battery_handler(BatteryChargeState charge_state) { 
	int16_t value = 0;
	value = (122*charge_state.charge_percent)/100+10;
  layer_set_hidden(bitmap_layer_get_layer(s_battery_layer), !charge_state.is_charging || charge_state.charge_percent == 100);
	#ifdef PBL_COLOR
  if(charge_state.charge_percent<=25){
		s_connection_bar_color = GColorDarkCandyAppleRed;
		text_layer_set_background_color(s_connection_bg_layer, GColorBulgarianRose);
	}else{
    if(!charge_state.is_charging){
		  s_connection_bar_color = GColorCyan;
		  text_layer_set_background_color(s_connection_bg_layer, GColorTiffanyBlue);
    }else{
      s_connection_bar_color = GColorChromeYellow;
		  text_layer_set_background_color(s_connection_bg_layer, GColorWindsorTan);
    }
	}
  #endif
  
  text_layer_set_size(s_connection_layer, GSize(value, 18));
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char hourText[] = "00";
  static char minuteText[] = "00";
  static char dateText[] = "MON_01_01_2001";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(hourText, sizeof("00"), "%H", tick_time);
  } else {
    //Use 12 hour format
    strftime(hourText, sizeof("00"), "%I", tick_time);	
  }
	
  strftime(minuteText, sizeof("00"), "%M", tick_time);
  
  // Write current date into the buffer
  strftime(dateText, sizeof("MON_01_01_2001"), "%a_%d_%m_%Y", tick_time);

  // Display this time and date on the TextLayer
  text_layer_set_text(s_hour_layer, hourText);
  text_layer_set_text(s_minute_layer, minuteText);
  text_layer_set_text(s_date_layer, dateText);	
}

static void text_update_proc(Layer* layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, s_connection_bar_color);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 18), 0, GCornerNone);
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, CONNECTION_TEXT, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0,0,144,18), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

static void main_window_load(Window *window) {
	//Create background bitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DEDSEC_LOGO);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
	
	//Create BT bitmap, then set to created BitmapLayer
  s_bluetooth_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SIGNAL);
  s_bluetooth_layer = bitmap_layer_create(GRect(0, 144, 20, 20));  
  bitmap_layer_set_bitmap(s_bluetooth_layer, s_bluetooth_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bluetooth_layer));
	
	//Create Battery bitmap, then set to created BitmapLayer
  s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);
  s_battery_layer = bitmap_layer_create(GRect(122, 123, 22, 18));
	bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_layer));
		
	// Create time TextLayer
  s_hour_layer = text_layer_create(GRect(0, -5, 60, 84));
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorWhite);
  text_layer_set_text(s_hour_layer, "00");

  s_minute_layer = text_layer_create(GRect(0, 55, 60, 84));
  text_layer_set_background_color(s_minute_layer, GColorClear);
  text_layer_set_text_color(s_minute_layer, GColorWhite);
  text_layer_set_text(s_minute_layer, "00");
  
  	//Create GFont
  	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WATCHDOGS_SUBSET_58));

  	//Apply to TextLayer
  	text_layer_set_font(s_hour_layer, s_time_font);
  	text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
	
	  text_layer_set_font(s_minute_layer, s_time_font);
  	text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);

  	// Add it as a child layer to the Window's root layer
  	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer));
	  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_minute_layer));
	
	//Create date text layer
	s_date_layer = text_layer_create(GRect(0, 150, 144, 18));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "MON_01_01_2001");
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
	text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
    	
	//Create connection background layer
	s_connection_bg_layer = text_layer_create(GRect(0, 123, 122, 18));	
	#ifdef PBL_COLOR
	  text_layer_set_background_color(s_connection_bg_layer, GColorTiffanyBlue);
	#else
	  text_layer_set_background_color(s_connection_bg_layer, GColorClear);
	#endif
  text_layer_set_text_color(s_connection_bg_layer, GColorWhite);
  text_layer_set_text(s_connection_bg_layer, CONNECTION_TEXT);
	text_layer_set_font(s_connection_bg_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_connection_bg_layer));
	
	//Create connection text layer
	s_connection_layer = text_layer_create(GRect(0, 123, 122, 18));
  text_layer_set_text(s_connection_layer, CONNECTION_TEXT);
  s_connection_bar_color = GColorWhite;
  layer_set_update_proc((Layer*) s_connection_layer, text_update_proc);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_connection_layer));
	
	//Create hacking text layer
	s_hacking_layer = text_layer_create(GRect(0, 137, 144, 18));
  text_layer_set_background_color(s_hacking_layer, GColorClear);
  text_layer_set_text_color(s_hacking_layer, GColorWhite);
  text_layer_set_text(s_hacking_layer, "_hacking is our weapon");
	text_layer_set_text_alignment(s_hacking_layer, GTextAlignmentRight);	
	text_layer_set_font(s_hacking_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hacking_layer));
	
	// Make sure the time is displayed from the start
  update_time();
	
	// Show current connection state
  bt_handler(bluetooth_connection_service_peek());
	
	// Show current battery state
  battery_handler(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
	//Unsuscribe from services
	tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();
	// Destroy GBitmap
	gbitmap_destroy(s_background_bitmap);
	gbitmap_destroy(s_bluetooth_bitmap);
	gbitmap_destroy(s_battery_bitmap);
	// Destroy BitmapLayer
	bitmap_layer_destroy(s_background_layer);
	bitmap_layer_destroy(s_bluetooth_layer);
	bitmap_layer_destroy(s_battery_layer);
	//Destroy text layers
	text_layer_destroy(s_hour_layer);
	text_layer_destroy(s_minute_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_connection_layer);
	text_layer_destroy(s_connection_bg_layer);
	text_layer_destroy(s_hacking_layer);
}

static void second_handler(struct tm *tick_time, TimeUnits units_changed) {
    if (strcmp(text_layer_get_text(s_hacking_layer),"_hacking is our weapon")==0){
		text_layer_set_text(s_hacking_layer, "hacking is our weapon");
	}else{
		text_layer_set_text(s_hacking_layer, "_hacking is our weapon");
	}
	
	update_time();
	battery_handler(battery_state_service_peek());
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  //window_set_fullscreen(s_main_window, true);
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, second_handler);
  //tick_timer_service_subscribe(MINUTE_UNIT, minute_handler);  
	
  // Subscribe to Bluetooth updates
  bluetooth_connection_service_subscribe(bt_handler);
  
  //Subscribe to Battery updates
  battery_state_service_subscribe(battery_handler);
}

static void deinit() {
	// Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}