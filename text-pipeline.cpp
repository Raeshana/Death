#include "data_path.hpp"

// font things
#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>
#include <algorithm>
#include <globals.h>
#include <story.h>
#include <runner.h>
#include <choice.h>
#include <memory.h>
#include <compiler.h>

#include <string>
#include <vector>
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

TextProcessor::TextProcessor(){
    // Abort if files not found/ not working
    if (FT_Init_FreeType( &ft_library )) {
        fprintf (stderr, "Could not find library\n");
        abort();
    }

    // https://www.1001fonts.com/lower-pixel-font.html
    // credit to IrbaCreative
	const char *fontfile = "ByteBounce.ttf";
    if (FT_New_Face(ft_library, fontfile, 0, &ft_face)) {
        fprintf (stderr, "Font file could be opened and read, but font format is unsupported\n");
        abort();
    }

    if ((FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0))){   
        fprintf (stderr, "Font size could not be set\n");
        abort();
    }

	// Create hb-ft font
    hb_font = hb_ft_font_create(ft_face, NULL);
	std::cout << "Font created\n";

}
TextProcessor::~TextProcessor(){
    if (hb_font) hb_font_destroy(hb_font);
    if (ft_face) FT_Done_Face(ft_face);
    if (ft_library) FT_Done_FreeType(ft_library);

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
    curr_line.characters.clear();
    float pen_x = 100.0f;
    float pen_y = 100.0f;




    for (unsigned int i = 0; i < num_chars; i++) { 
        // ignoring the null character
        // if i make i < num_chars-1 in the loopguard it crashes on comments in the ink file. 
        
        if (i == num_chars-1) continue;

        Character character;
        if (i == 0){
            // if guard for parsing 
        }

        FT_UInt glyph_index = info[i].codepoint;
        
        // Load glyph image into the slot (erase prev)
        if (FT_Load_Glyph (ft_face, glyph_index, FT_LOAD_DEFAULT)) {
            fprintf (stderr, "Error loading glyph image into slot\n");
            abort();
        }

        // Convert to an anti-aliased bitmap
        if (FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL)) {
            fprintf (stderr, "Error converting glyph to an anti-aliased bitmap\n");
            abort();
        }
        FT_GlyphSlot slot = ft_face->glyph;

        // Get the character
        character.character = imported_line.at(i);
        character.width = slot->bitmap.width;
        character.rows = slot->bitmap.rows;
        character.bitmap_left = slot->bitmap_left;
        character.bitmap_top = slot->bitmap_top;
        character.x = pen_x + pos[i].x_offset/64.0f;
        character.y = pen_y + pos[i].y_offset/64.0f;
        character.pixels.resize(character.width*character.rows);
        
        for (unsigned int row = 0; row < character.rows; row++) {
            
            const unsigned char* src = slot->bitmap.buffer + row*slot->bitmap.pitch;
            std::copy(src, src + character.width, character.pixels.begin() + row*character.width);
        }

        curr_line.characters.emplace_back(std::move(character));

        pen_x += pos[i].x_advance/64.0f;
        pen_y += pos[i].y_advance/64.0f;
        
    }
    hb_buffer_destroy(hb_buffer);
}

