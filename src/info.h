static Window 	*window_info;

static TextLayer		*tl_set1;
static TextLayer		*tl_set2;
static TextLayer		*tl_set3;
static TextLayer		*tl_set4;
static TextLayer		*tl_set5;

static bool info_setup = false;

static void info_select_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (game_matches == 0) { return; }
	static char bufferstr[20];
	for (int i=0; i<game_matches; i++) {
		snprintf(bufferstr, sizeof(bufferstr), "SET %u TIME %u", i, data_set_times[i]);
		APP_LOG(APP_LOG_LEVEL_DEBUG, bufferstr);
		snprintf(bufferstr, sizeof(bufferstr), "SCORE - %u:%u", data_set_scores[i].A, data_set_scores[i].B);
		APP_LOG(APP_LOG_LEVEL_DEBUG, bufferstr);
	}
}

static void info_up_click_handler(ClickRecognizerRef recognizer, void *context) {
	
}

static void info_down_click_handler(ClickRecognizerRef recognizer, void *context) {
	
}

static void info_click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, info_select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, info_up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, info_down_click_handler);
}

static void window_info_load(Window *window) {
	if (info_setup) { return; }

	info_setup = true;
}

static void window_info_unload(Window *window) {
	if (!info_setup) { return; }

	info_setup = false;
}

static void init_info(void) {
	window_info = window_create();

	window_set_click_config_provider(window_info, info_click_config_provider);
	window_set_window_handlers(window_info, (WindowHandlers) {
		.load 		= window_info_load,
		.unload 	= window_info_unload,
		.appear 	= window_info_load,
		.disappear 	= window_info_unload,
	});

	window_set_fullscreen(window_info, true);
}

static void deinit_info(void) {

}