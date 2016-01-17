#include <pebble.h>

#define KEY_VERB 0 // 0 for GET, 1 for PUT; TODO: enum?
#define KEY_DATA 1
#define KEY_ENDPOINT 2 // 0: led; 1: tmp; 2: adc; 3: relay

static Window *window;
static TextLayer *s_debug_layer;

static char s_buffer[64];

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");

  // Get the first pair
  Tuple *data = dict_find(iterator, KEY_DATA);
  if (data) {
    snprintf(s_buffer, sizeof(s_buffer), "Received '%d'", data->value->uint16); // Flip-Flop uses uint16_t
    #ifdef PBL_COLOR
      text_layer_set_text_color(s_debug_layer, GColorCyan);
    #endif
    text_layer_set_text(s_debug_layer, s_buffer);
  }  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_debug_layer, GColorOrange);
  #endif
  text_layer_set_text(s_debug_layer, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_debug_layer, GColorOrange);
  #endif
  text_layer_set_text(s_debug_layer, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_debug_layer, GColorOrange);
  #endif
  text_layer_set_text(s_debug_layer, "Outbox send success!");
}


// TODO: message sending fn

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_debug_layer, GColorMagenta);
  #endif
  text_layer_set_text(s_debug_layer, "Select");

  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, KEY_VERB, 0); // GET
  // (KEY_DATA unused for GET)
  //dict_write_uint8(iter, KEY_ENDPOINT, 0); // led
  //dict_write_uint8(iter, KEY_ENDPOINT, 1); // tmp - won't work since not int/valid
  //dict_write_uint8(iter, KEY_ENDPOINT, 2); // adc
  dict_write_uint8(iter, KEY_ENDPOINT, 3); // relay
  // KEY_ENDPOINT: 0: led; 1: tmp; 2: adc; 3: relay


  // Send the message!
  app_message_outbox_send();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_debug_layer, GColorMagenta);
  #endif
  text_layer_set_text(s_debug_layer, "Up");

  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, KEY_VERB, 1); // PUT
  dict_write_uint8(iter, KEY_DATA, 1); // ON
  dict_write_uint8(iter, KEY_ENDPOINT, 0); // led

  // Send the message!
  app_message_outbox_send();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_debug_layer, GColorMagenta);
  #endif
  text_layer_set_text(s_debug_layer, "Down");

  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, KEY_VERB, 1); // PUT
  dict_write_uint8(iter, KEY_DATA, 0); // OFF
  dict_write_uint8(iter, KEY_ENDPOINT, 0); // led

  // Send the message!
  app_message_outbox_send();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_debug_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h - 16 }, .size = { bounds.size.w - 0, 16 } });
  text_layer_set_font(s_debug_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_debug_layer, GColorBrightGreen);
  #else
    text_layer_set_text_color(s_debug_layer, GColorWhite);
  #endif
  text_layer_set_background_color(s_debug_layer, GColorBlack);
  text_layer_set_text_alignment(s_debug_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_debug_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_debug_layer, "Waiting...");
  layer_add_child(window_layer, text_layer_get_layer(s_debug_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_debug_layer);
}

static void init(void) {
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage with sensible buffer sizes
  app_message_open(64, 64);
  // TODO: ^^^ unsure why small (accurate?!) values fail after several messages?

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
