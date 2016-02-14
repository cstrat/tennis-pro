/////////////////
// Definitions //
/////////////////

static Window 			*window_main;

static TextLayer		*tl_clock;
static TextLayer		*tl_timer;
static TextLayer		*tl_banner;
static TextLayer		*tl_game_a;
static TextLayer		*tl_game_b;
static TextLayer		*tl_game_sep;
static TextLayer		*tl_set_a;
static TextLayer		*tl_set_b;
static TextLayer		*tl_set_sep;
static TextLayer		*tl_match_a;
static TextLayer		*tl_match_b;
static TextLayer		*tl_match_sep;

static GRect 			serve_marker_bounds;
static GRect 			serve_map_bounds;

static BitmapLayer 		*image_layer_serve_map;
static GBitmap 			*image_serve_map;
static BitmapLayer 		*image_layer_serve_marker;
static GBitmap 			*image_serve_marker;

static GFont 			custom_font_small;
static GFont 			custom_font_medium;
static GFont 			custom_font_large;

static char char_prev_time[]	= "00:00";
static char char_the_time[]		= "00:00";
static char char_timer[] 		= "000m";

static char banner_text[12];

static bool main_setup = false;


///////////////
// Functions //
///////////////

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	strftime(char_the_time, sizeof(char_the_time), "%R", tick_time);
	text_layer_set_text(tl_clock, char_the_time);


	// Bail on this if the game is over, leave the timer at the point the game finished
	if (game_over) { return; }
	
	if (strcmp(char_prev_time, char_the_time) != 0) {
		game_timer++;
	}

	if (game_timer < 0) {
		game_timer = 0;
	}

	strcpy(char_prev_time, char_the_time);

	snprintf(char_timer, sizeof(char_timer), "%um", game_timer);
	text_layer_set_text(tl_timer, char_timer);
}

static void window_main_redraw(void) {
	// Redraw all labels based on charPairs

	text_layer_set_text(tl_banner, "");

	if (game_in_tiebreak) {
		text_layer_set_text(tl_banner, "TIE-BREAK");

		snprintf(char_game.A, sizeof(char_game.A), "%u", int_game.A);
		text_layer_set_text(tl_game_a, char_game.A);
		
		snprintf(char_game.B, sizeof(char_game.B), "%u", int_game.B);
		text_layer_set_text(tl_game_b, char_game.B);

	}else{
		
		switch (int_game.A) {
			case LOVE: 		text_layer_set_text(tl_game_a, "0"); break;
			case FIFTEEN: 	text_layer_set_text(tl_game_a, "15"); break;
			case THIRTY: 	text_layer_set_text(tl_game_a, "30"); break;
			case FORTY: 	text_layer_set_text(tl_game_a, "40"); break;
			case ADV: 		text_layer_set_text(tl_game_a, "AD"); break;
		}

		switch (int_game.B) {
			case LOVE: 		text_layer_set_text(tl_game_b, "0"); break;
			case FIFTEEN: 	text_layer_set_text(tl_game_b, "15"); break;
			case THIRTY: 	text_layer_set_text(tl_game_b, "30"); break;
			case FORTY: 	text_layer_set_text(tl_game_b, "40"); break;
			case ADV: 		text_layer_set_text(tl_game_b, "AD"); break;
		}
	}
	
	snprintf(char_set.A, sizeof(char_set.A), "%u", int_set.A);
	text_layer_set_text(tl_set_a, char_set.A);
	
	snprintf(char_set.B, sizeof(char_set.B), "%u", int_set.B);
	text_layer_set_text(tl_set_b, char_set.B);
	
	snprintf(char_match.A, sizeof(char_match.A), "%u", int_match.A);
	text_layer_set_text(tl_match_a, char_match.A);
	
	snprintf(char_match.B, sizeof(char_match.B), "%u", int_match.B);
	text_layer_set_text(tl_match_b, char_match.B);

	if ((deuce_count) && (int_game.A == FORTY) && (int_game.B == FORTY)) {
		snprintf(banner_text, sizeof(banner_text), "DEUCE - %u", deuce_count);
		text_layer_set_text(tl_banner, banner_text);
	}

	switch (game_serve_pos) {
		case 1: serve_marker_bounds = (GRect) { .origin = { 30, 143 }, .size = { 15, 15 } }; break;
		case 2: serve_marker_bounds = (GRect) { .origin = { 11, 143 }, .size = { 15, 15 } }; break;
		case 3: serve_marker_bounds = (GRect) { .origin = { 11, 121 }, .size = { 15, 15 } }; break;
		case 4: serve_marker_bounds = (GRect) { .origin = { 30, 121 }, .size = { 15, 15 } }; break;
	}

	layer_set_bounds(bitmap_layer_get_layer(image_layer_serve_marker), serve_marker_bounds);
	layer_mark_dirty(bitmap_layer_get_layer(image_layer_serve_marker));

	if (game_over) {
		text_layer_set_text(tl_banner, "GAME OVER");
	}

}

