#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_subtext_layer;
static TextLayer *s_battery_layer;


static void battery_callback(BatteryChargeState charge_state) {
  static char battery_text[] = "Some Text";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "Charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void update_time() {

  //get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  //write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%I.%M" : "%I.%M", tick_time);

  //diplay this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);

  //this is the dating game (sets the date)
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);

  //set the layer text to the date
  text_layer_set_text(s_date_layer, date_buffer);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}

static void main_window_load(Window *window) {

  //the root layer is now defined as the window_layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //this creates the text layer, notice how theres specify of if its a round watch
  s_time_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);

  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  //add to the window

  //this creates the subtext layer
  s_subtext_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(38, 32), bounds.size.w, 30));
  text_layer_set_text(s_subtext_layer, "Alex's Watch");
  text_layer_set_background_color(s_subtext_layer, GColorClear);
  text_layer_set_font(s_subtext_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_subtext_layer, GTextAlignmentCenter);

  //this creates the date layer
  s_date_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(98, 102), bounds.size.w, 30));
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  //2nd attempt at battery layer
  s_battery_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(18, 52), bounds.size.w, 30));

  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text(s_battery_layer, "charged");
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);


   battery_state_service_subscribe(battery_callback);
  //this creates the battery layer number

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_subtext_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));

  battery_callback(battery_state_service_peek());

}

static void main_window_unload(Window *window) {
    //gotta destroy each thing you create so the memory doesn't get bogged down.
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_subtext_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_battery_layer);
    tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();

}


static void init() {
  //create the main window and assign to to the pointer, s_main_window
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorTiffanyBlue);

  //subscribe for battery updates
  //registering the event service TickTimerService(the event being the constant time change)
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

 

  //set handlers to manage the elements inside the window
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });

  //now then this shows this window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  update_time();

}

static void deinit() {
  //when you leave the app, this'll kill it, giving memory back to the watch
  window_destroy(s_main_window);


}

//this is the main business. it initializes the whole thing, app event loop exists, and deinit.

int main(void) {
    init();
    app_event_loop();
    deinit();
}