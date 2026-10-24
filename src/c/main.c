#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_text_layer;
static TextLayer *s_title_layer;
static TextLayer *s_unit_layer;
static TextLayer *s_caption_layer;
static Layer *s_dashboard_layer;
static GPath *s_heart_path;

static const GPathInfo HEART_PATH = {
    .num_points = 10,
    .points = (GPoint[]){
        {0, 8}, {-10, -2}, {-10, -7}, {-6, -10}, {-3, -10}, {0, -6}, {3, -10}, {6, -10}, {10, -7}, {10, -2}}};

static void dashboard_update_proc(Layer *layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(layer);
  int16_t center_x = bounds.size.w / 2;
  int16_t center_y = bounds.size.h / 2;
  int16_t inset = PBL_IF_ROUND_ELSE(22, 12);

  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack));
  graphics_fill_rect(ctx, GRect(inset, center_y - 44, bounds.size.w - inset * 2, 104), 12, GCornersAll);
  graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
  graphics_draw_line(ctx, GPoint(center_x - 16, center_y + 59),
                     GPoint(center_x + 16, center_y + 59));

  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorMelon, GColorWhite));
  gpath_move_to(s_heart_path, GPoint(center_x, center_y - 28));
  gpath_draw_filled(ctx, s_heart_path);
}

static TextLayer *create_label(Layer *parent, GRect frame, const char *text,
                               const char *font, GColor color)
{
  TextLayer *label = text_layer_create(frame);
  text_layer_set_background_color(label, GColorClear);
  text_layer_set_text_color(label, color);
  text_layer_set_font(label, fonts_get_system_font(font));
  text_layer_set_text_alignment(label, GTextAlignmentCenter);
  text_layer_set_text(label, text);
  layer_add_child(parent, text_layer_get_layer(label));
  return label;
}
#define SOURCE_FOREGROUND 0
#define SOURCE_BACKGROUND 1

const uint32_t inbox_size = 64;
const uint32_t outbox_size = 256;
static HealthValue health_value = 0;

// Key 2 carries the phone's WebSocket state; key 1 remains heart rate.
static void set_connection_status(bool connected)
{
  text_layer_set_text(s_caption_layer, connected ? "Connected" : "Disconnected");
  text_layer_set_text_color(s_caption_layer,
                            connected ? PBL_IF_COLOR_ELSE(GColorMintGreen, GColorWhite)
                                      : PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
}

static void inbox_received_handler(DictionaryIterator *iter, void *context)
{
  Tuple *status = dict_find(iter, 2);
  if (status)
  {
    set_connection_status(status->value->int32 == 1 &&
                          connection_service_peek_pebble_app_connection());
  }
}

static void phone_connection_handler(bool connected)
{
  // A phone connection alone does not confirm the relay is open.
  set_connection_status(false);
}

static void worker_message_handler(uint16_t type,
                                   AppWorkerMessage *message)
{




  uint16_t event_type = message->data0;
  health_value = message->data1;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "got messaged %d\n", event_type);

  static char s_buffer[32];

  snprintf(s_buffer, sizeof(s_buffer), "%d", (int)health_value);

  text_layer_set_text(s_text_layer, s_buffer);
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_int32(iter, event_type, health_value);

  app_message_outbox_send();

}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
  text_layer_set_text(s_text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
  text_layer_set_text(s_text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
  text_layer_set_text(s_text_layer, "Down");
}

static void click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  int16_t center_y = bounds.size.h / 2;
  int16_t title_y = PBL_IF_ROUND_ELSE(18, bounds.size.h >= 200 ? 22 : 6);
  window_set_background_color(window, GColorBlack);

  s_heart_path = gpath_create(&HEART_PATH);
  s_dashboard_layer = layer_create(bounds);
  layer_set_update_proc(s_dashboard_layer, dashboard_update_proc);
  layer_add_child(window_layer, s_dashboard_layer);

  s_title_layer = create_label(window_layer, GRect(0, title_y, bounds.size.w, 26),
                               "PEBBLEBEAT", FONT_KEY_GOTHIC_18_BOLD, GColorWhite);
  s_text_layer = create_label(window_layer,
                              GRect(0, center_y - 18, bounds.size.w, 54),
                              "--", FONT_KEY_BITHAM_42_BOLD, GColorWhite);
  s_unit_layer = create_label(window_layer,
                              GRect(0, center_y + 31, bounds.size.w, 22),
                              "BPM", FONT_KEY_GOTHIC_18_BOLD,
                              PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
  s_caption_layer = create_label(window_layer,
                                 GRect(0, bounds.size.h - PBL_IF_ROUND_ELSE(30, 25),
                                       bounds.size.w, 22),
                                 "Disconnected", FONT_KEY_GOTHIC_14,
                                 PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
  AppWorkerResult result = app_worker_launch();
}

static void main_window_unload(Window *window)
{
  text_layer_destroy(s_text_layer);
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_unit_layer);
  text_layer_destroy(s_caption_layer);
  layer_destroy(s_dashboard_layer);
  gpath_destroy(s_heart_path);
}

static void init(void)
{
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers){
                                                .load = main_window_load,
                                                .unload = main_window_unload,
                                            });
  window_stack_push(s_main_window, true);

  // Open AppMessage
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox_size, outbox_size);
  connection_service_subscribe((ConnectionHandlers){
      .pebble_app_connection_handler = phone_connection_handler});

  app_worker_message_subscribe(worker_message_handler);
}

static void deinit(void)
{
  connection_service_unsubscribe();
  app_message_deregister_callbacks();
  window_destroy(s_main_window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}
