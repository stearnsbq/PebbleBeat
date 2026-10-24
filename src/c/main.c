#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_text_layer;
#define SOURCE_FOREGROUND 0
#define SOURCE_BACKGROUND 1


const uint32_t inbox_size = 64;
const uint32_t outbox_size = 256;
static HealthValue heart_rate = 0;

static void worker_message_handler(uint16_t type, 
                                    AppWorkerMessage *message) {
  

  APP_LOG(APP_LOG_LEVEL_DEBUG, "got messaged %d\n", type);
    // Get the data, only if it was sent from the background
    heart_rate = message->data0;
    
    static char s_buffer[32];
    
    // Format the string inside your update function or event handler
    snprintf(s_buffer, sizeof(s_buffer), "BPM: %d", (int)heart_rate);
    
    // Update the Pebble TextLayer
    text_layer_set_text(s_text_layer, s_buffer);
  
    
      DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    // Write key 1 (matching KEY_BUTTON_PRESSED in package.json) with value 5
    dict_write_int32(iter, 1, heart_rate); 
    
    app_message_outbox_send();
    
  
  
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(s_text_layer, "Press a button");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  AppWorkerResult result = app_worker_launch();
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void init(void) {
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
    

  // Open AppMessage
  app_message_open(inbox_size, outbox_size);
  

  
  
  app_worker_message_subscribe(worker_message_handler);
  
}

static void deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
