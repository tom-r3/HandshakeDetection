#include <pebble_worker.h>

#define WORKER_LEADS 0

static uint16_t s_leads = 0;

static void tick_handler(struct tm *tick_timer, TimeUnits units_changed) {
  // Update value
  s_leads++;

  // Construct a data packet
  AppWorkerMessage msg_data = {
    .data0 = s_leads
  };

  // Send the data to the foreground app
  app_worker_send_message(WORKER_LEADS, &msg_data);
}

static void worker_init() {
  // Use the TickTimer Service as a data source
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void worker_deinit() {
  // Stop using the TickTimerService
  tick_timer_service_unsubscribe();
}

int main(void) {
  worker_init();
  worker_event_loop();
  worker_deinit();
}