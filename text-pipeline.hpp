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

#include <list>

#define FONT_SIZE 36
#define MARGIN (FONT_SIZE * 0.5)


struct TextProcessor {
    TextProcessor();
    ~TextProcessor();

    // Initialize library and load font face
    FT_Library ft_library; // handle to library
    FT_Face ft_face;       // handle to face object 
    FT_Error ft_error;     // handle to error

    // Create hb-font
    hb_font_t *hb_font = nullptr;

    struct Character {
        FT_Face ft_face;    // handle to face object 
        char character;     // sanity check (is the char the same?)
    };

    struct Line {
       
        std::list <Character> characters; 

        const float pen_x_start = 100.0f;
        const float pen_y_start = 100.0f;

        float pen_x = pen_x_start; // current x location
        float pen_y = pen_y_start; // current y location
        

    };

    Line curr_line;

    void handle_curr_line(std::string imported_line);

};

