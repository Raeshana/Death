#include "RoomMode.hpp"

#include "DrawLines.hpp"

#include <iostream>
#include <cmath>
#include <cstring>

#include "gl_errors.hpp"
#include "Load.hpp"

#include <glm/gtc/type_ptr.hpp>

const int MapWidth = 32;
const int MapHeight = 30;

const float TileSize = 8.0f;
const float ReaperSize = 16.0f;

static const char *ROOM_MAP[MapHeight] = {
	"################################",
	"#..............................#",
	"#..............................#",
	"#.............BBBB.............#",
	"#...........P.BBBB.............#",
	"#.............BBBB..TT.........#",
	"#.............BBBB..TT.........#",
	"#.............BBBB...D.........#",
	"#.............BBBB.............#",
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
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"###############==###############",
};

// text vertex shader
const char *text_vertex_shader = R"(
	#version 330

	layout(location = 0) in vec2 Position;
	layout(location = 1) in vec2 TexCoord;

	out vec2 texCoord;

	uniform mat4 OBJECT_TO_CLIP;

	void main() {
		gl_Position = OBJECT_TO_CLIP * vec4(Position, 0.0, 1.0);
		texCoord = TexCoord;
	}
)";

// text fragment shader
const char *text_fragment_shader = R"(
	#version 330

	in vec2 texCoord;

	uniform sampler2D glyphTexture;

	out vec4 fragColor;

	void main() {
		float alpha = texture(glyphTexture, texCoord).r;
		fragColor = vec4(1.0, 1.0, 1.0, alpha);
	}
)";

Load< Sound::Sample > after_soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("after_soul_loop.wav"));
});

Load< Sound::Sample > during_soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("during_soul_loop.wav"));
});

Load< Sound::Sample > before_soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("before_soul_loop.wav"));
});

RoomMode::RoomMode() {
	//----- sprite stuff -----
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
			}
		}
	}

	reaper_tex = sprites.load(data_path("reaper.png"));
	bed_tex = sprites.load(data_path("bed.png"));
	bool found_bed = false;
		for (int y = 0; y < MapHeight && !found_bed; y++) {
		for (int x = 0; x < MapWidth && !found_bed; x++) {
			if (room[y][x] == 'B') {
				bed_pos = glm::vec2(x * TileSize, y * TileSize);
				found_bed = true;
			}
		}
	}

	desk_tex = sprites.load(data_path("desk.png"));
	bool found_desk = false;
	for (int y = 0; y < MapHeight && !found_desk; y++) {
		for (int x = 0; x < MapWidth && !found_desk; x++) {			if (room[y][x] == 'T') {
				desk_pos = glm::vec2(x * TileSize, y * TileSize);
				found_desk = true;
			}
		}
	}

	floor_tex = sprites.load(data_path("tiles.png"));


	//----- text stuff -----
	// ink::compiler::run("test.json", "test.bin");
	myInk = ink::runtime::story::from_file(data_path("prototype_script.bin").c_str());
	thread = myInk->new_runner();

	//----- sound stuff -----
	Sound::loop(*before_soul_capture, 1.0f, 0.0f);
}

RoomMode::~RoomMode() {
}

