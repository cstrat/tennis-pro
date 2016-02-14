/*

  TENNIS PRO - lite
  ----------
  
  Three main screens.
  1. Setup Screen
  2. Game Screen
  3. Info Screen
  
  Setup screen displays the various options for the user.
  Game screen is where the game is played and scored.
  Info screen shows additional information during the match.
  
*/

#include <pebble.h>

// Include Other Files
#include <logic.h>
#include <info.h>
#include <main.h>
#include <menu.h>


/*
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}
*/

static void init(void) {
	load_data();

	init_menu();
	init_main();
	init_info();

	window_stack_push(window_menu, true);
}

static void deinit(void) {
	save_data();

	deinit_menu();
	deinit_main();
	deinit_info();

	window_destroy(window_menu);
	window_destroy(window_main);
	window_destroy(window_info);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
