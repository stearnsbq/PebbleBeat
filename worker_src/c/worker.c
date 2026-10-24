#include <pebble_worker.h>
#define SOURCE_FOREGROUND 0
#define SOURCE_BACKGROUND 1

static HealthValue heart_rate = 0;

static void worker_message_handler(uint16_t type, 
                                    AppWorkerMessage *message) {
  
}

static void prv_on_health_data(HealthEventType type, void *context) {
  

  // If the update was from the Heart Rate Monitor, query it
  if (type == HealthEventHeartRateUpdate) {
    heart_rate = health_service_peek_current_value(HealthMetricHeartRateBPM);
    AppWorkerMessage message = {
    .data0 = 1,
    .data1 = heart_rate
    };
    
    app_worker_send_message(SOURCE_BACKGROUND, &message);
  
  }else if (type == HealthEventHRVUpdate){
    uint16_t ppi = health_service_peek_hrv_ppi_ms();
    if (ppi > 0) {
        AppWorkerMessage message = {
        .data0 = 2,
        .data1 = ppi
        };

      app_worker_send_message(SOURCE_BACKGROUND, &message);
      // Display the peak-to-peak interval
    }
  }
}


static void prv_init() {
  bool success = health_service_set_heart_rate_sample_period(30);
  health_service_events_subscribe(prv_on_health_data, NULL);
  app_worker_message_subscribe(worker_message_handler);
}

static void prv_deinit() {
  // Deinitialize the worker here
}

int main(void) {
  prv_init();
  worker_event_loop();
  prv_deinit();
}