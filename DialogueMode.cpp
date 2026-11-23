#include "DialogueMode.hpp"

#include <iostream>
#include <string>
#include <unordered_map>

// Referenced https://github.com/JBenda/inkcpp for inky-c++ integration [RAHHH!]


std::shared_ptr<InkStory> InkStory::get(std::string const &script_file) {
	static std::unordered_map< std::string, std::shared_ptr<InkStory> > stories;

	auto found = stories.find(script_file);
	if (found != stories.end()) return found->second;

	auto s = std::make_shared<InkStory>();
	s->story = ink::runtime::story::from_file(data_path(script_file).c_str());
	s->globals = s->story->new_globals();
	s->runner = s->story->new_runner(s->globals); // also initializes the globals
	stories.emplace(script_file, s);
	return s;
}

// strip leading/trailing whitespace
static std::string trim(std::string const &s) {
	size_t begin = s.find_first_not_of(" \t\r\n");
	if (begin == std::string::npos) return "";
	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(begin, end - begin + 1);
}

DialogueMode::DialogueMode(std::string const &script_file) {
	ink = InkStory::get(script_file);
	ink->paused = false;

	auto &thread = ink->runner;
	if (!thread->can_continue() && !thread->has_choices()) {
		if (!thread->move_to(ink->next_patient.c_str())) {
			std::cerr << "DialogueMode: resume knot '" << ink->next_patient << "' not found in " << script_file << std::endl;
		}
	}
}

DialogueMode::~DialogueMode() {
}

//parse the tags and sets the variables for control flow. 
void DialogueMode::process_tags() {
	auto &thread = ink->runner;
	for (ink::size_t i = 0; i < thread->num_tags(); ++i) {
		std::string tag = trim(thread->get_tag(i));
		std::cout << "tag: " << tag << std::endl;

		if (tag == "battle") {
			can_battle = true;
		} else if (tag == "pause") {
			pause_pending = true;
		}
	}
}

void DialogueMode::leave() {
	auto self = shared_from_this();
	std::shared_ptr<Mode> back = return_to;
	Mode::set_current(back);
}

void DialogueMode::choose(int i) {
	auto &thread = ink->runner;
	if (!thread->has_choices() || i >= int(thread->num_choices())) return;

	thread->choose(i);
	curr_line = thread->can_continue() ? thread->getline() : "";
	process_tags();
	dialogue_processor.handle_curr_line(curr_line);
}

bool DialogueMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	auto &thread = ink->runner;

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_SPACE){ // process dialogue
			talk.pressed = true;

			if (pause_pending) {
				pause_pending = false;
				ink->paused = true;
				leave(); //close the dialogue
				return true;
			}

			curr_line = ""; // restart line
			dialogue_processor.reset_line(window_size);

			// can the runner continue?
			// yes - get the next line
			// no - at choice or end
			if ( thread->can_continue() ){
				curr_line = thread->getline();
				process_tags();

				// nothing to show: either a pause tag with no text, or the story
				// close right away when hit ->DONE or ->END
				bool stopped = !thread->can_continue() && !thread->has_choices();
				if (trim(curr_line).empty() && (pause_pending || stopped)) {
					pause_pending = false;
					ink->paused = true;
					leave();
					return true;
				}

				//if there is an empty line before choice  show the choices instead of a blank box
				if (!(trim(curr_line).empty() && thread->has_choices())) {
					dialogue_processor.handle_curr_line(curr_line);
					return true;
				}
			}

			if (thread->has_choices()) {
				process_tags();
				for ( const ink::runtime::choice c : *thread ) {
					curr_line.append(c.text());
				}
				dialogue_processor.handle_curr_line(curr_line);
				return true;
			}
			else {
				leave();
				return true;
			}
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_SPACE) {
			talk.pressed = false;
			return true;
		}
		if (evt.key.key == SDLK_1) { 
			choose(0);
			return true;
		}
		if (evt.key.key == SDLK_2) { 
			choose(1);
			return true; 
		}
		if (evt.key.key == SDLK_3) { 
			choose(2);
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
	float textbox_x_start = MARGIN;
	float textbox_y_start = MARGIN;
	float textbox_x_end = textbox_x_start + drawable_size.x - 2*MARGIN;
	float textbox_y_end = textbox_y_start + drawable_size.y/4;

	// // speaker dimensions
	// speaker_processor.textbox_width = 10*FONT_SIZE;
	// speaker_processor.textbox_height = 2*FONT_SIZE;
	// speaker_processor.pen_x_start = textbox_x_start;
	// speaker_processor.pen_y_start = textbox_y_end;

	dialogue_processor.textbox_width = textbox_x_end - textbox_x_start;
	dialogue_processor.textbox_height = textbox_y_end - textbox_y_start;
	dialogue_processor.pen_x_start = textbox_x_start + MARGIN;
	dialogue_processor.pen_y_start = textbox_y_end - MARGIN;
	dialogue_processor.draw_curr_line(textbox_x_start, textbox_y_start, textbox_x_end, textbox_y_end, drawable_size);
	dialogue_processor.curr_line.is_typing = true;

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