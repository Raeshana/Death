#include "PlayMode.hpp"
#include "data_path.hpp"
#include "gl_compile_program.hpp"
#include "Load.hpp"

#include <iostream>
#include <cmath>
#include <cstring>
#include "SoulCaptureMode.hpp"

const int MapWidth = 32;
const int MapHeight = 30;

const float TileSize = 8.0f;
const float ReaperSize = 16.0f;

static const char *ROOM_MAP_1[MapHeight] = {
	"################################",
	"#..............................#",
	"#...........CCBBBBII...........#",
	"#...........CCBBBBII...........#",
	"#...........CCBBBBII...........#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#...............P..............#",
	"#..............................#",
	"#.....TT.......................#",
	"#.....TT.......................#",
	"#.....D........................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_2[MapHeight] = {
	"################################",
	"#..............................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.................TT..#",
	"#...BBBB...................TT..#",
	"#...BBBB...................D...#",
	"#...BBBB.......................#",
	"#.....P........................#",
	"#..............................#",
	"#..............................#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#..............................#",
	"#..............................#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_3[MapHeight] = {
	"################################",
	"#..............................#",
	"#...........CCBBBBII...........#",
	"#...........CCBBBBII...........#",
	"#...........CCBBBBII...........#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#...............P..............#",
	"#..............................#",
	"#.....TT.......................#",
	"#.....TT.......................#",
	"#.....D........................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_4[MapHeight] = {
	"################################",
	"#..............................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.................TT..#",
	"#...BBBB...................TT..#",
	"#...BBBB...................D...#",
	"#...BBBB.......................#",
	"#.....P........................#",
	"#..............................#",
	"#..............................#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#..............................#",
	"#..............................#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_5[MapHeight] = {
	"################################",
	"#..............................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.................TT..#",
	"#...BBBB...................TT..#",
	"#...BBBB...................D...#",
	"#...BBBB.......................#",
	"#.....P........................#",
	"#..............................#",
	"#..............................#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#..............................#",
	"#..............................#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............X...............#",
	"###############==###############",
};

static const char **ROOM_MAPS[5] = {
	ROOM_MAP_1,
	ROOM_MAP_2,
	ROOM_MAP_3,
	ROOM_MAP_4,
	ROOM_MAP_5,
};

Load< Sound::Sample > after_soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("Hospital_Theme.wav"));
});

Load< Sound::Sample > during_soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("soul_capture.wav"));
});

Load< Sound::Sample > before_soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("Hospital_Theme.wav"));
});

PlayMode::PlayMode(int room_number_) : room_number(room_number_) {
	// sprites --------------------------------------------------------------------------------------------
	if (room_number < 1 || room_number > 5) {
		throw std::runtime_error("No map for room " + std::to_string(room_number));
	}
	const char **room_map = ROOM_MAPS[room_number - 1];

	room.resize(MapHeight);

	for (int row = 0; row < MapHeight; row++) {
		if (std::strlen(room_map[row]) != MapWidth) {
			throw std::runtime_error("ROOM_MAP row " + std::to_string(row) + " is not " + std::to_string(MapWidth) + " characters long.");
		}

		int y = MapHeight - 1 - row;
		room[y] = room_map[row];

		for (int x = 0; x < MapWidth; x++) {
			char c = room_map[row][x];
			if (c == 'P') {
				Interactable patient;
				patient.name = "patient";
				patient.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				patient.radius = 20.0f;
				interactables.push_back(patient);
			} else if (c == 'D') {
				Interactable diary;
				diary.name = "diary";
				diary.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				diary.radius = 10.0f;
				interactables.push_back(diary);
			} else if (c == 'F') {
				Interactable father;
				father.name = "father";
				father.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				father.radius = 10.0f;
				interactables.push_back(father);
			} else if (c == 'N') {
				Interactable nurse;
				nurse.name = "nurse";
				nurse.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				nurse.radius = 10.0f;
				interactables.push_back(nurse);
			} else if (c == 'X') {
				Interactable exit;
				exit.name = "exit";
				exit.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				exit.radius = 20.0f;
				interactables.push_back(exit);
			}
		}
	}

	reaper_tex = sprites.load(data_path("reaper.png"));
	
	patient1_tex = sprites.load(data_path("patient" + std::to_string(room_number) + ".png"));
	man1_tex = sprites.load(data_path("man1.png"));
	woman1_tex = sprites.load(data_path("woman1.png"));
	partition_tex = sprites.load(data_path("partition.png"));
	cardiogram_tex = sprites.load(data_path("cardiogram.png"));
	iv_tex = sprites.load(data_path("IV.png"));
	desk_tex = sprites.load(data_path("desk.png"));
	interact_tex = sprites.load(data_path("interact.png"));

	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			char c = room[y][x];

			Sprites::Texture tex;
			if (c == 'B') {
				tex = patient1_tex;
			} else if (c == 'M') {
				tex = man1_tex;
			} else if (c == 'W') {
				tex = woman1_tex;
			} else if (c == '|') {
				tex = partition_tex;
			} else if (c == 'C') {
				tex = cardiogram_tex;
			} else if (c == 'I') {
				tex = iv_tex;
			} else if (c == 'T') {
				tex = desk_tex;
			} else {
				continue;
			}

			int block_left = x;
			while (block_left > 0 && room[y][block_left - 1] == c) {
				block_left = block_left - 1;
			}
			int block_bottom = y;
			while (block_bottom > 0 && room[block_bottom - 1][x] == c) {
				block_bottom = block_bottom - 1;
			}

			int image_w = int(tex.size.x) / int(TileSize);
			int image_h = int(tex.size.y) / int(TileSize);
			if ((x - block_left) % image_w != 0) {
				continue;
			}
			if ((y - block_bottom) % image_h != 0) {
				continue;
			}

			Prop prop;
			prop.tex = tex;
			prop.pos = glm::vec2(x * TileSize, y * TileSize);
			props.push_back(prop);
		}
	}

	floor_tex = sprites.load(data_path("tiles.png"));

	// text --------------------------------------------------------------------------------------------
	// ink::compiler::run("test.json", "test.bin");
	hint_line = "WASD MOVE   E INTERACT   Talk to the patient first.\n";
	myInk = ink::runtime::story::from_file(data_path("prototype_script.bin").c_str());
	thread = myInk->new_runner();

	// sound --------------------------------------------------------------------------------------------
	Sound::loop(*before_soul_capture, 1.0f, 0.0f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	(void)window_size;

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_A) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_E) {
			interact.downs += 1;
			interact.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_A) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_E) {
			interact.pressed = false;
			return true;
		}
	}

	return false;
}

