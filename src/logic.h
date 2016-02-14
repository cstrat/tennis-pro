/////////////////
// Definitions //
/////////////////

#define SKEY_SINGLES 	1
#define SKEY_TIEBREAK	2
#define SKEY_NOAD		3
#define SKEY_PROSCORE	4
#define SKEY_LIMIT		5
#define SKEY_DATA 		6
#define SKEY_DATA_POS	7
#define SKEY_SET_TIMES	10
#define SKEY_TIMER		8
#define SKEY_OVER		9


typedef enum 
{
	LOVE = 0,
	FIFTEEN = 1,
	THIRTY = 2,
	FORTY = 3,
	ADV = 4,
	WIN = 5
} tennisScores;

typedef struct
{
	int A;
	int B;
} intPair;

typedef struct
{
	char A[2];
	char B[2];
} charPair;

static int		settings_limit			= 1;
static bool 	settings_singles		= 1;
static bool 	settings_tiebreak		= 1;
static bool 	settings_noadscoring	= 0;
static bool 	settings_proscoring		= 0;

static char 	data_points[256];					// Compressed data set
static int		data_position 			= 0;		// Current position in the data set
static int		data_set_times[20];					// Array of set times
static intPair	data_set_scores[20];				// Array of set scores
static int		last_data_position 		= 0;		// Last processed position in the data set

static int		game_timer 				= -1;		// Counts the minutes in game
static bool		game_started 			= false;	// Have any matches started
static bool 	game_in_match			= false;	// Toggles at the end of a match
static int		game_matches			= 0;		// Updates with the number of completed matches
static bool		game_in_tiebreak		= false;	// Toggles if you're in a tiebreak game
static bool		game_over				= false;	// Is there a winner?
static int		game_serve_pos			= 1;		// Serve Pos, 1=BR, 2=BL, 3=TL, 4=TR
static int		game_serve_start		= 1;		// Which position did we start the game

static int		deuce_count				= 0;		// Tally of deuces
static bool		new_point				= false;	// Flag for new point being added

static charPair	char_game;
static charPair	char_set;
static charPair	char_match;

static intPair	int_game;
static intPair	int_set;
static intPair	int_match;

static intPair	int_last_game;
static intPair	int_last_set;


// Forward Declaration [Game Logic]
static void window_main_redraw(void);
static void process_data_chain(void);

/////////////
// Storage //
/////////////

static void save_data(void) {
	persist_write_int(SKEY_LIMIT,		settings_limit);
	persist_write_bool(SKEY_SINGLES,	settings_singles);
	persist_write_bool(SKEY_TIEBREAK,	settings_tiebreak);
	persist_write_bool(SKEY_NOAD,		settings_noadscoring);
	persist_write_bool(SKEY_PROSCORE,	settings_proscoring);

	persist_write_bool(SKEY_OVER,		game_over);

	persist_write_int(SKEY_TIMER,		game_timer);
	persist_write_int(SKEY_DATA_POS,	data_position);
	persist_write_string(SKEY_DATA,		data_points);
}

static void load_data(void) {
	settings_limit			= persist_exists(SKEY_LIMIT)	? persist_read_int(SKEY_LIMIT)			: 1;
	settings_singles		= persist_exists(SKEY_SINGLES)	? persist_read_bool(SKEY_SINGLES)		: true;
	settings_tiebreak		= persist_exists(SKEY_TIEBREAK)	? persist_read_bool(SKEY_TIEBREAK)		: true;
	settings_noadscoring	= persist_exists(SKEY_NOAD)		? persist_read_bool(SKEY_NOAD)			: false;
	settings_proscoring		= persist_exists(SKEY_PROSCORE)	? persist_read_bool(SKEY_PROSCORE)		: false;

	game_over				= persist_exists(SKEY_OVER)		? persist_read_bool(SKEY_OVER)			: false;

	game_timer				= persist_exists(SKEY_TIMER)	? persist_read_int(SKEY_TIMER)			: -1;
	data_position			= persist_exists(SKEY_DATA_POS)	? persist_read_int(SKEY_DATA_POS)		: 0;
	
	if (persist_exists(SKEY_DATA)) {
		persist_read_string(SKEY_DATA, data_points, 256);
	}

	if (data_position > 0) {
		game_started = true;
	}
}

///////////////
// Functions //
///////////////

static void process_match() {
	int_last_set 	= int_set;
	int_set.A 		= 0;
	int_set.B 		= 0;
	game_in_match	= false;

	// At this point you can save the progress, even if you haven't finished the match limit.
	switch (settings_limit) {
		case 1:
			if ((int_match.A == 1) || (int_match.B == 1)) {
				// Winner!
				game_over = true;
			}

		break;
		case 3:
			// Need two wins
			if ((int_match.A == 2) || (int_match.B == 2)) {
				// Winner!
				game_over = true;
			}

		break;
		case 5:
			if ((int_match.A == 3) || (int_match.B == 3)) {
				// Winner!
				game_over = true;
			}

		break;
	}

}

