#include <pebble.h>

Window *window; // the pointer for the Window element
TextLayer *text_layer;
char buffer[] = "00:00";

void tick_handler(struct tm *tick_time, TimeUnits units_changed){ //updates watchface on minute
  // update watchface here
  strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  
  // Change the TextLayer to show the new time!
  text_layer_set_text(text_layer, buffer);
}

void window_load(Window *window){
  //to be added to
  text_layer = text_layer_create(GRect(0, 0, 144, 168)); //0,0 origin, 
  // layer of size 144 by 168 (aka full screen)
  
  text_layer_set_background_color(text_layer, GColorClear); // sets the layer’s 
  // background colour using the supplied pointer and GColor type
  text_layer_set_text_color(text_layer, GColorBlack); //sets the text colour itself
  // in a similar manner to the set_background_color line
 
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));
  // adds the TextLayer to the Window as a child (shown in front)
  
  text_layer_set_text(text_layer, "white horse");
  
  //Get a time structure so that the face doesn't start blank
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);
  
  //Manually call the tick handler when the window is loading
  tick_handler(t, MINUTE_UNIT);
}

void window_unload(Window *window){
  //to be added to
}

void init() { // set up app
  // Initialize the app elements here!
	window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });	
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
  window_stack_push(window, true);
}

void deinit() { // Frees up memory once app is done running
  // De-initialize the app elements here!
  text_layer_destroy(text_layer); // because text_layer is a child of window, it must
  // be destroyed first
  window_destroy(window);
  tick_timer_service_unsubscribe();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
