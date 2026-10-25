#include "data_path.hpp"

// font things
#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>
#include <globals.h>
#include <story.h>
#include <runner.h>
#include <choice.h>
#include <memory.h>
#include <compiler.h>

#include <string>
#include <list>
#include <iostream>

// rendering
#include "LitColorTextureProgram.hpp"
#include <glm/glm.hpp>
#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "gl_compile_program.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "text-pipeline.hpp"

#define FONT_SIZE 36
#define MARGIN (FONT_SIZE * 0.5)

// Referenced https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
// Referenced https://freetype.org/freetype2/docs/tutorial/step1.html

// int load_library() {
// 	// Abort if files not found/ not working
//     if (FT_Init_FreeType( &ft_library )) {
//         fprintf (stderr, "Could not find library\n");
//         abort();
//     }

// 	const char *fontfile = "C:\\Windows\\Fonts\\Arial.ttf";
//     if (FT_New_Face(ft_library, fontfile, 0, &ft_face)) {
//         fprintf (stderr, "Font file could be opened and read, but font format is unsupported\n");
//         abort();
//     }

//     if ((FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0))){   
//         fprintf (stderr, "Font size could not be set\n");
//         abort();
//     }

// 	// Create hb-ft font
//     hb_font = hb_ft_font_create(ft_face, NULL);
// 	std::cout << "Font created\n";
// }

int run_text_pipeline() {

    
}

// Create hb-buffer and populate
void TextProcessor::handle_curr_line(std::string imported_line) {
    hb_buffer_t *hb_buffer = hb_buffer_create();
    hb_buffer_add_utf8 (hb_buffer, imported_line.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties (hb_buffer);

    // Shape text
    hb_shape(hb_font, hb_buffer, NULL, 0);

    // Get glyph info and pos out of the buffer
    unsigned int num_chars = hb_buffer_get_length (hb_buffer);
    hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);

    float pen_x = 100.0f;
    float pen_y = 100.0f;

    for (unsigned int i = 0; i < num_chars; i++) {
        Character character;

        FT_UInt glyph_index = info[i].codepoint;
        
        // Load glyph image into the slot (erase prev)
        if (FT_Load_Glyph (character.ft_face, glyph_index, FT_LOAD_DEFAULT)) {
            fprintf (stderr, "Error loading glyph image into slot\n");
            abort();
        }

        // Convert to an anti-aliased bitmap
        if (FT_Render_Glyph( character.ft_face->glyph, FT_RENDER_MODE_NORMAL)) {
            fprintf (stderr, "Error converting glyph to an anti-aliased bitmap\n");
            abort();
        }

        // Get the character
        character.character = imported_line.at(i);

        // Append to the characters list (line)
        curr_line.characters.emplace_back(character);
    }
}