static void main_select_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (data_position == 0) {
		serve_swap();
		window_main_redraw();
	}else{
		window_stack_push(window_info, true);
	}	
}

static void main_up_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (game_over) { return; }

	if ((data_position > 0) && (data_points[data_position-1] == 'B')) {
		// Use compression
		data_points[data_position-1] = 'D';

	}else if ((data_position > 0) && (data_points[data_position-1] == 'C')) {
		data_points[data_position-1] = 'E';

	}else if ((data_position > 0) && (data_points[data_position-1] == 'D')) {
		data_points[data_position-1] = 'G';

	}else{
		// Add new data position
		data_points[data_position++] = 'A';
	}
	
	process_data_chain();
}

static void main_down_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (game_over) { return; }

	if ((data_position > 0) && (data_points[data_position-1] == 'A')) {
		// Use compression
		data_points[data_position-1] = 'C';

	} else if ((data_position > 0) && (data_points[data_position-1] == 'C')) {
		data_points[data_position-1] = 'F';

	} else if ((data_position > 0) && (data_points[data_position-1] == 'D')) {
		data_points[data_position-1] = 'H';
		
	}else{
		// Add new data position
		data_points[data_position++] = 'B';
	}

	process_data_chain();
}

static void main_select_hold_handler(ClickRecognizerRef recognizer, void *context) {
	data_position--;
	process_data_chain();
}

static void main_click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, main_select_click_handler);

	window_single_click_subscribe(BUTTON_ID_UP, 	main_up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, 	main_down_click_handler);

	window_long_click_subscribe(BUTTON_ID_SELECT, 	800, main_select_hold_handler, NULL);
}

