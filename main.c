#include <pebble.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define THRES 570
#define KEY_DATA 1
  
static Window *s_main_window;
static TextLayer *s_output_layer;


void send_alert_to_phone(){
  DictionaryIterator *iter;
  
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_DATA, 1);
  app_message_outbox_send();
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Msg Sent!");
}
void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  //send alert to phone via bluetooth
  //Window *window = (Window *)context;
  send_alert_to_phone();
}

void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  //recv ACK from phone and print 
  
}

int16_t max(int16_t a_in, int16_t b_in, int16_t c_in ){
  int16_t a = abs(a_in);
  int16_t b = abs(b_in);
  int16_t c = abs(c_in);
  if (a> b){
    if(c>a) return c;
    else return a;
  }
  else{
    if(b>c)return b;
    else return c;
  }
} 

bool isFalling(AccelData *data) {
  if (max(data[0].x, data[0].y, data[0].z)<THRES){
    return true;
  }
  
  return false;
}
/*
static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  static char s_buffer[128];
  if (isFalling(data)) {
    // Compose string of all data
    snprintf(s_buffer, sizeof(s_buffer), 
         "X,Y,Z\n %d,%d,%d", 
	      data[0].x, data[0].y, data[0].z
	           );
  
    //Show the data
    text_layer_set_text(s_output_layer, s_buffer);
  } else {
    text_layer_set_text(s_output_layer, "Not falling\n");
  }
  //memset(s_buffer, 0, sizeof(s_buffer));
}
*/

static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  static char s_buffer[128];

  // Compose string of all data for 3 samples
  // double g=sqrt(static_cast<double>(data[0].x*data[0].x+data[0].y*data[0].y+data[0].z*data[0].z));
  snprintf(s_buffer, sizeof(s_buffer), 
	   "N X,Y,Z\n0 %d,%d,%d\n1 %d,%d,%d\n2 %d,%d,%d\n", 
	   data[0].x, data[0].y, data[0].z, 
	   data[1].x, data[1].y, data[1].z, 
	   data[2].x, data[2].y, data[2].z
	   );
  

  //APP_LOG(APP_LOG_LEVEL_INFO, "time=%d, X=%d, Y=%d, Z=%d ",(int)data[0].timestamp, data[0].x, data[0].y, data[0].z);
  //APP_LOG(APP_LOG_LEVEL_INFO,"force=%f", g);
  
  //Show the data
  text_layer_set_text(s_output_layer, s_buffer);
  
  if(isFalling(data)){
    
    send_alert_to_phone();
  }
  
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "No data yet.");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
}

void config_provider(Window * window){
  
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, select_long_click_release_handler);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while (t != NULL) {
    // Long lived buffer
    static char s_buffer[64];

    // Process this pair's key
    switch (t->key) {
    case KEY_DATA:
      // Copy value and display
      snprintf(s_buffer, sizeof(s_buffer), "Received '%s'", t->value->cstring);
      text_layer_set_text(s_output_layer, s_buffer);
      break;
      
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void init() {
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = main_window_load,
    .unload = main_window_unload
	});
  window_stack_push(s_main_window, true);

  // Subscribe to the accelerometer data service
  int num_samples = 3;
  accel_data_service_subscribe(num_samples, data_handler);

  // Choose update rate
  accel_service_set_sampling_rate(ACCEL_SAMPLING_50HZ);

  window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);

}
  

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);
  accel_data_service_unsubscribe();

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

