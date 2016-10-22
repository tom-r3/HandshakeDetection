#include <pebble.h>

#define WORKER_LEADS 0

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_lead_layer;
static BitmapLayer *s_logo_layer;
static GBitmap *s_logo_bitmap;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void worker_launcher(){
  int result = app_worker_launch();
  /* 
   * need to implement error checking here, left for now to get basic functionality
   */
  APP_LOG(APP_LOG_LEVEL_INFO, "Result: %d", result);
}

static void worker_message_handler(uint16_t type, AppWorkerMessage *data) {
  if(type == WORKER_LEADS) { 
    // Read lead info from worker's packet
    int leads = data->data0;

    // Show to user in TextLayer
    static char s_buffer[32];
    snprintf(s_buffer, sizeof(s_buffer), "Leads: %d", leads);
    text_layer_set_text(s_lead_layer, s_buffer);
  }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create GBitmap
  s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BITMAP_LOGO);

  // Create BitmapLayer to display the GBitmap
  s_logo_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto the layer
  bitmap_layer_set_bitmap(s_logo_layer, s_logo_bitmap);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(6, 0), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Create the TextLayer with specific bounds
  s_lead_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(120, 130), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_lead_layer, GColorClear);
  text_layer_set_text_color(s_lead_layer, GColorBlack);
  text_layer_set_text(s_lead_layer, "Leads: 0");
  text_layer_set_font(s_lead_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_lead_layer, GTextAlignmentCenter);

  // Add logo to window
  layer_add_child(window_layer, bitmap_layer_get_layer(s_logo_layer));
  // Add time as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  // Add leads as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_lead_layer));
  
}

static void main_window_unload(Window *window) {
  // Destroy Time TextLayer
  text_layer_destroy(s_time_layer);
  
  // Destroy Leads TextLayer
  text_layer_destroy(s_lead_layer);
  
  // Destroy GBitmap
  gbitmap_destroy(s_logo_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_logo_layer);
}

static void init() {
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Launch the background worker that monitors the accelerometer
  worker_launcher();
  
  // Subscribe to Worker messages
  app_worker_message_subscribe(worker_message_handler);
}

static void deinit() {
  // Destroy main window
  window_destroy(s_main_window);
  
  // No more worker updates
  app_worker_message_unsubscribe();
  
  /*
   * Do I need to kill the background worker here?
   */
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}