static void window_main_load(Window *window) {
	// Avoid double drawing the window
	if (main_setup) { return; }

	Layer *window_layer = window_get_root_layer(window_main);

	/*
	custom_font_small 	= fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CUSTOMFONT_20));
	custom_font_medium	= fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CUSTOMFONT_24));
	custom_font_large 	= fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CUSTOMFONT_36));
	*/

	static int height_small = 25;
	static int height_med	= 30;
	static int height_large = 50;

	static int padding		= 2;
	static int current_y	= 0;

	// Add Text Layers
	// ROW 1 - Clock & Timer
	current_y = 2;
	tl_clock = text_layer_create(GRect(2, current_y, 70, height_small));
	text_layer_set_text_color(tl_clock, GColorBlack);
	text_layer_set_text_alignment(tl_clock, GTextAlignmentCenter);
	text_layer_set_background_color(tl_clock, GColorWhite);
	//text_layer_set_font(tl_clock, custom_font_small);
	layer_add_child(window_layer, text_layer_get_layer(tl_clock));
	text_layer_set_text(tl_clock, "");

	tl_timer = text_layer_create(GRect(74, current_y, 70, height_small));
	text_layer_set_text_color(tl_timer, GColorBlack);
	text_layer_set_text_alignment(tl_timer, GTextAlignmentCenter);
	text_layer_set_background_color(tl_timer, GColorWhite);
	//text_layer_set_font(tl_timer, custom_font_small);
	layer_add_child(window_layer, text_layer_get_layer(tl_timer));
	text_layer_set_text(tl_timer, "");

	// ROW 2 - Banner
	current_y = height_small + (padding * 2);
	tl_banner = text_layer_create(GRect(2, current_y, 140, height_small));
	text_layer_set_text_color(tl_banner, GColorBlack);
	text_layer_set_text_alignment(tl_banner, GTextAlignmentCenter);
	text_layer_set_background_color(tl_banner, GColorWhite);
	//text_layer_set_font(tl_banner, custom_font_medium);
	layer_add_child(window_layer, text_layer_get_layer(tl_banner));
	text_layer_set_text(tl_banner, "NOTICE BANNER");


	// ROW 3 - Game Score
	current_y = (height_small * 2) + (padding * 4);
	tl_game_b = text_layer_create(GRect(2, current_y, 56, height_large));
	text_layer_set_text_color(tl_game_b, GColorBlack);
	text_layer_set_text_alignment(tl_game_b, GTextAlignmentRight);
	text_layer_set_background_color(tl_game_b, GColorWhite);
	//text_layer_set_font(tl_game_b, custom_font_large);
	layer_add_child(window_layer, text_layer_get_layer(tl_game_b));
	text_layer_set_text(tl_game_b, "15");

	tl_game_sep = text_layer_create(GRect(60, current_y, 20, height_large));
	text_layer_set_text_color(tl_game_sep, GColorBlack);
	text_layer_set_text_alignment(tl_game_sep, GTextAlignmentCenter);
	text_layer_set_background_color(tl_game_sep, GColorWhite);
	//text_layer_set_font(tl_game_sep, custom_font_large);
	layer_add_child(window_layer, text_layer_get_layer(tl_game_sep));
	text_layer_set_text(tl_game_sep, ":");

	tl_game_a = text_layer_create(GRect(84, current_y, 56, height_large));
	text_layer_set_text_color(tl_game_a, GColorBlack);
	text_layer_set_text_alignment(tl_game_a, GTextAlignmentLeft);
	text_layer_set_background_color(tl_game_a, GColorWhite);
	//text_layer_set_font(tl_game_a, custom_font_large);
	layer_add_child(window_layer, text_layer_get_layer(tl_game_a));
	text_layer_set_text(tl_game_a, "45");

	// ROW 4 - Current Set
	current_y = height_large + (height_small * 2) + (padding * 6);
	tl_set_b = text_layer_create(GRect(2, current_y, 56, height_small));
	text_layer_set_text_color(tl_set_b, GColorBlack);
	text_layer_set_text_alignment(tl_set_b, GTextAlignmentRight);
	text_layer_set_background_color(tl_set_b, GColorWhite);
	//text_layer_set_font(tl_set_b, custom_font_medium);
	layer_add_child(window_layer, text_layer_get_layer(tl_set_b));
	text_layer_set_text(tl_set_b, "15");

	tl_set_sep = text_layer_create(GRect(60, current_y, 20, height_small));
	text_layer_set_text_color(tl_set_sep, GColorBlack);
	text_layer_set_text_alignment(tl_set_sep, GTextAlignmentCenter);
	text_layer_set_background_color(tl_set_sep, GColorWhite);
	//text_layer_set_font(tl_set_sep, custom_font_medium);
	layer_add_child(window_layer, text_layer_get_layer(tl_set_sep));
	text_layer_set_text(tl_set_sep, ":");

	tl_set_a = text_layer_create(GRect(84, current_y, 56, height_small));
	text_layer_set_text_color(tl_set_a, GColorBlack);
	text_layer_set_text_alignment(tl_set_a, GTextAlignmentLeft);
	text_layer_set_background_color(tl_set_a, GColorWhite);
	//text_layer_set_font(tl_set_a, custom_font_medium);
	layer_add_child(window_layer, text_layer_get_layer(tl_set_a));
	text_layer_set_text(tl_set_a, "45");


	// ROW 5 - Current Match
	current_y = height_large + (height_small * 3) + (padding * 8);
	tl_match_b = text_layer_create(GRect(2, current_y, 60, height_small));
	text_layer_set_text_color(tl_match_b, GColorBlack);
	text_layer_set_text_alignment(tl_match_b, GTextAlignmentRight);
	text_layer_set_background_color(tl_match_b, GColorWhite);
	//text_layer_set_font(tl_match_b, custom_font_medium);
	layer_add_child(window_layer, text_layer_get_layer(tl_match_b));
	text_layer_set_text(tl_match_b, "15");

	tl_match_sep = text_layer_create(GRect(60, current_y, 24, height_small));
	text_layer_set_text_color(tl_match_sep, GColorBlack);
	text_layer_set_text_alignment(tl_match_sep, GTextAlignmentCenter);
	text_layer_set_background_color(tl_match_sep, GColorWhite);
	//text_layer_set_font(tl_match_sep, custom_font_medium);
	layer_add_child(window_layer, text_layer_get_layer(tl_match_sep));
	text_layer_set_text(tl_match_sep, ":");

	tl_match_a = text_layer_create(GRect(84, current_y, 60, height_small));
	text_layer_set_text_color(tl_match_a, GColorBlack);
	text_layer_set_text_alignment(tl_match_a, GTextAlignmentLeft);
	text_layer_set_background_color(tl_match_a, GColorWhite);
	//text_layer_set_font(tl_match_a, custom_font_medium);
	layer_add_child(window_layer, text_layer_get_layer(tl_match_a));
	text_layer_set_text(tl_match_a, "45");

	// Serve Map and Marker
	image_serve_map = gbitmap_create_with_resource(RESOURCE_ID_SERVE_MAP);
	serve_map_bounds = (GRect) { .origin = { 4, 168-58 }, .size = { 46, 58 } };
	image_layer_serve_map = bitmap_layer_create(serve_map_bounds);
	bitmap_layer_set_bitmap(image_layer_serve_map, image_serve_map);
	bitmap_layer_set_alignment(image_layer_serve_map, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer_serve_map));

	image_serve_marker = gbitmap_create_with_resource(RESOURCE_ID_SERVE_MARKER);
	serve_marker_bounds = (GRect) { .origin = { 30, 143 }, .size = { 15, 15 } };
	image_layer_serve_marker = bitmap_layer_create(serve_marker_bounds);
	bitmap_layer_set_bitmap(image_layer_serve_marker, image_serve_marker);
	bitmap_layer_set_alignment(image_layer_serve_marker, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer_serve_marker));


	// Start timer
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

	// Invoke the timer now
	static time_t now;
	now = time(NULL);
	struct tm *tick_time = localtime(&now);
	handle_minute_tick(tick_time, MINUTE_UNIT);

	// Run Refresh
	process_data_chain();

	// Avoid double drawing the window
	main_setup = true;
}

