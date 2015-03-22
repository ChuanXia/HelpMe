#include <pebble.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define THRES 550
#define KEY_DATA 1
  
static Window *s_main_window;
static Window *alert_window;
static Window *select_window;
static TextLayer *s_output_layer;
static TextLayer *alert_layer;
static TextLayer *select_layer;
static bool has_select_window;

static int s_uptime = 0;



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
//Main window loading
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

//Alert Window loading
static void alert_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  alert_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(alert_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(alert_layer, "Hold on, Sending Alert!!!");
  text_layer_set_overflow_mode(alert_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(alert_layer));

}

static void alert_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(alert_layer);
}

void send_alert_to_phone(){
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_DATA, 1);
  app_message_outbox_send();
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Msg Sent!");
  
  alert_window = window_create();
  APP_LOG(APP_LOG_LEVEL_DEBUG , "Alert window create step1");
  window_set_window_handlers(alert_window, (WindowHandlers) {
      .load = alert_window_load,
    .unload = alert_window_unload
	});
  
  window_stack_push(alert_window, true);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"alert window created");
 
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if(s_uptime==10){
    send_alert_to_phone();  
  }
  s_uptime++;
}
//Select window loading
static void select_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  select_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(select_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(select_layer, "Do you want to send the alert? UP:YES | DOWN:NO");
  

  
  text_layer_set_overflow_mode(select_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(select_layer));

}

static void select_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(select_layer);
}




void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  //send alert to phone via bluetooth
  // accel_data_service_unsubscribe();
  if(click_recognizer_get_button_id(recognizer)==BUTTON_ID_UP){
    send_alert_to_phone();
  }
  
  else if(click_recognizer_get_button_id(recognizer)==BUTTON_ID_DOWN){
    s_uptime=100;
    
  }

  
}
void select_long_click_handler(ClickRecognizerRef recognizer, void *context){
  send_alert_to_phone();
}

void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  //recv ACK from phone and print 
  
}



bool isFalling(AccelData *data) {
  int16_t max0=max(data[0].x, data[0].y, data[0].z);
  int16_t max1=max(data[1].x, data[1].y, data[1].z);
  int16_t max2=max(data[2].x, data[2].y, data[2].z);
  if ( max(max0,max1,max2)<THRES){
    return true;
  }
  return false;
}




void config_provider(Window * window){
  
  window_single_click_subscribe(BUTTON_ID_UP,select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN,select_single_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, select_long_click_release_handler);
}
//Whether to send signal after fall down
void create_select_dialogue(){
  if (!has_select_window) {
    select_window = window_create();
    has_select_window=true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating new window ");
    window_set_window_handlers(select_window, (WindowHandlers) {
        .load = select_window_load,
      .unload = select_window_unload
	  });
    window_stack_push(select_window, true);
    APP_LOG(APP_LOG_LEVEL_DEBUG,"%d", s_uptime);
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    window_set_click_config_provider(select_window, (ClickConfigProvider) config_provider);
  } else {
    // set select_window as the active window
      
  }
}

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
    //Create a dialogue
    create_select_dialogue();
  }
  
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
  accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);

  window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);

}
  

static void deinit() {
  // Destroy main Window
  accel_data_service_unsubscribe();
  window_destroy(alert_window);
  window_destroy(select_window);
  window_destroy(s_main_window);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "destroy window");
}

int main(void) {
  has_select_window = false;
  init();
  app_event_loop();
  deinit();
}

