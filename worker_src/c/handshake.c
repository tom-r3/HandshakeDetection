#include <pebble_worker.h>

#define SOURCE_FOREGROUND 0
#define SOURCE_BACKGROUND 1

// Need something to send foreground app
static uint16_t s_leads = 0;

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // Update leads
  s_leads++;
  
  
  // Construct a data packet
  AppWorkerMessage msg_data = {
    .data0 = s_leads
  };

  // Send the data to the foreground app
  app_worker_send_message(SOURCE_BACKGROUND, &msg_data);
  
}

static void worker_message_handler(uint16_t type, 
                                    AppWorkerMessage *message) {
  
  //this is used by the foreground app to request lead count
  if(type == SOURCE_FOREGROUND) {
      // Construct a data packet
      AppWorkerMessage msg_data = {
        .data0 = s_leads
      };

      // Send the data to the foreground app
      app_worker_send_message(SOURCE_BACKGROUND, &msg_data);
  }
}

static void worker_init() {  
  // Subscribe to get AppWorkerMessages
  app_worker_message_subscribe(worker_message_handler);
  
  // Subscribe to tap events
  accel_tap_service_subscribe(accel_tap_handler);
}

static void worker_deinit() {
  //do I need to unsubscribe from worker message here?
  
  // Unsubscribe from tap events
  accel_tap_service_unsubscribe();
}

int main(void) {
  worker_init();
  worker_event_loop();
  worker_deinit();
}