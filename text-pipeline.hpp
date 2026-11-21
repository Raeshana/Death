#pragma once
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

#include <vector>

#define FONT_SIZE 28
#define MARGIN (FONT_SIZE * 2)

struct Character {
    std::vector<unsigned char> pixels; // width*rows bytes, tightly packed
    unsigned int width = 0;
    unsigned int rows = 0;
    int bitmap_left = 0;
    int bitmap_top = 0;
    float x = 0.0f;   
    float y = 0.0f;
    char character;     // sanity check (is the char the same?)
};

struct Line {
    std::vector <Character> characters;
    float typing_elapsed = 0;
    double typing_max = 0.04;
    int char_num = 0;
    bool is_typing = false;

    GLuint tex = 0; // texture drawn to fbo
    GLuint fbo = 0; // framebuffer
};

struct TextProcessor {
    TextProcessor();
    ~TextProcessor();

    // Textbox sizes
    // don't want these to be fixed in case we want text to change
    // in different circumstances
    // e.g. dialogue vs. hints
    float textbox_width;
	float textbox_height;
    float pen_x_start;
    float pen_y_start;

    // drawing colour; defaults to white so existing callers are unaffected
    glm::vec3 color = glm::vec3(1.0f);

    // Freetype and Harbuff 
    FT_Library ft_library = nullptr; // handle to library
    FT_Face ft_face = nullptr;       // handle to face object 
    hb_font_t *hb_font = nullptr;   // created font

    Line curr_line; // stores the next Line to be printed

    // functions
    void handle_curr_line(std::string imported_line);
    void draw_curr_character(glm::uvec2 const &drawable_size);
    void draw_curr_line(float textbox_x_start, float textbox_y_start, float textbox_x_end, float textbox_y_end, glm::uvec2 const &drawable_size);
    void update(float elapsed);
    void draw_textbox(float textbox_x_start, float textbox_y_start, float textbox_x_end, float textbox_y_end, glm::uvec2 const &drawable_size);
    void reset_line(glm::uvec2 const &window_size);

    GLuint text_program = 0;
};