static void process_set() {
	int_last_game 	= int_game;
	int_game.A 		= 0;
	int_game.B 		= 0;

	static int games_to_win = 6;

	if (settings_proscoring) {
		games_to_win = 8;
	}

	// TIEBREAK DISABLED - Must win by two sets
	if (!settings_tiebreak) {
		if (int_set.A >= games_to_win && int_set.A >= (int_set.B+2)) {
			int_match.A++;
			process_match();
		}
		if (int_set.B >= games_to_win && int_set.B >= (int_set.A+2)) {
			int_match.B++;
			process_match();
		}

	// TIEBREAK ENABLED - Special Game @ 6:6
	}else{
		if ((game_in_tiebreak && int_set.A == (games_to_win+1)) || (int_set.A >= games_to_win && int_set.A >= (int_set.B+2))) {
			int_match.A++;
			process_match();
		}
		if ((game_in_tiebreak && int_set.B == (games_to_win+1)) || (int_set.B >= games_to_win && int_set.B >= (int_set.A+2))) {
			int_match.B++;
			process_match();
		}

		if ((int_set.A == games_to_win) && (int_set.B == games_to_win)) {
			game_in_tiebreak = true;
		}else{
			game_in_tiebreak = false;
		}

	}
}

static void serve_alternate() {
	switch (game_serve_pos) {
		case 1: game_serve_pos = 2; break;
		case 2: game_serve_pos = 1; break;
		case 3: game_serve_pos = 4; break;
		case 4: game_serve_pos = 3; break;
	}
}

static void serve_swap() {
	switch (game_serve_pos) {
		case 1: game_serve_pos = 3; break;
		case 2: game_serve_pos = 3; break;
		case 3: game_serve_pos = 1; break;
		case 4: game_serve_pos = 1; break;
	}
}

static void process_game() {
	game_in_match = true;

	static bool call_end_game = false;

	// Scoring
	// Normal Play (Advantage)
	if (!game_in_tiebreak) {
		if (!settings_noadscoring) {
			if (int_game.A == ADV && int_game.B == ADV) {
				int_game.A = FORTY;
				int_game.B = FORTY;

			}else if ((int_game.A == ADV && int_game.B <= THIRTY) || (int_game.A == WIN)) {
				int_set.A++;
				call_end_game = true;

			}else if ((int_game.B == ADV && int_game.A <= THIRTY) || (int_game.B == WIN)) {
				int_set.B++;
				call_end_game = true;
			}

			// Track number of times you hit deuce!
			if (int_game.A == FORTY && int_game.B == FORTY) {
				deuce_count++;
			}

		}else{
			if (int_game.A == ADV) {
				int_set.A++;
				call_end_game = true;

			}else if (int_game.B == ADV) {
				int_set.B++;
				call_end_game = true;
			}
		}

		if (call_end_game) {
			process_set();
			serve_swap();
			call_end_game 	= false;
			deuce_count = 0;

		}else{
			serve_alternate();
		}


	// In Tiebreak
	}else{
		if ((int_game.A >= 7) && (int_game.A >= int_game.B+2)) {
			int_set.A++;
			call_end_game = true;

		}else if ((int_game.B >= 7) && (int_game.B >= int_game.A+2)) {
			int_set.B++;
			call_end_game = true;
		}

		if (call_end_game) {
			process_set();
			serve_swap();
			call_end_game = false;
		}else{
			if (((int_game.A + int_game.B) == 1) || ((int_game.A + int_game.B - 1) % 2 == 0)) {
				serve_swap();
				serve_alternate();
			}else{
				serve_alternate();
			}
		}
	}
}

static void process_data_chain() {
	int_game.A			= 0;
	int_game.B			= 0;
	int_set.A			= 0;
	int_set.B			= 0;
	int_match.A			= 0;
	int_match.B			= 0;

	game_in_match		= false;
	game_in_tiebreak	= false;
	game_over			= false;
	game_serve_pos		= game_serve_start;

	deuce_count			= 0;

	new_point 			= false;	// flag for new point being added

	// New point added, or just redrawing the page?
	if (last_data_position != data_position) {
		new_point = true;
	}
	
	for (int i=0; i < data_position; i++) {
		/*
			Very Basic Compression - allows for maybe 500 logged actions
			----------------------
			A = Player 2
			B = Player 1
			
			C = AB
			D = BA
			
			E = ABA [CA]
			F = ABB [CB]

			G = BAA [DA]
			H = BAB [DB]

		*/

		switch (data_points[i]){
			case 'A': int_game.A++; break;
			case 'B': int_game.B++; break;
			case 'C': int_game.A++; process_game(); int_game.B++; break;
			case 'D': int_game.B++; process_game(); int_game.A++; break;
			case 'E': int_game.A++; process_game(); int_game.B++; process_game(); int_game.A++; break;
			case 'F': int_game.A++; process_game(); int_game.B++; process_game(); int_game.B++; break;
			case 'G': int_game.B++; process_game(); int_game.A++; process_game(); int_game.A++; break;
			case 'H': int_game.B++; process_game(); int_game.A++; process_game(); int_game.B++; break;
		}
			
		process_game();
	}

	// Avoid Corruption
	if (data_position == 255) {
		game_over = true;
	}

	// Anything Special Need to Happen?
	if (new_point && game_over) {
		vibes_long_pulse();
	}

	if (new_point && !game_in_match) {
		game_matches++;
		vibes_double_pulse();
		data_set_times[game_matches - 1] 	= game_timer;
		data_set_scores[game_matches - 1]	= int_last_set;

		if (game_in_tiebreak) {
			data_set_scores[game_matches - 1].A	= (100 * int_last_set.A) + int_last_game.A;
			data_set_scores[game_matches - 1].B	= (100 * int_last_set.B) + int_last_game.B;
		}
	}

	last_data_position = data_position;

	window_main_redraw();
}

