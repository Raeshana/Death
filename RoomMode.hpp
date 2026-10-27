#pragma once

#include "PPU466.hpp"
#include "Mode.hpp"
#include "PixelCanvas.hpp"
#include "SoulCaptureMode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <string>

// text pipeline
#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <iostream>
#include <string>

#include <globals.h>
#include <story.h>
#include <runner.h>
#include <choice.h>
#include <memory.h>
#include <compiler.h>

#include "text-pipeline.hpp"

// Referenced https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
// Referenced https://freetype.org/freetype2/docs/tutorial/step1.html
// Referenced PPU466.cpp from project 1 to render bitmap textures
// Referenced https://github.com/JBenda/inkcpp for inky-c++ integration [RAHHH!]

struct RoomMode : Mode {
	RoomMode();
	virtual ~RoomMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, up, down, interact;

	glm::vec2 player_at = glm::vec2(128.0f, 60.0f);

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

	PPU466 ppu;

	float hover_time = 0.0f;

	PixelCanvas canvas;
	std::string hint = "WASD+E. Read the diary first, or just reap the patient.";
	bool read_diary = false;
	std::shared_ptr< SoulCaptureMode > capture;

	//----- font stuff -----

	TextProcessor text_processor;

	//----- story stuff -----

	// Story variables
	// int current_line = 0; // hard-coded

	// Load ink binary story, generated from the inkCPP compiler
	ink::runtime::story* myInk;

	// Create a new thread
	ink::runtime::runner thread;

	std::string curr_line;
};