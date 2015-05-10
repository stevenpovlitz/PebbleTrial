/*
 * main.c
 * Constructs a Window housing an output TextLayer to show data from 
 * either modes of operation of the accelerometer.
 */

#include <pebble.h>
#include <stdlib.h> // for abs() method
bool unsubbed = false;
#define NUMSAMPLES 200

static Window *s_main_window;
static TextLayer *s_output_layer;
// should this be declared in main, to save memory?
int gatherStats = 0;
int accelSamples [NUMSAMPLES][4]; //fourth is for absolute sum of three places
// long accelTimeStamp [100]; //is a long actually big enough to hold these? I dunno, seems to work now

// find num samples between max thrown height and impact with ground
static int find_throw_length(){
  int throwLength = 0;
  int h = 10; // not 0 to avoid initial vibs throwing off data
  
  // find a large val (prob start of throw) then find when ensuing dip aka throw
  // ends with a dramatic change in abs acceleration
  while (h < NUMSAMPLES){
    if (accelSamples[h][3] > 4000){ // high accel values = start of throw
      int biggest = accelSamples[h][3];
      printf("Biggest aka over 4000: %d", biggest);
      int i = 0;
      for(int j = 0; j < 10; j++){
        if (accelSamples[h+j][3] >= biggest){
          i = h+j; // i is highest of values up to 10 after an over 4000 value
          biggest = accelSamples[h+j][3];
        }
      }
      for(i = i+5; i < NUMSAMPLES - 2; i++){ // start looking for end of throw
        if((accelSamples[i][3] - accelSamples[i+1][3]) > 2000){ //dramatic change in accel
          printf("i:%d h:%d" , i, h); // log out the data we want to work with
          throwLength = i - h; // throw end - throw start
          return throwLength;
        }
      } 
    }
    h++;
  }
  return -1;;
}

// output how high the pebble was thrown
static void data_process(){ 
  int x = find_throw_length(); // get length of throw in # of samples
  if (x == -1) {
    printf ("No peak found, try tossing harder next time.");
  }
  else {
    printf("Samples in throw: %d", x);
    // should calc and store and output as a float
    double heightThrown = ((double)x) / 2.0 / 25.0 * 9.8; // find in meters height thrown
    printf ("Pebble was tossed %4d meters high.\n" , (int)(heightThrown * 100));
  }
}
static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  static char s_buffer[128];
  
  // Compose string of all data
  if (gatherStats < NUMSAMPLES){
    accelSamples[gatherStats][0] = data[0].x;
    accelSamples[gatherStats][1] = data[0].y;
    accelSamples[gatherStats][2] = data[0].z; 
    accelSamples[gatherStats][3] = abs(data[0].x) + abs(data[0].y) + abs(data[0].z);
    // accelTimeStamp [gatherStats] = data[0].timestamp;
    
    //data[0].timestamp is a "uint64_t", which I just cast to a long for convenience
    printf("Index,X,Y,Z\t %5d\t %5d\t %5d\t %5d\t %5d",  //,abs:\t%10li taken out
           gatherStats, data[0].x, data[0].y, data[0].z,accelSamples[gatherStats][3]); //(long)data[0].timestamp,  taken out
    
    //printf("Index, X, Y, X\t%5d\t%5d" , gatherStats, data[0].x);
    
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
    unsubbed = true;
    
    // unsubscribe from accel data, saves battery here
    printf("Done collecting data");
    // vibrate to let user know data collection is done
    vibes_long_pulse();
    //implements calculation function, uses the data
    data_process();
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

  // Subscribe to the accelerometer data service
  int num_samples = 1;
  accel_data_service_subscribe(num_samples, data_handler);

  // Choose update rate
  accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);
  if (unsubbed == false){
    accel_data_service_unsubscribe();
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}