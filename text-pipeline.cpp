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

// Referenced https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
// Referenced https://freetype.org/freetype2/docs/tutorial/step1.html
// Referenced PPU466.cpp from project 1 to render bitmap textures

TextProcessor::TextProcessor(){
    // Abort if files not found/ not working
    if (FT_Init_FreeType( &ft_library )) {
        fprintf (stderr, "Could not find library\n");
        abort();
    }

    // https://www.1001fonts.com/lower-pixel-font.html
    // credit to IrbaCreative
	const char *fontfile = "dist/LowresPixel-Regular.otf";
    FT_Error error = FT_New_Face(ft_library, fontfile, 0, &ft_face);
    
    if (error) {
        std::cerr << "FT_New_Face failed: " << error
                << "\nFont path: " << fontfile << '\n';
        abort();
    }

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

/// @brief Takes an imported line from ink.bin file
///         And converts it into a Line
/// @param imported_line String from ink.bin
void TextProcessor::handle_curr_line(std::string imported_line) {
    // Create hb-buffer and populate
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
    float pen_x = pen_x_start;
    float pen_y = pen_y_start;

	float line_height = ft_face->size->metrics.height / 64.0f;
    float right_edge = pen_x_start + textbox_width;
    size_t word_start = 0;

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

		// really bad text wrapping attempt
        if (character.x >= pen_x + textbox_width) {
            pen_x = pen_x_start;
            pen_y = pen_y_start - (pen_y_start/2);
        }

        character.pixels.resize(character.width*character.rows);
        
        for (unsigned int row = 0; row < character.rows; row++) {
            const unsigned char* src = slot->bitmap.buffer + row*slot->bitmap.pitch;
            std::copy(src, src + character.width, character.pixels.begin() + row*character.width);
        }

        curr_line.characters.emplace_back(std::move(character));

        pen_x += pos[i].x_advance/64.0f;
        pen_y += pos[i].y_advance/64.0f;

		//word wrap
        Character const &added = curr_line.characters.back();
        if (added.character == ' ') {
            word_start = curr_line.characters.size();
        } else if (pen_x > right_edge
                   && word_start < curr_line.characters.size()
                   && curr_line.characters[word_start].x > pen_x_start + 1.0f) {
            float shift_x = pen_x_start - curr_line.characters[word_start].x;
            for (size_t c = word_start; c < curr_line.characters.size(); c++) {
                curr_line.characters[c].x += shift_x;
                curr_line.characters[c].y -= line_height;
            }
            pen_x += shift_x;
            pen_y -= line_height;
        }
    }
    hb_buffer_destroy(hb_buffer);
}

void TextProcessor::draw_textbox(float textbox_x_start, float textbox_y_start, float textbox_x_end, float textbox_y_end, glm::uvec2 const &drawable_size){
	{
		glm::mat4 OBJECT_TO_CLIP = glm::mat4(
			glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f), // x
			glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f), // y
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), // z
			glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f) // bottom-left
		);

		DrawLines lines(OBJECT_TO_CLIP);

		lines.fill_rect(
			glm::vec2(textbox_x_start, textbox_y_start), // min
			glm::vec2(textbox_x_end, textbox_y_end), // max
			glm::u8vec4(20, 20, 30, 255) // colour
		);

		float line_thickness = 5.0f;

		// bottom line
		lines.fill_line( 
			glm::vec2(textbox_x_start, textbox_y_start), // point a
			glm::vec2(textbox_x_end, textbox_y_start), // point b
			line_thickness, // width
			glm::u8vec4(0, 0, 0, 255) // colour
		);

		// top line
		lines.fill_line( 
			glm::vec2(textbox_x_start, textbox_y_end), // point a
			glm::vec2(textbox_x_end, textbox_y_end), // point b
			line_thickness, // width
			glm::u8vec4(0, 0, 0, 255) // colour
		);

		// left line
		lines.fill_line( 
			glm::vec2(textbox_x_start, textbox_y_start), // point a
			glm::vec2(textbox_x_start, textbox_y_end), // point b
			line_thickness, // width
			glm::u8vec4(0, 0, 0, 255) // colour
		);

		// right line
		lines.fill_line( 
			glm::vec2(textbox_x_end, textbox_y_start), // point a
			glm::vec2(textbox_x_end, textbox_y_end), // point b
			line_thickness, // width
			glm::u8vec4(0, 0, 0, 255) // colour
		);

		// lines.fill_rect(
		// 	glm::vec2(textbox_x_start, textbox_y_start), // min
		// 	glm::vec2(textbox_x_end, textbox_y_end), // max
		// 	glm::u8vec4(0, 0, 0, 255) // colour
		// );

		// lines.fill_triangle(
		// 	glm::vec3(speaker_x_end, speaker_processor.pen_y_start, 0), // point a
		// 	glm::vec3(speaker_x_end + 50.0f, speaker_processor.pen_y_start, 0), // point b
		// 	glm::vec3(speaker_x_end, speaker_y_end, 0), // point c
		// 	glm::u8vec4(0, 0, 0, 255) // colour
		// );
	}
}