static void window_main_unload(Window *window) {
	if (!main_setup) { return; }

	// Destroy Text Layers
	text_layer_destroy(tl_clock);
	text_layer_destroy(tl_timer);
	text_layer_destroy(tl_banner);
	text_layer_destroy(tl_game_a);
	text_layer_destroy(tl_game_b);
	text_layer_destroy(tl_game_sep);
	text_layer_destroy(tl_set_a);
	text_layer_destroy(tl_set_b);
	text_layer_destroy(tl_set_sep);
	text_layer_destroy(tl_match_a);
	text_layer_destroy(tl_match_b);
	text_layer_destroy(tl_match_sep);

	// Stop Timer
	tick_timer_service_unsubscribe();

	// Unload Images
	gbitmap_destroy(image_serve_marker);
	bitmap_layer_destroy(image_layer_serve_marker);
	gbitmap_destroy(image_serve_map);
	bitmap_layer_destroy(image_layer_serve_map);

	// Unload font
	/*
	fonts_unload_custom_font(custom_font_small);
	fonts_unload_custom_font(custom_font_medium);
	fonts_unload_custom_font(custom_font_large);
	*/

	main_setup = false;
}

static void init_main(void) {
	window_main = window_create();

	window_set_click_config_provider(window_main, main_click_config_provider);
	window_set_window_handlers(window_main, (WindowHandlers) {
		.load 		= window_main_load,
		.unload 	= window_main_unload,
		.appear 	= window_main_load,
		.disappear 	= window_main_unload,
	});

	window_set_fullscreen(window_main, true);
}

static void deinit_main(void) {
	window_main_unload(window_main);
}