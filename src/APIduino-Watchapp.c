#include <pebble.h>

#define KEY_VERB 0 // 0 for GET, 1 for PUT; TODO: enum?
#define KEY_DATA 1
#define KEY_ENDPOINT 2 // 0: led; 1: tmp; 2: adc; 3: relay

static Window *window;
static TextLayer *s_debug_layer;
static TextLayer *s_led_layer;
static TextLayer *s_tmp_layer;
static TextLayer *s_adc_layer;
static TextLayer *s_relay_layer;

static char s_buffer[64];

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");

  // Get the data pair
  Tuple *data = dict_find(iterator, KEY_DATA);

  // Get the endpoint pair
  Tuple *endpoint = dict_find(iterator, KEY_ENDPOINT);

  if (data) {

    #ifdef PBL_COLOR
      text_layer_set_text_color(s_debug_layer, GColorCyan);
    #endif

    if (endpoint) {

      snprintf(s_buffer, sizeof(s_buffer), "R'd data: %d endpoint: %d", data->value->uint16, endpoint->value->uint16);

      // update UI
      switch (endpoint->value->uint16) 
      {
        static char text_buffer[6];

        // led
        case 0:
          // note that this is the requested, rather than "actual" value
          snprintf(text_buffer, sizeof(text_buffer), "%d", data->value->uint16);
          text_layer_set_text(s_led_layer, text_buffer);
          break;
        // tmp
        case 1:
          snprintf(text_buffer, sizeof(text_buffer), "%d°", data->value->uint16);
          text_layer_set_text(s_tmp_layer, text_buffer);
          break;
        // adc
        case 2:
          snprintf(text_buffer, sizeof(text_buffer), "%d", data->value->uint16);
          text_layer_set_text(s_adc_layer, text_buffer);
          break;
        // relay
        case 3:
          snprintf(text_buffer, sizeof(text_buffer), "%d", data->value->uint16);
          text_layer_set_text(s_relay_layer, text_buffer);
          break;
        default:
          // ?!
          #ifdef PBL_COLOR
            text_layer_set_text_color(s_debug_layer, GColorOrange);
          #endif
          break;
      }

    } else {
      snprintf(s_buffer, sizeof(s_buffer), "Rec'd data: '%d'", data->value->uint16); // Flip-Flop uses uint16_t
    }
    text_layer_set_text(s_debug_layer, s_buffer);
  }  else {    
    #ifdef PBL_COLOR
      text_layer_set_text_color(s_debug_layer, GColorOrange);
    #endif
    text_layer_set_text(s_debug_layer, "Unexpected message received!");
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
    text_layer_set_text_color(s_debug_layer, GColorBrightGreen);
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
  // *adding anyway due to some weirdness...
  dict_write_uint8(iter, KEY_DATA, 0); 
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

  // centered vertically for visibility on PTR
  s_debug_layer = text_layer_create((GRect) { .origin = { 0, (bounds.size.h - 16) / 2 }, .size = { bounds.size.w, 16 } });
  text_layer_set_font(s_debug_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_debug_layer, GColorBrightGreen);
  #else
    text_layer_set_text_color(s_debug_layer, GColorClear);
  #endif
  text_layer_set_background_color(s_debug_layer, GColorClear);
  text_layer_set_text_alignment(s_debug_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_debug_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_debug_layer, "Waiting...");
  layer_add_child(window_layer, text_layer_get_layer(s_debug_layer));


  // each box is now 70x56 + 2px padding in each direction from center
  // TODO: make all of these layout numbers constants for easy tweaking

  // above/left debug
  s_led_layer = text_layer_create((GRect) { .origin = { ((bounds.size.w / 2) - 70) - 2, (((bounds.size.h - 16) / 2) - 56) - 2 }, 
                                            .size = { 70, 56 } });
  text_layer_set_font(s_led_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_led_layer, GColorElectricBlue);
  #else
    text_layer_set_text_color(s_led_layer, GColorClear);
  #endif
  text_layer_set_background_color(s_led_layer, GColorClear);
  text_layer_set_text_alignment(s_led_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_led_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_led_layer, "?");
  layer_add_child(window_layer, text_layer_get_layer(s_led_layer));

  // above/right
  s_tmp_layer = text_layer_create((GRect) { .origin = { (bounds.size.w / 2) + 2, (((bounds.size.h - 16) / 2) - 56) - 2 }, 
                                            .size = { 70, 56 } });
  text_layer_set_font(s_tmp_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_tmp_layer, GColorYellow);
  #else
    text_layer_set_text_color(s_tmp_layer, GColorClear);
  #endif
  text_layer_set_background_color(s_tmp_layer, GColorClear);
  text_layer_set_text_alignment(s_tmp_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_tmp_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_tmp_layer, "??°");
  layer_add_child(window_layer, text_layer_get_layer(s_tmp_layer));

  // below/left; note tweaked GRect due to different font/value size
  s_adc_layer = text_layer_create((GRect) { .origin = { ((bounds.size.w / 2) - 70 - 4) - 2, ((bounds.size.h + 16) / 2) + 2 + 6 }, 
                                            .size = { 70 + 4, 56 - 12 } });
  // this value is longer, so use smaller font
  text_layer_set_font(s_adc_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_adc_layer, GColorMagenta);
  #else
    text_layer_set_text_color(s_adc_layer, GColorClear);
  #endif
  text_layer_set_background_color(s_adc_layer, GColorClear);
  text_layer_set_text_alignment(s_adc_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_adc_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_adc_layer, "????");
  layer_add_child(window_layer, text_layer_get_layer(s_adc_layer));

  // below/right
  s_relay_layer = text_layer_create((GRect) { .origin = { (bounds.size.w / 2) + 2, ((bounds.size.h + 16) / 2) + 2 }, 
                                            .size = { 70, 56 } });
  text_layer_set_font(s_relay_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_relay_layer, GColorVividCerulean);
  #else
    text_layer_set_text_color(s_relay_layer, GColorClear);
  #endif
  text_layer_set_background_color(s_relay_layer, GColorClear);
  text_layer_set_text_alignment(s_relay_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_relay_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_relay_layer, "?");
  layer_add_child(window_layer, text_layer_get_layer(s_relay_layer));

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
  window_set_background_color(window, GColorBlack);
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
