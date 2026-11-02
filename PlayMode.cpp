#include "PlayMode.hpp"
#include "data_path.hpp"

#include <iostream>
#include <cmath>
#include <cstring>

const int MapWidth = 32;
const int MapHeight = 30;

const float TileSize = 8.0f;
const float ReaperSize = 16.0f;

static const char *ROOM_MAP[MapHeight] = {
	"################################",
	"#..............................#",
	"#..........................WWW.#",
	"#.........................NWWW.#",
	"#..........................WWW.#",
	"#..............................#",
	"#....BBBB...||....||...BBBB....#",
	"#..IIBBBBCC.||....||.IIBBBBCC..#",
	"#..IIBBBBCC.||....||.IIBBBBCC..#",
	"#....BBBB.P.||....||.P.BBBB....#",
	"#..TTBBBB...||....||...BBBBTT..#",
	"#..TTBBBB...||....||...BBBBTT..#",
	"#..D........||....||........D..#",
	"#...........||....||...........#",
	"#...........||....||...........#",
	"#MMM........||....||...........#",
	"#MMM........||....||...........#",
	"#MMM........||....||...........#",
	"#.F............................#",
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
	"###############==###############",
};

PlayMode::PlayMode() {
	room.resize(MapHeight);
	for (int row = 0; row < MapHeight; row++) {
		if (std::strlen(ROOM_MAP[row]) != MapWidth) {
			throw std::runtime_error("ROOM_MAP row " + std::to_string(row) + " is not " + std::to_string(MapWidth) + " characters long.");
		}

		int y = MapHeight - 1 - row;
		room[y] = ROOM_MAP[row];

		for (int x = 0; x < MapWidth; x++) {
			char c = ROOM_MAP[row][x];
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
			}
		}
	}

	reaper_tex = sprites.load(data_path("reaper.png"));
	
	patient1_tex = sprites.load(data_path("patient1.png"));
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
		std::cout << "You approach the patient. (TODO: Reap / Talk choice)" << std::endl;
	} else if (thing.name == "diary") {
		std::cout << "You open the diary. (TODO: show diary text)" << std::endl;
	} else if (thing.name == "father") {
		std::cout << "You talked to the father. (TODO: tell story)" << std::endl;
	} else if (thing.name == "nurse") {
		std::cout << "You talked to the nurse. (TODO: tell story)" << std::endl;
	}
}

void PlayMode::update(float elapsed) {
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
}