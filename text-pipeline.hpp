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

#define FONT_SIZE 36
#define MARGIN (FONT_SIZE * 0.5)


struct TextProcessor {
    TextProcessor();
    ~TextProcessor();

    // Initialize library and load font face
    FT_Library ft_library = nullptr; // handle to library
    FT_Face ft_face = nullptr;       // handle to face object 
    // Create hb-font
    hb_font_t *hb_font = nullptr;
    bool pause = false; 

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

        

    };

    Line curr_line;

    void handle_curr_line(std::string imported_line);

};

