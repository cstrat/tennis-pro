/////////////////
// Definitions //
/////////////////

static Window 				*window_menu;

static BitmapLayer 			*image_layer_logo;
static GBitmap 				*image_logo;
static int 					logo_height = 40;

static SimpleMenuLayer 		*simple_menu_layer;
static SimpleMenuSection 	menu_sections[2];
static SimpleMenuItem 		menu_list_settings[5];
static SimpleMenuItem 		menu_list_actions[5];

const char *library_limit[] 	= {"Continous", "Single Match","", "Best of Three", "", "Best of Five"};
const char *library_singles[] 	= {"Doubles", "Singles"};
const char *library_toggle[] 	= {"Disabled", "Enabled"};

static bool menu_setup 			= false;

///////////////
// Functions //
///////////////

static void special_select_callback(int index, void *ctx) {}

// Cycle through match limit options, potentially add other options too!
static void menu_cycle_match_limit(int index, void *ctx) {

	switch (settings_limit) {
		case 0: settings_limit = 1; break;
		case 1: settings_limit = 3; break;
		case 3: settings_limit = 5; break;
		case 5: settings_limit = 0; break;
	}

	menu_list_settings[index].subtitle = library_limit[settings_limit];
	layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
}

// Toggle singles/doubles match
static void menu_toggle_singles(int index, void *ctx) {
	settings_singles = !settings_singles;
	menu_list_settings[index].subtitle = library_singles[settings_singles];
	layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
}

// Toggle tiebreak mode
static void menu_toggle_tiebreak(int index, void *ctx) {
	settings_tiebreak = !settings_tiebreak;
	menu_list_settings[index].subtitle = library_toggle[settings_tiebreak];
	layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
}

// Toggle noadscoring mode
static void menu_toggle_noadscoring(int index, void *ctx) {
	settings_noadscoring = !settings_noadscoring;
	menu_list_settings[index].subtitle = library_toggle[settings_noadscoring];
	layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
}

// Toggle proscoring mode
static void menu_toggle_proscoring(int index, void *ctx) {
	settings_proscoring = !settings_proscoring;
	menu_list_settings[index].subtitle = library_toggle[settings_proscoring];
	layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
}

// Forward Declaration [Abandon Action Requires This]
static void window_menu_load(Window *window);
static void window_menu_unload(Window *window);

// Menu Actions
static void menu_action_start(int index, void *ctx) {
	window_stack_push(window_main, true);
	game_started = true;
}

static void menu_action_resume(int index, void *ctx) {
	window_stack_push(window_main, true);
}

static void menu_action_abandon(int index, void *ctx) {
	data_position 		= 0;
	game_timer 			= -1;
	game_started 		= false;
	game_in_match 		= false;
	game_in_tiebreak	= false;
	game_matches		= 0;
	
	window_menu_unload(window_menu);
	window_menu_load(window_menu);
}

static void menu_action_extend(int index, void *ctx) {
	game_over = false;
	settings_limit = 0;
	menu_action_resume(index, ctx);
}

static void menu_action_save(int index, void *ctx) {
	/* STUB */
}


static void window_menu_load(Window *window) {
	// Avoid double drawing the window
	if (menu_setup) { return; }

	// Window layer
	Layer *window_layer = window_get_root_layer(window_menu);

	// Load Logo
	image_logo = gbitmap_create_with_resource(RESOURCE_ID_LOGO);
	GRect logo_bounds = { .origin = { 0, 0 }, .size = { 144, logo_height } };
	image_layer_logo = bitmap_layer_create(logo_bounds);
	bitmap_layer_set_bitmap(image_layer_logo, image_logo);
	bitmap_layer_set_alignment(image_layer_logo, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer_logo));

	// Setup Menu
	int int_menu_sections	= 0;
	int int_list_settings	= 0;
	int int_list_actions	= 0;

	// Action Items
	if (!game_started) {
		menu_list_actions[int_list_actions++] = (SimpleMenuItem){
			.title = "Start Play",
			.callback = menu_action_start,
		};
	}else{
		if (game_over) {
			menu_list_actions[int_list_actions++] = (SimpleMenuItem){
				.title = "Review Play",
				.subtitle = "Game Over",
				.callback = menu_action_resume,
			};
			menu_list_actions[int_list_actions++] = (SimpleMenuItem){
				.title = "Extend Play",
				.subtitle = "Continous Match",
				.callback = menu_action_extend,
			};
		}else{
			menu_list_actions[int_list_actions++] = (SimpleMenuItem){
				.title = "Resume Play",
				.callback = menu_action_resume,
			};
		}
		menu_list_actions[int_list_actions++] = (SimpleMenuItem){
			.title = "Abandon Play",
			.subtitle = "Delete Game Data",
			.callback = menu_action_abandon,
		};

		/*
		menu_list_actions[int_list_actions++] = (SimpleMenuItem){
			.title = "Save Game",
			.callback = menu_action_save,
		};
		*/
	}

	// Settings Items
	if (!game_started) {
		menu_list_settings[int_list_settings++] = (SimpleMenuItem){
			.title = "Match Type",
			.subtitle = library_limit[settings_limit],
			.callback = menu_cycle_match_limit,
		};
		menu_list_settings[int_list_settings++] = (SimpleMenuItem){
			.title = "Singles/Doubles",
			.subtitle = library_singles[settings_singles],
			.callback = menu_toggle_singles,
		};
		menu_list_settings[int_list_settings++] = (SimpleMenuItem){
			.title = "Tiebreak Scoring",
			.subtitle = library_toggle[settings_tiebreak],
			.callback = menu_toggle_tiebreak,
		};
		menu_list_settings[int_list_settings++] = (SimpleMenuItem){
			.title = "No-Ad Scoring",
			.subtitle = library_toggle[settings_noadscoring],
			.callback = menu_toggle_noadscoring,
		};
		menu_list_settings[int_list_settings++] = (SimpleMenuItem){
			.title = "Pro Scoring",
			.subtitle = library_toggle[settings_proscoring],
			.callback = menu_toggle_proscoring,
		};
	}

	// Bind Items to Sections
	menu_sections[int_menu_sections++] = (SimpleMenuSection){
		.title 		= "Game Actions",
		.num_items 	= int_list_actions,
		.items 		= menu_list_actions,
	};

	if (!game_started) {
		menu_sections[int_menu_sections++] = (SimpleMenuSection){
			.title 		= "Game Settings",
			.num_items 	= int_list_settings,
			.items 		= menu_list_settings,
		};
	}

	// Bind Section to Layer & Window
	GRect menu_bounds = { .origin = { 0, logo_height }, .size = { 144, 168-logo_height } };
	simple_menu_layer = simple_menu_layer_create(menu_bounds, window_menu, menu_sections, int_menu_sections, NULL);
	layer_add_child(window_layer, simple_menu_layer_get_layer(simple_menu_layer));

	// Flag to avoid double drawing of this window
	menu_setup = true;
}

static void window_menu_unload(Window *window) {
	if (!menu_setup) { return; }

	simple_menu_layer_destroy(simple_menu_layer);
	gbitmap_destroy(image_logo);
	bitmap_layer_destroy(image_layer_logo);
	
	menu_setup = false;
}

static void init_menu(void) {
	window_menu = window_create();

	window_set_window_handlers(window_menu, (WindowHandlers) {
		.load 		= window_menu_load,
		.unload 	= window_menu_unload,
		.appear 	= window_menu_load,
		.disappear 	= window_menu_unload,
	});

	window_set_fullscreen(window_menu, true);
}

static void deinit_menu(void) {
	window_menu_unload(window_menu);
}