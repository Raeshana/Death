#pragma once
#include "Mode.hpp"


#include "Scene.hpp"
#include "Sound.hpp"
#include "data_path.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

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

#define FONT_SIZE 36
#define MARGIN (FONT_SIZE * 0.5)

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	//camera:
	Scene::Camera *camera = nullptr;

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