bool PlayMode::tile_is_solid(int tx, int ty) {
	if (tx < 0 || ty < 0 || tx >= MapWidth || ty >= MapHeight) {
		return true;
	}
	char c = room[ty][tx];
	if (c == '#' || c == 'B' || c == 'T' || c == '=' || c == '|' || c == 'C' || c == 'I' || c == 'M' || c == 'W') {
		return true;
	}
	return false;
}

bool PlayMode::can_move_to(glm::vec2 pos) {
	float left_x = pos.x;
	float right_x = pos.x + ReaperSize - 1.0f;
	float bottom_y = pos.y;
	float top_y = pos.y + ReaperSize - 1.0f;

	if (tile_is_solid(int(left_x) / 8, int(bottom_y) / 8)) {
		return false;
	}
	if (tile_is_solid(int(right_x) / 8, int(bottom_y) / 8)) {
		return false;
	}
	if (tile_is_solid(int(left_x) / 8, int(top_y) / 8)) {
		return false;
	}
	if (tile_is_solid(int(right_x) / 8, int(top_y) / 8)) {
		return false;
	}
	return true;
}

void PlayMode::play_music(Sound::Sample const &sample, std::shared_ptr< Sound::PlayingSample > &slot) {
	for (auto *other : { &before_soul_loop, &during_soul_loop, &after_soul_loop }) {
		if (*other) {
			(*other)->stop(1.0f);
			*other = nullptr;
		}
	}
	slot = Sound::loop(sample, 0.0f, 0.0f);
	slot->set_volume(1.0f, 1.0f);
}

void PlayMode::on_interact(Interactable const &thing) {
	if (thing.name == "patient") {
		if (soul_taken) {
			hint_line = "This soul has already been collected.";;
			return;
		}

		std::shared_ptr< DialogueMode > dialogue = std::make_shared< DialogueMode >("prototype_script.bin");
		dialogue->return_to = shared_from_this();
		active_dialogue = dialogue;

		reset_buttons();
		Mode::set_current(dialogue);

	} else if (thing.name == "diary") {
		diary_read = true;
		hint_line = "You read the diary. The patient seems calmer now.";
	} else if (thing.name == "father") {
		hint_line = "You talked to the father. (TODO: tell story)";
	} else if (thing.name == "nurse") {
		hint_line = "You talked to the nurse. (TODO: tell story)";
	} else if (thing.name == "exit") {
		reset_buttons();
		std::shared_ptr< Mode > back = return_to;
		Mode::set_current(back);
	}
}

