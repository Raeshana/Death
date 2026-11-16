#include "HallwayMode.hpp"
#include "PlayMode.hpp"

#include "data_path.hpp"

#include <cmath>
#include <cstring>

const float TileSize = 8.0f;
const float ReaperSize = 16.0f;
const float FloorTileSize = 32.0f;

const int MapWidth = 32;
const int MapHeight = 30;

static const char *HALLWAY_MAP[MapHeight] = {
	"################################",
	"################################",
	"################################",
	"################################",
	"################################",
	"###11####22####33####44####55###",
	"###11####22####33####44####55###",
	"###11####22####33####44####55###",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"################################",
};

static bool is_door_char(char c) {
	if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5') {
		return true;
	}
	return false;
}

HallwayMode::HallwayMode() {
	hallway.resize(MapHeight);
	for (int row = 0; row < MapHeight; row++) {
		if (std::strlen(HALLWAY_MAP[row]) != MapWidth) {
			throw std::runtime_error("HALLWAY_MAP row " + std::to_string(row) + " is not " + std::to_string(MapWidth) + " characters long.");
		}
		int y = MapHeight - 1 - row;
		hallway[y] = HALLWAY_MAP[row];
	}

	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			char c = hallway[y][x];
			if (!is_door_char(c)) {
				continue;
			}
			if (x > 0 && hallway[y][x - 1] == c) {
				continue;
			}
			if (y > 0 && hallway[y - 1][x] == c) {
				continue;
			}
			Door door;
			door.number = c - '0';
			door.pos = glm::vec2(x * TileSize + 8.0f, y * TileSize);
			doors.push_back(door);
		}
	}

	reaper_tex = sprites.load(data_path("reaper.png"));
	floor_tex = sprites.load(data_path("tiles.png"));
	interact_tex = sprites.load(data_path("interact.png"));
}

HallwayMode::~HallwayMode() {
}

bool HallwayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
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

bool HallwayMode::tile_is_solid(int tx, int ty) {
	if (tx < 0 || ty < 0 || tx >= MapWidth || ty >= MapHeight) {
		return true;
	}
	char c = hallway[ty][tx];
	if (c == '#' || is_door_char(c)) {
		return true;
	}
	return false;
}

bool HallwayMode::can_move_to(glm::vec2 pos) {
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

void HallwayMode::reset_buttons() {
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

void HallwayMode::enter_door(int number) {
	int index = number - 1;

	if (!rooms[index]) {
		std::shared_ptr< PlayMode > room = std::make_shared< PlayMode >(number);
		room->return_to = shared_from_this();
		rooms[index] = room;
	}

	reset_buttons();
	Mode::set_current(rooms[index]);
}

void HallwayMode::update(float elapsed) {
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

	const float DoorRadius = 16.0f;
	nearby = -1;
	float best_dist = 1e9f;
	glm::vec2 player_center = player_at + glm::vec2(ReaperSize * 0.5f);
	for (size_t i = 0; i < doors.size(); i++) {
		float dist = glm::length(doors[i].pos - player_center);
		if (dist < DoorRadius && dist < best_dist) {
			best_dist = dist;
			nearby = int(i);
		}
	}

	if (interact.downs > 0 && nearby != -1) {
		enter_door(doors[nearby].number);
		return;
	}

	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	interact.downs = 0;
}

void HallwayMode::draw(glm::uvec2 const &drawable_size) {
	sprites.begin(drawable_size);

	for (float y = 0.0f; y < MapHeight * TileSize; y += FloorTileSize) {
		for (float x = 0.0f; x < MapWidth * TileSize; x += FloorTileSize) {
			sprites.draw(floor_tex, glm::vec2(x, y), glm::vec2(FloorTileSize));
		}
	}

	glm::vec4 wall_color = glm::vec4(0x60 / 255.0f, 0x70 / 255.0f, 0x78 / 255.0f, 1.0f);
	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			if (hallway[y][x] == '#') {
				sprites.draw(sprites.white, glm::vec2(x * TileSize, y * TileSize), glm::vec2(TileSize), wall_color);
			}
		}
	}

	glm::vec4 door_color = glm::vec4(0x8a / 255.0f, 0x5a / 255.0f, 0x3a / 255.0f, 1.0f);
	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			if (is_door_char(hallway[y][x])) {
				sprites.draw(sprites.white, glm::vec2(x * TileSize, y * TileSize), glm::vec2(TileSize), door_color);
			}
		}
	}

	float hover = 2.0f * std::sin(hover_time * 3.0f);
	sprites.draw(reaper_tex, glm::vec2(player_at.x, player_at.y + hover), glm::vec2(ReaperSize));

	if (nearby != -1) {
		glm::vec2 marker_size = glm::vec2(interact_tex.size);
		glm::vec2 marker_pos = glm::vec2(
			player_at.x + (ReaperSize - marker_size.x) * 0.5f,
			player_at.y + ReaperSize + 2.0f + hover
		);
		sprites.draw(interact_tex, marker_pos, marker_size);
	}

	sprites.end(drawable_size);
}