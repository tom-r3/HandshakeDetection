#include <pebble.h>

#define REQUEST_LEAD_COUNT 0
#define SOURCE_BACKGROUND 1
#define RESET_LEAD_COUNT 2

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_lead_layer;
static BitmapLayer *s_logo_layer;
static GBitmap *s_logo_bitmap;
static DictationSession *s_dictation_session;
static char s_last_text[512];
static char audio_text[512];

/******************** AppMessage **************************/

/*
 * send audio, add_lead and reset_leads reuse a lot of code, clean this up
 * define send reasons globally, have one function to send with an input argument
 *
 */
static void mobileapp_add_lead(){

  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  
  if(result == APP_MSG_OK) {
    // Add lead signal
    int value = 0;

    // Add an item to signify a new lead
    dict_write_int(out_iter, MESSAGE_KEY_NewLead, &value, sizeof(int), true);
    
    // Send this message
    result = app_message_outbox_send();
    /* AppMessageOutboxSent or AppMessageOutboxFailed callback will be called */
    /* Remember to write these later on! */
    
    // Check the result
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
        /* REMOVE THIS IN FINAL VERSION */
        static char s_buffer[32];
        snprintf(s_buffer, sizeof(s_buffer), "SendErr: %d", result);
        text_layer_set_text(s_lead_layer, s_buffer);
    }
  }
  else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
      /* REMOVE THIS IN FINAL VERSION */
      static char s_buffer[32];
      snprintf(s_buffer, sizeof(s_buffer), "PrepErr: %d", result);
      text_layer_set_text(s_lead_layer, s_buffer);
  }
}

static void mobileapp_reset_leads(){

  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  
  if(result == APP_MSG_OK) {
    // Reset lead signal
    int value = 1;

    // Add an item to signify a new lead
    dict_write_int(out_iter, MESSAGE_KEY_ResetLeads, &value, sizeof(int), true);
    
    // Send this message
    result = app_message_outbox_send();
    /* AppMessageOutboxSent or AppMessageOutboxFailed callback will be called */
    /* Remember to write these later on! */
    
    // Check the result
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
        /* REMOVE THIS IN FINAL VERSION */
        static char s_buffer[32];
        snprintf(s_buffer, sizeof(s_buffer), "SendErr: %d", result);
        text_layer_set_text(s_lead_layer, s_buffer);
    }
  }
  else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
      /* REMOVE THIS IN FINAL VERSION */
      static char s_buffer[32];
      snprintf(s_buffer, sizeof(s_buffer), "PrepErr: %d", result);
      text_layer_set_text(s_lead_layer, s_buffer);
  }
}

static void mobileapp_send_audio(){

  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  
  if(result == APP_MSG_OK) {

    // Add message
    dict_write_int(out_iter, MESSAGE_KEY_AudioCapture, &audio_text, sizeof(int), true);
    
    // Send this message
    result = app_message_outbox_send();
    /* AppMessageOutboxSent or AppMessageOutboxFailed callback will be called */
    /* Remember to write these later on! */
    
    // Check the result
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
        /* REMOVE THIS IN FINAL VERSION */
        static char s_buffer[32];
        snprintf(s_buffer, sizeof(s_buffer), "SendErr: %d", result);
        text_layer_set_text(s_lead_layer, s_buffer);
    }
  }
  else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
      /* REMOVE THIS IN FINAL VERSION */
      static char s_buffer[32];
      snprintf(s_buffer, sizeof(s_buffer), "PrepErr: %d", result);
      text_layer_set_text(s_lead_layer, s_buffer);
  }
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  // The message just sent has been successfully delivered
  /*
   * No sense in updating leads here, it is updated by worker message right before message
   * phone is called 
   *
   */
}

static void outbox_failed_callback(DictionaryIterator *iter,
                                      AppMessageResult reason, void *context) {
  // The message just sent failed to be delivered
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message send failed. Reason: %d", (int)reason);
  
  /* REMOVE THIS IN FINAL VERSION */
  static char s_buffer[32];
  snprintf(s_buffer, sizeof(s_buffer), "SendErr: %d", reason);
  text_layer_set_text(s_lead_layer, s_buffer);
}

