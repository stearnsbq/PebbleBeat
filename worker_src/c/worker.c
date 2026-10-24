#include <pebble_worker.h>
#define SOURCE_FOREGROUND 0
#define SOURCE_BACKGROUND 1

static HealthValue heart_rate = 0;
static HealthValue ppi = 0;

static void worker_message_handler(uint16_t type, 
                                    AppWorkerMessage *message) {
  
}

static void prv_on_health_data(HealthEventType type, void *context) {
  

  // If the update was from the Heart Rate Monitor, query it
  if (type == HealthEventHeartRateUpdate) {
    heart_rate = health_service_peek_current_value(HealthMetricHeartRateBPM);
    AppWorkerMessage message = {
    .data0 = heart_rate,
    .data1 = ppi
    };
    
    app_worker_send_message(SOURCE_BACKGROUND, &message);
  
  }else if (type == HealthEventHRVUpdate){
    ppi = health_service_peek_hrv_ppi_ms();
  }
}


static void prv_init() {
  health_service_set_heart_rate_sample_period(30);
  health_service_set_hrv_sample_period(10);
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