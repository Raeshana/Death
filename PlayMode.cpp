#include "PlayMode.hpp"
#include "data_path.hpp"
#include "gl_compile_program.hpp"

#include <iostream>
#include <cmath>
#include <cstring>
#include "SoulCaptureMode.hpp"

#include "text-pipeline.hpp"

PlayMode::PlayMode(){
	// Create patients
	patients[0].name = "Abigail";
	patients[1].name = "Sherry";
	patients[2].name = "Keith";
	patients[3].name = "David";
	patients[4].name = "Twins";

	// Assign patient rooms
	for(int i = 0; i < 5; i++){
		patients[i].difficulty = i+1; // temp
		patients[i].room_num = i;
		patients[i].room = std::make_shared<RoomMode>(i);
	}
}

PlayMode::~PlayMode() {
}

void PlayMode::update(float elapsed) {
}