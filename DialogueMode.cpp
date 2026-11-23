#include "DialogueMode.hpp"

#include <iostream>
#include <string>

#include <cmath>
#include <algorithm>

// Referenced https://github.com/JBenda/inkcpp for inky-c++ integration [RAHHH!]


DialogueMode::DialogueMode(std::string const &script_file) {
	inkDialogue = ink::runtime::story::from_file(data_path(script_file).c_str());
	thread = inkDialogue->new_runner();

	can_battle = false;
}

DialogueMode::~DialogueMode() {
}

bool DialogueMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_SPACE){ // process dialogue
            talk.pressed = true;

			curr_line = ""; // restart line
			dialogue_processor.reset_line(last_drawable_size);
            
			// can the runner continue?
			// yes - get the next line
			// no - at choice or end
			if ( thread->can_continue() ){ 
				curr_line = thread->getline();
				// check for tags
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
				// std::cout << "dialogue mode: " << dialogue_processor.curr_line.characters.empty() << std::endl;
                return true;
			}
			else if (thread->has_choices()) { // display as 3 different choices
				// check for tags
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
				// iterate choices
				int id = 0;
				(void)id;
				for ( const ink::runtime::choice c : *thread )
				{
					id++;
					curr_line.append(c.text());
				}
				dialogue_processor.handle_curr_line(curr_line);
                return true;
			}
			else {
				auto self = shared_from_this();
				std::shared_ptr< Mode > back = return_to;
				Mode::set_current(back);
				return true;
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

	// keep ghost floating
	return_to->update(elapsed);

	// timer for text effect
	dialogue_processor.curr_line.typing_elapsed += elapsed;
	// std::cout << "dialogue_processor.curr_line.typing_elapsed = " << dialogue_processor.curr_line.typing_elapsed << std::endl;
		
}

void DialogueMode::draw(glm::uvec2 const &drawable_size) {	
	// background --------------------------------------------------------------------------------------------
	if (!can_battle) return_to->draw(drawable_size);

	// textbox dimensions
	last_drawable_size = drawable_size;

	float scale = std::floor(std::min(drawable_size.x / 256.0f, drawable_size.y / 240.0f));
	if (scale < 1.0f) {
		scale = 1.0f;
	}
	float view_x = std::floor((drawable_size.x - 256.0f * scale) * 0.5f);
	float view_y = std::floor((drawable_size.y - 240.0f * scale) * 0.5f);

	unsigned int font_pixels = (unsigned int)(9.0f * scale);
	if (font_pixels != dialogue_font_pixels) {
		FT_Set_Pixel_Sizes(dialogue_processor.ft_face, 0, font_pixels);
		hb_ft_font_changed(dialogue_processor.hb_font);
		dialogue_font_pixels = font_pixels;
	}

	float textbox_x_start = view_x + 8.0f * scale;
	float textbox_y_start = view_y + 8.0f * scale;
	float textbox_x_end = view_x + (256.0f - 8.0f) * scale;
	float textbox_y_end = view_y + 64.0f * scale;

	// // speaker dimensions
	// speaker_processor.textbox_width = 10*FONT_SIZE;
	// speaker_processor.textbox_height = 2*FONT_SIZE;
	// speaker_processor.pen_x_start = textbox_x_start;
	// speaker_processor.pen_y_start = textbox_y_end;

	dialogue_processor.textbox_width = (textbox_x_end - textbox_x_start) - 16.0f * scale;
	dialogue_processor.textbox_height = textbox_y_end - textbox_y_start;
	dialogue_processor.pen_x_start = textbox_x_start + 8.0f * scale;
	dialogue_processor.pen_y_start = textbox_y_end - 14.0f * scale;
	dialogue_processor.curr_line.is_typing = true;
	dialogue_processor.draw_curr_line(textbox_x_start, textbox_y_start, textbox_x_end, textbox_y_end, drawable_size);

	// // dialogue --------------------------------------------------------------------------------------------
	// float textbox_x_start = MARGIN;
	// float textbox_y_start = MARGIN;
	// float textbox_x_end = textbox_x_start + drawable_size.x - 2*MARGIN;
	// float textbox_y_end = textbox_y_start + drawable_size.y/4;
	// {
	// 	glm::mat4 OBJECT_TO_CLIP = glm::mat4(
	// 		glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f), // x
	// 		glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f), // y
	// 		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), // z
	// 		glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f) // bottom-left
	// 	);

	// 	DrawLines lines(OBJECT_TO_CLIP);

	// 	lines.fill_rect(
	// 		glm::vec2(textbox_x_start, textbox_y_start), // min
	// 		glm::vec2(textbox_x_end, textbox_y_end), // max
	// 		glm::u8vec4(20, 20, 30, 255) // colour
	// 	);

	// 	float line_thickness = 5.0f;

	// 	// bottom line
	// 	lines.fill_line( 
	// 		glm::vec2(textbox_x_start, textbox_y_start), // point a
	// 		glm::vec2(textbox_x_end, textbox_y_start), // point b
	// 		line_thickness, // width
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);

	// 	// top line
	// 	lines.fill_line( 
	// 		glm::vec2(textbox_x_start, textbox_y_end), // point a
	// 		glm::vec2(textbox_x_end, textbox_y_end), // point b
	// 		line_thickness, // width
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);

	// 	// left line
	// 	lines.fill_line( 
	// 		glm::vec2(textbox_x_start, textbox_y_start), // point a
	// 		glm::vec2(textbox_x_start, textbox_y_end), // point b
	// 		line_thickness, // width
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);

	// 	// right line
	// 	lines.fill_line( 
	// 		glm::vec2(textbox_x_end, textbox_y_start), // point a
	// 		glm::vec2(textbox_x_end, textbox_y_end), // point b
	// 		line_thickness, // width
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);
	// }

	// // we can put all of this into one texture :3

	// // draw dialogue line
	// dialogue_processor.textbox_width = textbox_x_end - textbox_x_start;
	// dialogue_processor.textbox_height = textbox_y_end - textbox_y_start;
	// dialogue_processor.pen_x_start = textbox_x_start + MARGIN;
	// dialogue_processor.pen_y_start = textbox_y_end - MARGIN;
	// dialogue_processor.draw_curr_line(drawable_size);

	// // draw current speaker
	// speaker_processor.textbox_width = 10*FONT_SIZE;
	// speaker_processor.textbox_height = 2*FONT_SIZE;
	// speaker_processor.pen_x_start = textbox_x_start;
	// speaker_processor.pen_y_start = textbox_y_end;

	// {
	// 	glm::mat4 OBJECT_TO_CLIP = glm::mat4(
	// 		glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f),
	// 		glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f),
	// 		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
	// 		glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f) // bottom-left
	// 	);

	// 	DrawLines lines(OBJECT_TO_CLIP);

	// 	float speaker_x_end = speaker_processor.pen_x_start + speaker_processor.textbox_width;
	// 	float speaker_y_end = speaker_processor.pen_y_start + speaker_processor.textbox_height;

	// 	lines.fill_rect(
	// 		glm::vec2(speaker_processor.pen_x_start, speaker_processor.pen_y_start), // min
	// 		glm::vec2(speaker_x_end, speaker_y_end), // max
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);

	// 	lines.fill_triangle(
	// 		glm::vec3(speaker_x_end, speaker_processor.pen_y_start, 0), // point a
	// 		glm::vec3(speaker_x_end + 50.0f, speaker_processor.pen_y_start, 0), // point b
	// 		glm::vec3(speaker_x_end, speaker_y_end, 0), // point c
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);
	// }
	
	// speaker_processor.pen_x_start = textbox_x_start + MARGIN/2;
	// speaker_processor.pen_y_start = textbox_y_end + MARGIN/4;
	// speaker_processor.handle_curr_line("SOMEBODY\n"); 
	// speaker_processor.draw_curr_line(drawable_size);

	// // draw hint (bottom-right)
	// hint_processor.textbox_width = 10*FONT_SIZE;
	// hint_processor.textbox_height = FONT_SIZE;
	// hint_processor.pen_x_start = textbox_x_end - hint_processor.textbox_width;
	// hint_processor.pen_y_start = textbox_y_start + hint_processor.textbox_height;
	// hint_processor.handle_curr_line("SPACE TO CONTINUE\n"); 
	// hint_processor.draw_curr_line(drawable_size);
}