void PlayMode::update(float elapsed) {

	// change w/ patient struct to keep track of info
	if (active_dialogue && Mode::current.get() == this) {
		bool battle = active_dialogue->can_battle;
		active_dialogue.reset();
		if (battle) {
			start_minigame();
			return;
		}
	}

	hover_time = hover_time + elapsed;
	if (hover_time > 1000.0f) {
		hover_time = hover_time - 1000.0f;
	}

	const float PlayerSpeed = 60.0f;

	glm::vec2 move = glm::vec2(0.0f);
	if (left.pressed) {
		move.x -= 1.0f;
	}
	if (right.pressed) {
		move.x += 1.0f;
	}
	if (down.pressed) {
		move.y -= 1.0f;
	}
	if (up.pressed) {
		move.y += 1.0f;
	}
	if (move != glm::vec2(0.0f)) {
		move = glm::normalize(move) * PlayerSpeed * elapsed;
	}

	glm::vec2 try_x = glm::vec2(player_at.x + move.x, player_at.y);
	if (can_move_to(try_x)) {
		player_at.x = try_x.x;
	}
	glm::vec2 try_y = glm::vec2(player_at.x, player_at.y + move.y);
	if (can_move_to(try_y)) {
		player_at.y = try_y.y;
	}

	nearby = -1;
	float best_dist = 1e9f;
	glm::vec2 player_center = player_at + glm::vec2(ReaperSize * 0.5f);
	for (size_t i = 0; i < interactables.size(); i++) {
		float dist = glm::length(interactables[i].pos - player_center);
		if (dist < interactables[i].radius && dist < best_dist) {
			best_dist = dist;
			nearby = int(i);
		}
	}

	if (interact.downs > 0 && nearby != -1) {
		on_interact(interactables[nearby]);
	}

	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	interact.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	// sprites --------------------------------------------------------------------------------------------
	sprites.begin(drawable_size);

	const float FloorTileSize = 32.0f;
	for (float y = 0.0f; y < MapHeight * TileSize; y += FloorTileSize) {
		for (float x = 0.0f; x < MapWidth * TileSize; x += FloorTileSize) {
			sprites.draw(floor_tex, glm::vec2(x, y), glm::vec2(FloorTileSize));
		}
	}

	glm::vec4 wall_color = glm::vec4(0x60 / 255.0f, 0x70 / 255.0f, 0x78 / 255.0f, 1.0f);
	glm::vec4 door_color = glm::vec4(0x40 / 255.0f, 0x48 / 255.0f, 0x50 / 255.0f, 1.0f);
	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			char c = room[y][x];
			if (c == '#') {
				sprites.draw(sprites.white, glm::vec2(x * TileSize, y * TileSize), glm::vec2(TileSize), wall_color);
			} else if (c == '=') {
				sprites.draw(sprites.white, glm::vec2(x * TileSize, y * TileSize), glm::vec2(TileSize), door_color);
			}
		}
	}

	float hover = 2.0f * std::sin(hover_time * 3.0f);
	sprites.draw(reaper_tex, glm::vec2(player_at.x, player_at.y + hover), glm::vec2(ReaperSize));
	for (size_t i = 0; i < props.size(); i++) {
		sprites.draw(props[i].tex, props[i].pos, glm::vec2(props[i].tex.size));
	}

	if (nearby != -1) {
		glm::vec2 marker_size = glm::vec2(interact_tex.size);
		glm::vec2 marker_pos = glm::vec2(player_at.x + (ReaperSize - marker_size.x) * 0.5f, player_at.y + ReaperSize + 2.0f + hover);
		sprites.draw(interact_tex, marker_pos, marker_size);
	}

	sprites.end(drawable_size);

	// text --------------------------------------------------------------------------------------------
	
	//controls hint along the bottom of the screen:
	// hint_processor.textbox_width = float(drawable_size.x);
	// hint_processor.textbox_height = FONT_SIZE;
	// hint_processor.pen_x_start = (soul_taken ? drawable_size.x * 0.5f - 11.0f * FONT_SIZE : float(FONT_SIZE));
	// hint_processor.pen_y_start = (soul_taken ? drawable_size.y * 0.5f : FONT_SIZE * 1.5f);
	// hint_processor.handle_curr_line(hint_line);
	// hint_processor.draw_curr_line(drawable_size);
}

void PlayMode::reset_buttons() {
	left.pressed = false;
	right.pressed = false;
	up.pressed = false;
	down.pressed = false;
	interact.pressed = false;
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	interact.downs = 0;
}

void PlayMode::start_minigame() {
	SoulCaptureMode::Difficulty difficulty = SoulCaptureMode::hard();
	if (diary_read) {
		difficulty = SoulCaptureMode::normal();
	}

	std::shared_ptr< SoulCaptureMode > minigame = std::make_shared< SoulCaptureMode >(difficulty);
	minigame->return_to = shared_from_this();
	soul_taken = true;

	reset_buttons();
	Mode::set_current(minigame);
}