void TextProcessor::reset_line(glm::uvec2 const &window_size){
	//free the previous line's texture and framebuffer
	if (curr_line.tex != 0) { glDeleteTextures(1, &curr_line.tex); curr_line.tex = 0; }
	if (curr_line.fbo != 0) { glDeleteFramebuffers(1, &curr_line.fbo); curr_line.fbo = 0; }
	
	// Line texture
	glGenTextures(1, &curr_line.tex);
	glBindTexture(GL_TEXTURE_2D, curr_line.tex);

	// Freetype's bitmap is one byte per pixel
	// Keep in mind: 1 alpha channel
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//passing 'nullptr' to TexImage says "allocate memory but don't store anything there":
	// (textures will be uploaded later)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, window_size.x, window_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	//make the texture have sharp pixels when magnified:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//when access past the edge, clamp to the edge:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// other Line vars
	curr_line.is_typing = false;
	curr_line.char_num = 0;
	curr_line.typing_elapsed = 0;

	// Line framebuffer
	// Ref https://learnopengl.com/Advanced-OpenGL/Framebuffers
	glGenFramebuffers(1, &curr_line.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, curr_line.fbo);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		curr_line.tex,
		0
	);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    	std::cout << "Framebuffer is not complete!" << std::endl;
	}

	// clear texture
	glViewport(0, 0, window_size.x, window_size.y);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// @brief Create text typing effect
/// @param elapsed 
void TextProcessor::draw_curr_line(float textbox_x_start, float textbox_y_start, float textbox_x_end, float textbox_y_end, glm::uvec2 const &drawable_size){
	glViewport(0, 0, drawable_size.x, drawable_size.y);

	if (curr_line.typing_elapsed > curr_line.typing_max &&
		!curr_line.characters.empty() &&
		curr_line.char_num < curr_line.characters.size()) {

			// fbo
			glBindFramebuffer(GL_FRAMEBUFFER, curr_line.fbo);

			// if first character, then draw textbox
			if (curr_line.char_num == 0) {
				draw_textbox(textbox_x_start, textbox_y_start, textbox_x_end, textbox_y_end, drawable_size);
			}
			draw_curr_character(drawable_size);
			curr_line.char_num++; // increment char num
			curr_line.typing_elapsed = 0; // reset timer

			// unbind fbo/ set drwaing back to screen
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// draw the line to the screen
	// code adapted from PixelCanvas commit dba4374 (Deming)
	if (curr_line.tex == 0) return;

	//one shader and one quad buffer shared by all text, made on first use:
	static GLuint program = 0, vertex_array = 0, vertex_buffer = 0;
	static GLint to_clip_loc = -1, image_loc = -1;
	if (program == 0) {
		program = gl_compile_program(
			"#version 330\n"
			"uniform mat4 TO_CLIP;\n"
			"in vec2 Position;\n"
			"in vec2 TexCoord;\n"
			"out vec2 texCoord;\n"
			"void main() {\n"
			"	gl_Position = TO_CLIP * vec4(Position, 0.0, 1.0);\n"
			"	texCoord = TexCoord;\n"
			"}\n",
			"#version 330\n"
			"uniform sampler2D IMAGE;\n"
			"in vec2 texCoord;\n"
			"out vec4 fragColor;\n"
			"void main() {\n"
			"	fragColor = texture(IMAGE, texCoord);\n"
			"}\n"
		);
		to_clip_loc = glGetUniformLocation(program, "TO_CLIP");
		image_loc = glGetUniformLocation(program, "IMAGE");
		GLint position_loc = glGetAttribLocation(program, "Position");
		GLint texcoord_loc = glGetAttribLocation(program, "TexCoord");

		glGenBuffers(1, &vertex_buffer);
		glGenVertexArrays(1, &vertex_array);
		glBindVertexArray(vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLbyte *)0);
		glEnableVertexAttribArray(position_loc);
		glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLbyte *)0 + 2 * sizeof(float));
		glEnableVertexAttribArray(texcoord_loc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	float const quad[6][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ float(drawable_size.x), 0.0f, 1.0f, 0.0f },
		{ float(drawable_size.x), float(drawable_size.y), 1.0f, 1.0f },

		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ float(drawable_size.x), float(drawable_size.y), 1.0f, 1.0f },
		{ 0.0f, float(drawable_size.y), 0.0f, 1.0f },
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//drawable pixels, (0,0) at the lower left, to clip space:
	glm::mat4 to_clip = glm::mat4(
		2.0f / drawable_size.x, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / drawable_size.y, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 1.0f
	);

	glUseProgram(program);
	glUniformMatrix4fv(to_clip_loc, 1, GL_FALSE, glm::value_ptr(to_clip));
	glUniform1i(image_loc, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, curr_line.tex);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(vertex_array);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	GL_ERRORS();
}

