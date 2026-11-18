#pragma once

#include "Mode.hpp"
#include "Sprites.hpp"

#include <glm/glm.hpp>

#include <vector>
#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <deque>

// text pipeline
#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <iostream>
#include <string>

#include "DialogueMode.hpp"

// #define FONT_SIZE 36
// #define MARGIN (FONT_SIZE * 0.5)
#include <string>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	static constexpr float ScreenWidth = 176.0f;
	static constexpr float ScreenHeight = 160.0f;


	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, interact;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	//camera:
	Scene::Camera *camera = nullptr;

	// Initialize library and load font face
    FT_Library ft_library = nullptr; // handle to library
    FT_Face ft_face = nullptr;       // handle to face object

	// Create hb-ft font
    hb_font_t *hb_font = nullptr;

	// Story variables
	int current_line = 0;

	glm::vec2 player_at = glm::vec2(120.0f, 20.0f);
	float hover_time = 0.0f;

	std::vector<std::string> room;

	struct Interactable {
		std::string name;
		glm::vec2 pos;
		float radius;
	};
	std::vector<Interactable> interactables;
	int nearby = -1;

	bool tile_is_solid(int tx, int ty);
	bool can_move_to(glm::vec2 pos);
	void on_interact(Interactable const &thing);

	Sprites sprites;
	Sprites::Texture reaper_tex;
	Sprites::Texture floor_tex;
	Sprites::Texture patient1_tex;
	Sprites::Texture man1_tex;
	Sprites::Texture woman1_tex;
	Sprites::Texture partition_tex;
	Sprites::Texture cardiogram_tex;
	Sprites::Texture iv_tex;
	Sprites::Texture desk_tex;
	Sprites::Texture interact_tex;
	Sprites::Texture vignette_tex;
	Sprites::Texture light_tex;
	std::vector< glm::vec2 > lights;
	Sprites::Texture soul_tex;
	glm::vec2 soul_pos = glm::vec2(128.0f, 200.0f);
	Sprites::Texture patient_dead_tex;
	Sprites::Texture cardiogram_dead_tex;

	struct Prop {
		Sprites::Texture tex;
		glm::vec2 pos;
		char kind = '.';
	};
	std::vector<Prop> props;
	Sprites::Texture texture_for(Prop const &prop);
	bool soul_taking = false;

    bool diary_read = false;         
	bool soul_taken = false;
	std::shared_ptr< DialogueMode > active_dialogue;
	void start_minigame();

	void reset_buttons();

	PlayMode(int room_number);
	int room_number = 1;
	std::shared_ptr< Mode > return_to;

	float fade_out = 0.0f;
	std::shared_ptr< Mode > leaving_to;
	void start_leaving(std::shared_ptr< Mode > next);

	float bed_center_x = 128.0f;

	void draw_window();
};