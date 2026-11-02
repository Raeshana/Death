#include "DialogueMode.hpp"

#include <iostream>
#include <string>

// Referenced https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
// Referenced https://freetype.org/freetype2/docs/tutorial/step1.html
// Referenced PPU466.cpp from project 1 to render bitmap textures
// Referenced https://github.com/JBenda/inkcpp for inky-c++ integration [RAHHH!]

DialogueMode::DialogueMode() {
	// inky
	// ink::compiler::run("test.json", "test.bin");
	inkDialogue = ink::runtime::story::from_file(data_path("prototype_script.bin").c_str());
	thread = inkDialogue->new_runner();

    // can_battle = false;
}

DialogueMode::~DialogueMode() {
}

bool DialogueMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    // if (return_to) {
    //     auto self = shared_from_this();
    //     Mode::set_current(std::move(return_to));
    //     return true;
    // }

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_SPACE){ // process dialogue
            talk.pressed = true;

			curr_line = ""; // restart line
            
			// can the runner continue?
			// yes - get the next line
			// no - at choice or end
			if ( thread->can_continue() ){ 
				curr_line = thread->getline();
				if (thread->has_tags()){
					std::cout << "tag avalible: " << "\n" << std::endl;

					for (ink::size_t i = 0; i < thread->num_tags(); ++i){
						std::string tag = thread->get_tag(i);
						std::cout << "tag: " << tag << "\n" << std::endl;

                        // set can battle status to true
                        if (tag == "battle") {
                            can_battle = true;
							std::cout << "can battle: " << can_battle << std::endl;
                        }
					}
				}
				dialogue_processor.handle_curr_line(curr_line);
                return true;
			}
			else if (thread->has_choices()) { // display as 3 different choices
				// iterate choices
				if (thread->has_tags()){
					std::cout << "tag avalible: " << "\n" << std::endl;

					for (ink::size_t i = 0; i < thread->num_tags(); ++i){
						std::string tag = thread->get_tag(i);
						std::cout << "tag: " << tag << "\n" << std::endl;

                        // set can battle status to true
                        if (tag == "battle") {
                            can_battle = true;
							std::cout << "can battle: " << can_battle << std::endl;
                        }
					}
				}
				int id = 0;
				for ( const ink::runtime::choice c : *thread )
				{
					id++;
					curr_line.append(c.text());
				}
				dialogue_processor.handle_curr_line(curr_line);
                return true;
			}
			else { // at end
				// this will be moved under choices
				std::cout << "I AM IN THE ELSE\n" << can_battle << std::endl;
				if (can_battle) {
					Mode::set_current(std::move(return_to));
					std::cout << "I AM IN THE IF STATEMENT\n" << can_battle << std::endl;
				}
			}
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
        // use this for choice stuff instead of hard coding?
        if (evt.key.key ==SDLK_SPACE) {
            talk.pressed = false;
			return true;
        }
	    if (evt.key.key == SDLK_1) { // choice 1
			if(thread->has_choices()){
				thread->choose(0);
				curr_line = thread->getline();
				dialogue_processor.handle_curr_line(curr_line);
			}
            return true;
		} else if (evt.key.key == SDLK_2) { // choice 2
			if(thread->has_choices()){
				thread->choose(1);
				curr_line = thread->getline();
				dialogue_processor.handle_curr_line(curr_line);
			}
            return true;
		} else if (evt.key.key == SDLK_3) { // choice 3
			if(thread->has_choices()){
				thread->choose(2);
				curr_line = thread->getline();
				dialogue_processor.handle_curr_line(curr_line);
			}
            return true;
		}
	}
	return false;
};

void DialogueMode::update(float elapsed) {
	//reset button press counters:
	talk.downs = 0;
}

void DialogueMode::draw(glm::uvec2 const &drawable_size) {
	// draw textbox
	glm::mat4 OBJECT_TO_CLIP = glm::mat4(
		glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f) // bottom-left
	);

	DrawLines lines(OBJECT_TO_CLIP);

	lines.fill_rect(
		glm::vec2(dialogue_pen_x_start, dialogue_pen_y_start), // min
		glm::vec2(dialogue_pen_x_start + drawable_size.x - 2*MARGIN, 
					dialogue_pen_y_start + dialogue_textbox_height), // max
		glm::u8vec4(20, 20, 30, 100) // colour
	);

	// draw dialogue line
	dialogue_processor.textbox_width = (float)drawable_size.x - 4*MARGIN;
	dialogue_processor.textbox_height = dialogue_textbox_height - 2*MARGIN;
	dialogue_processor.pen_x_start = dialogue_pen_x_start + MARGIN;
	dialogue_processor.pen_y_start = dialogue_pen_y_start + dialogue_textbox_height - MARGIN;
	dialogue_processor.draw_curr_line(drawable_size);

	// draw current speaker
	speaker_processor.pen_x_start = dialogue_pen_x_start;
	speaker_processor.pen_y_start = dialogue_textbox_height + MARGIN;
	speaker_processor.textbox_width = FONT_SIZE*20;
	speaker_processor.textbox_height = FONT_SIZE*5;

	lines.fill_rect(
		glm::vec2(speaker_processor.pen_x_start, speaker_processor.pen_y_start), // min
		glm::vec2(speaker_processor.pen_x_start + speaker_processor.textbox_width, 
					speaker_processor.pen_x_start + speaker_processor.textbox_height), // max
		glm::u8vec4(0, 0, 0, 0) // colour
	);
	
	speaker_processor.handle_curr_line("SOMEBODY\n"); 
	speaker_processor.draw_curr_line(drawable_size);

	// draw hint (bottom-right)
	hint_processor.handle_curr_line("SPACE TO CONTINUE\n"); 
	hint_processor.textbox_width = FONT_SIZE*20;
	hint_processor.textbox_height = FONT_SIZE*5;
	hint_processor.pen_x_start = MARGIN + MARGIN/2;
	hint_processor.pen_y_start = MARGIN + MARGIN/2;
	hint_processor.draw_curr_line(drawable_size);
}