void TextProcessor::draw_curr_character(glm::uvec2 const &drawable_size){
    //set up screen scaling (ref. from Sprites.cpp)
	// float scale_x = drawable_size.x / ScreenWidth;
	// float scale_y = drawable_size.y / ScreenHeight;
	// float scale = scale_x;
	// if (scale_y < scale) {
	// 	scale = scale_y;
	// }
	// scale = std::floor(scale);
	// if (scale < 1.0f) {
	// 	scale = 1.0f;
	// }
	// int view_w = int(ScreenWidth * scale);
	// int view_h = int(ScreenHeight * scale);
	// int view_x = (int(drawable_size.x) - view_w) / 2;
	// int view_y = (int(drawable_size.y) - view_h) / 2;
	// glViewport(view_x, view_y, view_w, view_h);

    // text vertex shader
    const char *text_vertex_shader = R"(
        #version 330

        layout(location = 0) in vec2 Position;
        layout(location = 1) in vec2 TexCoord;

        out vec2 texCoord;

        uniform mat4 OBJECT_TO_CLIP;

        void main() {
            gl_Position = OBJECT_TO_CLIP * vec4(Position, 0.0, 1.0);
            texCoord = TexCoord;
        }
    )";

    // text fragment shader
    const char *text_fragment_shader = R"(
        #version 330

        in vec2 texCoord;

        uniform sampler2D glyphTexture;
		uniform vec3 textColor;

        out vec4 fragColor;

        void main() {
            float alpha = texture(glyphTexture, texCoord).r;
            fragColor = vec4(textColor, alpha);
        }
    )";

	if (text_program == 0) {
		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &text_vertex_shader, nullptr);
		glCompileShader(vs);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &text_fragment_shader, nullptr);
		glCompileShader(fs);
		text_program = glCreateProgram();
		glAttachShader(text_program, vs);
		glAttachShader(text_program, fs);
		glLinkProgram(text_program);
		glDeleteShader(vs);
		glDeleteShader(fs);
	}

	// get texture for 1 char -----------------------------------------------------------------------

	auto const &character = curr_line.characters.at(curr_line.char_num);
	if (character.pixels.empty()) return;

	// // For testing
	// printf("glyph index: %u\n", glyph_index);
	// printf("bitmap: %d x %d\n", character.width, character.rows);
	// printf("pitch: %d\n", slot->bitmap.pitch);
	// printf("pixel mode: %d\n", slot->bitmap.pixel_mode);
	// printf("bitmap_left: %d, bitmap_top: %d\n", slot->bitmap_left, slot->bitmap_top);

	// Built off of ideas in PPU466.cpp from project 1 
	// Create texture for glyph
	GLuint glyph_tex = 0;

	glGenTextures(1, &glyph_tex);
	glBindTexture(GL_TEXTURE_2D, glyph_tex);

	// Freetype's bitmap is one byte per pixel
	// Keep in mind: 1 alpha channel
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//passing 'nullptr' to TexImage says "allocate memory but don't store anything there":
	// (textures will be uploaded later)
	// GL_RGBA8 to GL_R8 b/c 1 colour channel instead of 4
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, character.width, character.rows, 0, GL_RED, GL_UNSIGNED_BYTE, character.pixels.data());
	//make the texture have sharp pixels when magnified:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//when access past the edge, clamp to the edge:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// vertex
	struct Vertex {
		glm::vec2 Position;
		glm::vec2 TexCoord;
	};

	// need to account for the bitmap starting points
	float pen_x_offset = character.x + character.bitmap_left;
	float pen_y_offset = character.y + character.bitmap_top - float(character.rows);

	// build rectangle representing background and sprites (of bitmap):
	std::vector<Vertex> vertices = {
		// triangle 1
		{glm::vec2(pen_x_offset, pen_y_offset), glm::vec2(0.0f, 1.0f)},
		{glm::vec2(pen_x_offset + character.width, pen_y_offset), glm::vec2(1.0f, 1.0f)},
		{glm::vec2(pen_x_offset + character.width, pen_y_offset + character.rows), glm::vec2(1.0f, 0.0f)},
		// triangle 2
		{glm::vec2(pen_x_offset, pen_y_offset), glm::vec2(0.0f, 1.0f)},
		{glm::vec2(pen_x_offset, pen_y_offset + character.rows), glm::vec2(0.0f, 0.0f)},
		{glm::vec2(pen_x_offset + character.width, pen_y_offset + character.rows), glm::vec2(1.0f, 0.0f)}
	};

	//vertex_buffer will (eventually) hold vertex data for drawing:
	GLuint vertex_buffer = 0;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// vao
	GLuint vertex_array = 0;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// attribute array for position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

	// attribute array for text coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(text_program);

	// make text white
	GLint text_color = glGetUniformLocation(text_program, "textColor");
	glUniform3f(text_color, 1.0f, 1.0f, 1.0f);

	// uniforms for shader programs
	glm::mat4 OBJECT_TO_CLIP = glm::mat4(
		glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f)
	);
	GLint object_to_clip = glGetUniformLocation(text_program, "OBJECT_TO_CLIP");
	glUniformMatrix4fv(object_to_clip, 1, GL_FALSE, glm::value_ptr(OBJECT_TO_CLIP));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glyph_tex);

	GLint glyph_texture = glGetUniformLocation(text_program, "glyphTexture");
	glUniform1i(glyph_texture, 0);

	// actual drawing
	glBindVertexArray(vertex_array);

	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteTextures(1, &glyph_tex);

    GL_ERRORS();
}