bool RoomMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
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
		} else if (evt.key.key == SDLK_E) { // interaction
			interact.downs += 1;
			interact.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_SPACE){ // progress dialogue
			curr_line = ""; // restart line
			// can the runner continue?
			// yes - get the next line
			// no - at choice or end
			if ( thread->can_continue() ){
				curr_line = thread->getline();
				if (thread->has_tags()){
					std::cout << "tag avalible: " << "\n" << std::endl;

					for (ink::size_t i = 0; i < thread->num_tags(); ++i){
						std::cout << "tag: " << thread->get_tag(i) << "\n" << std::endl;
					}
				}
				text_processor.handle_curr_line(curr_line); //
			}
			else if (thread->has_choices()) { // display as 3 different choices
				// iterate choices
				if (thread->has_tags()){
					std::cout << "tag avalible: " << "\n" << std::endl;

					for (ink::size_t i = 0; i < thread->num_tags(); ++i){
						std::cout << "tag: " << thread->get_tag(i) << "\n" << std::endl;
					}
				}
				int id = 0;
				for ( const ink::runtime::choice c : *thread )
				{
					id++;
					curr_line.append(c.text());
				}
				text_processor.handle_curr_line(curr_line);
			}
			else { // at the end
				auto self = shared_from_this();
				// loop during soul capture after stopping all sound
				Sound::stop_all_samples();
				Sound::loop(*during_soul_capture, 1.0f, 0.0f);
				capture = std::make_shared< SoulCaptureMode >(read_diary ? SoulCaptureMode::normal() : SoulCaptureMode::hard());
				capture->return_to = self;
				Mode::set_current(capture);

				// loop after soul capture after stopping all sound
				Sound::stop_all_samples();
				Sound::loop(*after_soul_capture, 1.0f, 0.0f);
			}
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
		} else if (evt.key.key == SDLK_E) { // interaction
			interact.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_1) { // dialogue choice 1
			if(thread->has_choices()){
				thread->choose(0);
				curr_line = thread->getline();
				text_processor.handle_curr_line(curr_line);
			}
		} else if (evt.key.key == SDLK_2) { // dialogue choice 2
			if(thread->has_choices()){
				thread->choose(1);
				curr_line = thread->getline();
				text_processor.handle_curr_line(curr_line);
			}
		} else if (evt.key.key == SDLK_3) { // dialogue choice 3
			if(thread->has_choices()){
				thread->choose(2);
				curr_line = thread->getline();
				text_processor.handle_curr_line(curr_line);
			}
		}
	}

	return false;
}

bool RoomMode::tile_is_solid(int tx, int ty) {
	if (tx < 0 || ty < 0 || tx >= MapWidth || ty >= MapHeight) {
		return true;
	}
	char c = room[ty][tx];
	if (c == '#' || c == 'B' || c == 'T' || c == '=') {
		return true;
	}
	return false;
}

