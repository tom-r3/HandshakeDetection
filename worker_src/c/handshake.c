#include <pebble_worker.h>

#define WORKER_LEADS 0

static uint16_t s_leads = 0;

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // Update leads count
  s_leads++;

  // Construct a data packet
  AppWorkerMessage msg_data = {
    .data0 = s_leads
  };

  // Send the data to the foreground app
  app_worker_send_message(WORKER_LEADS, &msg_data);
}

static void worker_init() { 
  // Subscribe to tap events
  accel_tap_service_subscribe(accel_tap_handler);
}

static void worker_deinit() {
  // Unsubscribe from tap events
  accel_tap_service_unsubscribe();
}

int main(void) {
  worker_init();
  worker_event_loop();
  worker_deinit();
}