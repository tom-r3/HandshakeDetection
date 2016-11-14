#include <pebble_worker.h>

#define REQUEST_LEAD_COUNT 0
#define SOURCE_BACKGROUND 1
#define RESET_LEAD_COUNT 2

// Need something to send foreground app
static uint16_t s_leads = 0;
// Change this to adjust the handshake sensivity
static int y_sensitivity = 800;
// Change this to adjust the allowed time between handshakes
static int min_handshake_interval = 2000;

static void accel_data_handler(AccelData* data, uint32_t num_samples) {
  // Calculate the acceleration
   int y_gesture = data[num_samples - 1].y - data[0].y;
  
  if(y_gesture > y_sensitivity){
    // Update leads
    s_leads++;
  
    // Construct a data packet
    AppWorkerMessage msg_data = {
      .data0 = s_leads
    };

    // Send the data to the foreground app
    app_worker_send_message(SOURCE_BACKGROUND, &msg_data);
    
    //sleep to avoid double counts
    psleep(min_handshake_interval);
  }
}

static void worker_message_handler(uint16_t type, AppWorkerMessage *message) {
  
  //this is used by the foreground app to request lead count
  if(type == REQUEST_LEAD_COUNT) {
      // Construct a data packet
      AppWorkerMessage msg_data = {
        .data0 = s_leads
      };

      // Send the data to the foreground app
      app_worker_send_message(SOURCE_BACKGROUND, &msg_data);
  }
  
    if(type == RESET_LEAD_COUNT) {
      // Reset the lead count
      s_leads = 0;
      
      // Notify foreground app to update window
      AppWorkerMessage msg_data = {
        .data0 = s_leads
      };
      app_worker_send_message(SOURCE_BACKGROUND, &msg_data);
  }
}

static void worker_init() {  
  // Subscribe to get AppWorkerMessages
  app_worker_message_subscribe(worker_message_handler);
  
  // Subscribe to accelerometer data
  accel_data_service_subscribe(5, accel_data_handler);
}

static void worker_deinit() {
  //do I need to unsubscribe from worker message here?
  
  // Unsubscribe from accelerometer data
  accel_data_service_unsubscribe();
}

int main(void) {
  worker_init();
  worker_event_loop();
  worker_deinit();
}