/******************** Audio Capture ***********************/

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) {

  if(status == DictationSessionStatusSuccess) {

    // Send audio to phone
    mobileapp_send_audio(transcription);

    // Write text to audio_text variable
    snprintf(audio_text, sizeof(audio_text), "%s", transcription);
    
    // Display text on lead layer
    snprintf(s_last_text, sizeof(s_last_text), "%s", transcription);
    text_layer_set_text(s_lead_layer, s_last_text);

  } else {
    // Display the reason for any error
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Error ID: %d", (int)status);
    text_layer_set_text(s_lead_layer, s_failed_buff);

  }
}

/******************* Background Worker ********************/

static void update_leads(){
  // Construct a message to send
  AppWorkerMessage message = {
    .data0 = 0
  };

  // Send dummy data to background app to request a lead update
  app_worker_send_message(REQUEST_LEAD_COUNT, &message);
}

static void reset_leads(){
  // Construct a message to send
  AppWorkerMessage message = {
    .data0 = 0
  };

  // Send dummy data to background app to request a lead update
  app_worker_send_message(RESET_LEAD_COUNT, &message);
}

static void worker_launcher(){
  int result = app_worker_launch();
  /* 
   * need to implement error checking here, left for now to get basic functionality
   */
  APP_LOG(APP_LOG_LEVEL_INFO, "Result: %d", result);
}

/* Calls mobileapp_add_lead */
static void worker_message_handler(uint16_t type, AppWorkerMessage *data) {
  if(type == SOURCE_BACKGROUND) {     
    int leads = data->data0;

    // Show new lead to user in TextLayer
    static char s_buffer[32];
    snprintf(s_buffer, sizeof(s_buffer), "Leads: %d", leads);
    text_layer_set_text(s_lead_layer, s_buffer);
    
    // Message phone to update server
    mobileapp_add_lead();
  }
}

/******************** Click Handlers **********************/

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Start voice dictation UI
  text_layer_set_text(s_lead_layer, "AudioCapture");
  dictation_session_start(s_dictation_session);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Reset Lead Counter on Watch
  reset_leads();
  /* ADD CODE TO RESET LEAD COUNTER ON PHONE */
}

static void click_config_provider(void *context) {
  ButtonId id = BUTTON_ID_SELECT;  // The select button
  uint16_t delay_ms = 500;         // Minimum time pressed to fire

  // Click handlers for single press and long press
  window_single_click_subscribe(id, select_click_handler); //start audio capture
  window_long_click_subscribe(id, delay_ms, NULL, select_long_click_handler); //reset lead counter
}

/****************** Watchface *****************************/

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
      GRect(0, PBL_IF_ROUND_ELSE(10, 0), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Create the TextLayer with specific bounds
  s_lead_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(130, 130), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_lead_layer, GColorClear);
  text_layer_set_text_color(s_lead_layer, GColorBlack);
  // Ensure the correct number of leads are displayed when the app is relaunched
  update_leads(); 
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

/***************** App ************************************/

static void init() {
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Register the click config provider that starts dictation
  window_set_click_config_provider(s_main_window, click_config_provider);
  
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
  
  // Make sure the number of leads are displayed from the start
  text_layer_set_text(s_lead_layer, "Leads: 0");
  
  // Set up Appmessage
  const uint32_t inbox_size = 32;
  const uint32_t outbox_size = inbox_size;
  app_message_open(inbox_size, outbox_size);
  // Register to be notified about outbox events
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  
  // Create dictation session for audio capture
  s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
}

static void deinit() {
  // Destroy main window
  window_destroy(s_main_window);
  
  // No more worker updates
  app_worker_message_unsubscribe();
  
  // Free the last session data
  dictation_session_destroy(s_dictation_session);
  
  /*
   * Do I need to kill the background worker here?
   */
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}