bool RoomMode::can_move_to(glm::vec2 pos) {
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

void RoomMode::on_interact(Interactable const &thing) {
	if (thing.name == "patient" && !capture) {
		// talked_patient ? hint = "Space to capture soul" : hint = "Space to talk";
	} else if (thing.name == "diary") {
		read_diary = true;
		// hint = "You read the diary. The patient seems calmer now.";
	}
}

void RoomMode::update(float elapsed) {
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
	glm::vec2 player_center = player_at + glm::vec2(4.0f, 4.0f);
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

void RoomMode::draw(glm::uvec2 const &drawable_size) {
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
	sprites.draw(bed_tex, bed_pos, glm::vec2(bed_tex.size));
	sprites.draw(desk_tex, desk_pos, glm::vec2(desk_tex.size));

	if (nearby != -1) {
		glm::vec4 alert_color = glm::vec4(1.0f, 0xd0 / 255.0f, 0x20 / 255.0f, 1.0f);
		glm::vec2 alert_pos = glm::vec2(player_at.x + 6.0f, player_at.y + ReaperSize + 3.0f + hover);
		sprites.draw(sprites.white, alert_pos, glm::vec2(4.0f, 6.0f), alert_color);
	}

	sprites.end(drawable_size);

	// if (capture) hint = "Soul collected in " + std::to_string(int(capture->timer)) + "s with " + std::to_string(capture->misses) + " misses.";
	// canvas.begin(glm::vec3(0.0f));
	// glDisable(GL_FRAMEBUFFER_SRGB);
	// // ppu.draw(glm::uvec2(PixelCanvas::Width, PixelCanvas::Height));
	// glEnable(GL_FRAMEBUFFER_SRGB);
	// {
	// 	DrawLines lines(canvas.world_to_clip());
	// 	lines.fill_rect(glm::vec2(-PixelCanvas::Aspect, -1.0f), glm::vec2(PixelCanvas::Aspect, -0.86f), glm::u8vec4(0x10, 0x10, 0x18, 0xff));
	// 	lines.draw_text(hint, glm::vec3(-PixelCanvas::Aspect + 0.05f, -0.97f, 0.0f), glm::vec3(0.075f, 0.0f, 0.0f), glm::vec3(0.0f, 0.075f, 0.0f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	// }
	// canvas.end(drawable_size);

	static GLuint text_program = 0;
	if (text_program == 0) {
		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &text_vertex_shader, nullptr);
		glCompileShader(vs);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &text_fragment_shader, nullptr);
		glCompileShader(fs);
		text_program = glCreateProgram();
		glAttachShader(text_program, vs);
		glAttachShader(text_program, fs);
		glLinkProgram(text_program);
		glDeleteShader(vs);
		glDeleteShader(fs);
	}

	for (unsigned int i = 0; i < text_processor.curr_line.characters.size(); i++) {

		auto const &character = text_processor.curr_line.characters.at(i);
		if (character.pixels.empty()) continue; 

		// // For testing
		// printf("glyph index: %u\n", glyph_index);
		// printf("bitmap: %d x %d\n", character.width, character.rows);
		// printf("pitch: %d\n", slot->bitmap.pitch);
		// printf("pixel mode: %d\n", slot->bitmap.pixel_mode);
		// printf("bitmap_left: %d, bitmap_top: %d\n", slot->bitmap_left, slot->bitmap_top);

		// Built off of ideas in PPU466.cpp from project 1 
		// Create texture for glyph
		GLuint glyph_tex = 0;

		glGenTextures(1, &glyph_tex);
		glBindTexture(GL_TEXTURE_2D, glyph_tex);

		// Freetype's bitmap is one byte per pixel
		// Keep in mind: 1 alpha channel
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		//passing 'nullptr' to TexImage says "allocate memory but don't store anything there":
		// (textures will be uploaded later)
		// GL_RGBA8 to GL_R8 b/c 1 colour channel instead of 4
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, character.width, character.rows, 0, GL_RED, GL_UNSIGNED_BYTE, character.pixels.data());
		//make the texture have sharp pixels when magnified:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//when access past the edge, clamp to the edge:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		// vertex
		struct Vertex {
			glm::vec2 Position;
			glm::vec2 TexCoord;
		};

		
		// need to account for the bitmap starting points
		float pen_x_offset = character.x + character.bitmap_left;
		float pen_y_offset = character.y + character.bitmap_top - float(character.rows);

		// build rectangle representing background and sprites (of bitmap):
		std::vector<Vertex> vertices = {
			// triangle 1
			{glm::vec2(pen_x_offset, pen_y_offset), glm::vec2(0.0f, 1.0f)},
			{glm::vec2(pen_x_offset + character.width, pen_y_offset), glm::vec2(1.0f, 1.0f)},
			{glm::vec2(pen_x_offset + character.width, pen_y_offset + character.rows), glm::vec2(1.0f, 0.0f)},
			// triangle 2
			{glm::vec2(pen_x_offset, pen_y_offset), glm::vec2(0.0f, 1.0f)},
			{glm::vec2(pen_x_offset, pen_y_offset + character.rows), glm::vec2(0.0f, 0.0f)},
			{glm::vec2(pen_x_offset + character.width, pen_y_offset + character.rows), glm::vec2(1.0f, 0.0f)}
		};

		//vertex_buffer will (eventually) hold vertex data for drawing:
		GLuint vertex_buffer = 0;
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// vao
		GLuint vertex_array = 0;
		glGenVertexArrays(1, &vertex_array);
		glBindVertexArray(vertex_array);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		// attribute array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

		// attribute array for text coords
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		glUseProgram(text_program);

		// uniforms for shader programs
		glm::mat4 OBJECT_TO_CLIP = glm::mat4(
			glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f)
		);
		GLint object_to_clip = glGetUniformLocation(text_program, "OBJECT_TO_CLIP");
		glUniformMatrix4fv(object_to_clip, 1, GL_FALSE, glm::value_ptr(OBJECT_TO_CLIP));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glyph_tex);

		GLint glyph_texture = glGetUniformLocation(text_program, "glyphTexture");
		glUniform1i(glyph_texture, 0);

		glBindVertexArray(vertex_array);

		// enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteVertexArrays(1, &vertex_array);
		glDeleteBuffers(1, &vertex_buffer);
		glDeleteTextures(1, &glyph_tex);
	}

	GL_ERRORS();
}