#include "PlayMode.hpp"
#include "data_path.hpp"
#include "gl_compile_program.hpp"

#include <iostream>
#include <cmath>
#include <cstring>
#include "SoulCaptureMode.hpp"

const int MapWidth = 32;
const int MapHeight = 30;

const float TileSize = 8.0f;
const float ReaperSize = 16.0f;

static void draw_floor_shadow(Sprites &sprites, float center_x, float floor_y, float width) {
	glm::vec4 shadow_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.4f);
	float narrow = width - 4.0f;
	sprites.draw(sprites.white, glm::vec2(center_x - narrow * 0.5f, floor_y + 1.0f), glm::vec2(narrow, 1.0f), shadow_color);
	sprites.draw(sprites.white, glm::vec2(center_x - width * 0.5f, floor_y - 1.0f), glm::vec2(width, 2.0f), shadow_color);
	sprites.draw(sprites.white, glm::vec2(center_x - narrow * 0.5f, floor_y - 2.0f), glm::vec2(narrow, 1.0f), shadow_color);
}

static const char *ROOM_MAP_1[MapHeight] = {
	"################################",
	"################################",
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
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_2[MapHeight] = {
	"################################",
	"################################",
	"################################",
	"#..............................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.................TT..#",
	"#...BBBB...................TT..#",
	"#...BBBB...................D...#",
	"#...BBBB.......................#",
	"#.....P........................#",
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

PlayMode::PlayMode(int room_number_) : room_number(room_number_) {
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
				bed_center_x = x * TileSize;
				lights.push_back(glm::vec2(x * TileSize, y * TileSize + 8.0f));

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
	vignette_tex = sprites.load(data_path("vignette.png"));
	light_tex = sprites.load(data_path("light.png"));

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
	lights.push_back(glm::vec2(MapWidth * TileSize * 0.5f, MapHeight * TileSize * 0.4f));

	floor_tex = sprites.load(data_path("tiles.png"));
	light_tex = sprites.load(data_path("light.png"));

	floor_tex = sprites.load(data_path("tiles.png"));
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

void PlayMode::on_interact(Interactable const &thing) {
	if (thing.name == "patient") {
		if (soul_taken) {
			std::cout << "This soul has already been collected." << std::endl;
			return;
		}

		std::shared_ptr< DialogueMode > dialogue = std::make_shared< DialogueMode >("prototype_script.bin");
		dialogue->return_to = shared_from_this();
		active_dialogue = dialogue;

		reset_buttons();
		Mode::set_current(dialogue);
	} else if (thing.name == "diary") {
		diary_read = true;
		std::cout << "You read the diary. The patient seems calmer now." << std::endl;
	} else if (thing.name == "father") {
		std::cout << "You talked to the father. (TODO: tell story)" << std::endl;
	} else if (thing.name == "nurse") {
		std::cout << "You talked to the nurse. (TODO: tell story)" << std::endl;
	} else if (thing.name == "exit") {
		start_leaving(return_to);
	}
}

static bool room_is_floor(std::vector< std::string > const &room, int x, int y) {
	if (x < 0 || y < 0 || x >= MapWidth || y >= MapHeight) {
		return false;
	}
	char c = room[y][x];
	if (c == '#' || c == '=') {
		return false;
	}
	return true;
}

static int room_floor_distance_below(std::vector< std::string > const &room, int x, int y) {
	int distance = 1;
	for (int yy = y - 1; yy >= 0; yy--) {
		if (room_is_floor(room, x, yy)) {
			return distance;
		}
		distance = distance + 1;
	}
	return 1000;
}

void PlayMode::update(float elapsed) {

	const float FadeTime = 0.4f;
	fade_in = fade_in - elapsed / FadeTime;
	if (fade_in < 0.0f) {
		fade_in = 0.0f;
	}

	if (leaving_to) {
		fade_out = fade_out + elapsed / FadeTime;
		if (fade_out >= 1.0f) {
			auto self = shared_from_this();
			std::shared_ptr< Mode > next = leaving_to;
			leaving_to.reset();
			fade_out = 0.0f;
			Mode::set_current(next);
		}
		return;
	}

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
	sprites.begin(drawable_size);

	const float FloorTileSize = 32.0f;

	glm::vec4 floor_tint = glm::vec4(0.52f, 0.58f, 0.62f, 1.0f);
	for (float y = 0.0f; y < MapHeight * TileSize; y += FloorTileSize) {
		for (float x = 0.0f; x < MapWidth * TileSize; x += FloorTileSize) {
			sprites.draw(floor_tex, glm::vec2(x, y), glm::vec2(FloorTileSize), floor_tint);
		}
	}

	glm::vec4 wall_color      = glm::vec4(0x60 / 255.0f, 0x70 / 255.0f, 0x78 / 255.0f, 1.0f);
	glm::vec4 wall_face_color = glm::vec4(0x3b / 255.0f, 0x46 / 255.0f, 0x4b / 255.0f, 1.0f);
	glm::vec4 baseboard_color = glm::vec4(0x26 / 255.0f, 0x2d / 255.0f, 0x32 / 255.0f, 1.0f);
	glm::vec4 wall_edge_color = glm::vec4(0x7a / 255.0f, 0x88 / 255.0f, 0x8c / 255.0f, 1.0f);
	glm::vec4 wall_shadow_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.25f);
	glm::vec4 door_color      = glm::vec4(0x40 / 255.0f, 0x48 / 255.0f, 0x50 / 255.0f, 1.0f);

	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			char c = room[y][x];
			glm::vec2 pos = glm::vec2(x * TileSize, y * TileSize);

			if (c == '#') {
				const int WallFaceHeight = 2;
				int dist = room_floor_distance_below(room, x, y);

				if (dist <= WallFaceHeight) {
					sprites.draw(sprites.white, pos, glm::vec2(TileSize), wall_face_color);
					if (dist == 1) {
						sprites.draw(sprites.white, pos, glm::vec2(TileSize, 2.0f), baseboard_color);
						sprites.draw(sprites.white, pos + glm::vec2(0.0f, -2.0f), glm::vec2(TileSize, 2.0f), wall_shadow_color);
					}
					if (dist == WallFaceHeight) {
						sprites.draw(sprites.white, pos + glm::vec2(0.0f, TileSize - 1.0f), glm::vec2(TileSize, 1.0f), wall_edge_color);
					}
				} else {
					sprites.draw(sprites.white, pos, glm::vec2(TileSize), wall_color);
					if (room_is_floor(room, x, y + 1)) {
						sprites.draw(sprites.white, pos + glm::vec2(0.0f, TileSize - 1.0f), glm::vec2(TileSize, 1.0f), wall_edge_color);
					}
				}
			} else if (c == '=') {
				sprites.draw(sprites.white, pos, glm::vec2(TileSize), door_color);
			}
		}
	}

	glm::vec4 frame_dark  = glm::vec4(0x2a / 255.0f, 0x32 / 255.0f, 0x36 / 255.0f, 1.0f);
	glm::vec4 night_sky   = glm::vec4(0x18 / 255.0f, 0x21 / 255.0f, 0x3a / 255.0f, 1.0f);
	glm::vec4 moon_color  = glm::vec4(0xd9 / 255.0f, 0xe2 / 255.0f, 0xc8 / 255.0f, 1.0f);
	glm::vec4 star_color  = glm::vec4(0x9f / 255.0f, 0xb0 / 255.0f, 0xd0 / 255.0f, 1.0f);
	glm::vec4 paper_color = glm::vec4(0xc9 / 255.0f, 0xc4 / 255.0f, 0xb4 / 255.0f, 1.0f);
	glm::vec4 ink_color   = glm::vec4(0x7a / 255.0f, 0x74 / 255.0f, 0x66 / 255.0f, 1.0f);
	glm::vec4 clock_face  = glm::vec4(0xd8 / 255.0f, 0xdc / 255.0f, 0xd6 / 255.0f, 1.0f);

	glm::vec2 win = glm::vec2(std::floor(bed_center_x - 12.0f), 228.0f);
	sprites.draw(sprites.white, win, glm::vec2(24.0f, 10.0f), frame_dark);
	sprites.draw(sprites.white, win + glm::vec2(1.0f, 1.0f), glm::vec2(22.0f, 8.0f), night_sky);
	sprites.draw(sprites.white, win + glm::vec2(16.0f, 4.0f), glm::vec2(3.0f, 3.0f), moon_color);
	sprites.draw(sprites.white, win + glm::vec2(17.0f, 5.0f), glm::vec2(2.0f, 2.0f), night_sky);
	sprites.draw(sprites.white, win + glm::vec2(4.0f, 6.0f), glm::vec2(1.0f, 1.0f), star_color);
	sprites.draw(sprites.white, win + glm::vec2(8.0f, 3.0f), glm::vec2(1.0f, 1.0f), star_color);
	sprites.draw(sprites.white, win + glm::vec2(21.0f, 2.0f), glm::vec2(1.0f, 1.0f), star_color);
	sprites.draw(sprites.white, win + glm::vec2(11.0f, 1.0f), glm::vec2(2.0f, 8.0f), frame_dark);
	sprites.draw(sprites.white, win + glm::vec2(-1.0f, -1.0f), glm::vec2(26.0f, 1.0f), wall_edge_color);

	glm::vec2 chart = glm::vec2(std::floor(bed_center_x - 30.0f), 228.0f);
	sprites.draw(sprites.white, chart, glm::vec2(6.0f, 9.0f), paper_color);
	sprites.draw(sprites.white, chart + glm::vec2(2.0f, 8.0f), glm::vec2(2.0f, 2.0f), frame_dark);
	sprites.draw(sprites.white, chart + glm::vec2(1.0f, 6.0f), glm::vec2(4.0f, 1.0f), ink_color);
	sprites.draw(sprites.white, chart + glm::vec2(1.0f, 4.0f), glm::vec2(3.0f, 1.0f), ink_color);
	sprites.draw(sprites.white, chart + glm::vec2(1.0f, 2.0f), glm::vec2(4.0f, 1.0f), ink_color);

	glm::vec2 clock = glm::vec2(std::floor(bed_center_x + 24.0f), 229.0f);
	sprites.draw(sprites.white, clock, glm::vec2(7.0f, 7.0f), frame_dark);
	sprites.draw(sprites.white, clock + glm::vec2(1.0f, 1.0f), glm::vec2(5.0f, 5.0f), clock_face);
	sprites.draw(sprites.white, clock + glm::vec2(3.0f, 3.0f), glm::vec2(1.0f, 3.0f), frame_dark);
	sprites.draw(sprites.white, clock + glm::vec2(3.0f, 3.0f), glm::vec2(2.0f, 1.0f), frame_dark);


	glm::vec4 light_tint = glm::vec4(1.0f, 1.0f, 1.0f, 0.35f);
	for (size_t i = 0; i < lights.size(); i++) {
		glm::vec2 size = glm::vec2(light_tex.size);
		sprites.draw(light_tex, lights[i] - size * 0.5f, size, light_tint);
	}

	float hover = 2.0f * std::sin(hover_time * 3.0f);

	glm::vec4 prop_shadow_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.35f);
	glm::vec2 prop_shadow_offset = glm::vec2(2.0f, -2.0f);
	for (size_t i = 0; i < props.size(); i++) {
		sprites.draw(props[i].tex, props[i].pos + prop_shadow_offset, glm::vec2(props[i].tex.size), prop_shadow_color);
	}

	float shadow_width = 12.0f - hover;
	draw_floor_shadow(sprites, player_at.x + ReaperSize * 0.5f, player_at.y - 3.0f, shadow_width);

	for (size_t i = 0; i < props.size(); i++) {
		if (props[i].pos.y >= player_at.y) {
			sprites.draw(props[i].tex, props[i].pos, glm::vec2(props[i].tex.size));
		}
	}

	sprites.draw(reaper_tex, glm::vec2(player_at.x, player_at.y + hover), glm::vec2(ReaperSize));

	for (size_t i = 0; i < props.size(); i++) {
		if (props[i].pos.y < player_at.y) {
			sprites.draw(props[i].tex, props[i].pos, glm::vec2(props[i].tex.size));
		}
	}

	glm::vec2 screen_size = glm::vec2(MapWidth * TileSize, MapHeight * TileSize);

	glm::vec4 cold_tint = glm::vec4(0.44f, 0.65f, 0.77f, 0.06f);
	sprites.draw(sprites.white, glm::vec2(0.0f, 0.0f), screen_size, cold_tint);

	sprites.draw(vignette_tex, glm::vec2(0.0f, 0.0f), screen_size);

	if (nearby != -1) {
		glm::vec2 marker_size = glm::vec2(interact_tex.size);
		glm::vec2 marker_pos = glm::vec2(player_at.x + (ReaperSize - marker_size.x) * 0.5f, player_at.y + ReaperSize + 2.0f + hover);
		sprites.draw(interact_tex, marker_pos, marker_size);
	}

	float fade = fade_in;
	if (fade_out > fade) {
		fade = fade_out;
	}
	if (fade > 0.0f) {
		glm::vec4 fade_color = glm::vec4(0.0f, 0.0f, 0.0f, fade);
		sprites.draw(sprites.white, glm::vec2(0.0f, 0.0f), screen_size, fade_color);
	}

	sprites.end(drawable_size);
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

	start_leaving(minigame);
}

void PlayMode::start_leaving(std::shared_ptr< Mode > next) {
	leaving_to = next;
	fade_out = 0.0f;
	reset_buttons();
}