/*
 * main.c
 * Constructs a Window housing an output TextLayer to show data from 
 * either modes of operation of the accelerometer.
 */

#include <pebble.h>
#include <stdlib.h> // for abs() method
#define TAP_NOT_DATA false

static Window *s_main_window;
static TextLayer *s_output_layer;
// should this be declared in main, to save memory?
int gatherStats = 0;
int accelSamples [100][4]; //fourth is for absolute sum of three places
long accelTimeStamp [100]; //is a long actually big enough to hold these? I dunno, seems to work now

static void data_process(){ // only calculates beginning of longest run at the moment
  // should for end of longest run too
  int longestRun = 0;
  int thisRun = 0;
  int longIndex = 0;
  int thisIndex = 0;
  bool firstTime = true; 
  // loop through whole data set, find longest run of decreasing values
  for(int i = 0; i < 100 - 3; i += 2){
    // checks if 3 consecutive values are increasing
    if (accelSamples[i][3] < accelSamples[i+1][3] &&
        accelSamples[i+1][3] < accelSamples[i+2][3]) {
      thisRun += 2;
      if (firstTime == true){
        thisIndex = i; // start of run value set only if its the start of a run
        firstTime = false;
      }
    }
    else {
      firstTime = true;
      if (thisRun > longestRun) {
        longestRun = thisRun;
        longIndex = thisIndex;
        thisRun = 0;
        thisIndex = 0;
      }
    }
  }
  // output results of data
  printf("Run Length (25 samples a second): %5d\tLongest run start time: %10li", 
         longestRun, accelTimeStamp[longIndex]);
}

static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  static char s_buffer[128];
  
  // Compose string of all data
  if (gatherStats < 100){
    accelSamples[gatherStats][0] = data[0].x;
    accelSamples[gatherStats][1] = data[0].y;
    accelSamples[gatherStats][2] = data[0].z; 
    accelSamples[gatherStats][3] = abs(data[0].x) + abs(data[0].y) + abs(data[0].z);
    accelTimeStamp [gatherStats] = data[0].timestamp;
    
    //data[0].timestamp is a "uint64_t", which I just cast to a long for convenience
    printf("time,X,Y,Z,abs:%10li\t%5d\t%5d\t%5d\t%5d", 
           (long)data[0].timestamp, data[0].x, data[0].y, data[0].z,accelSamples[gatherStats][3]);
    snprintf(s_buffer, sizeof(s_buffer), 
      "X,Y,Z\n> %d,%d,%d", 
      data[0].x, data[0].y, data[0].z
    );
  
    //Show the data
    text_layer_set_text(s_output_layer, s_buffer);
    gatherStats++; // increment index counter
  }
  else {
    // maybe later I can add some text to print out to the user (ie watch screen)
    accel_data_service_unsubscribe();
    // unsubscribe from accel data, saves battery here
    printf("Done collecting data");
    //implements calculation function, uses the data
    data_process();
  }
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  switch (axis) {
  case ACCEL_AXIS_X:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "X axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "X axis negative.");
    }
    break;
  case ACCEL_AXIS_Y:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "Y axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "Y axis negative.");
    }
    break;
  case ACCEL_AXIS_Z:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "Z axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "Z axis negative.");
    }
    break;
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

static void init() {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  // Use tap service? If not, use data service
  if (TAP_NOT_DATA) {
    // Subscribe to the accelerometer tap service
    accel_tap_service_subscribe(tap_handler);
  } else {
    // Subscribe to the accelerometer data service
    int num_samples = 1;
    accel_data_service_subscribe(num_samples, data_handler);

    // Choose update rate
    accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
  }
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);

  if (TAP_NOT_DATA) {
    accel_tap_service_unsubscribe();
  } else {
    // accel_data_service_unsubscribe();
    // moved above for when 100 samples (10 seconds) is hit for battery concerns
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}