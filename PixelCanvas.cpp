#include "PixelCanvas.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

#include <hb-ft.h>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <climits>
#include <cmath>
#include <stdexcept>
#include <vector>

PixelCanvas::~PixelCanvas() {
	if (framebuffer) glDeleteFramebuffers(1, &framebuffer); 
	if (texture) glDeleteTextures(1, &texture);
}

void PixelCanvas::begin(glm::vec3 const &background) {
	if (framebuffer == 0) {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("Pixel canvas framebuffer is incomplete.");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, Width, Height);
	glClearColor(background.r, background.g, background.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
}

void PixelCanvas::end(glm::uvec2 const &drawable_size) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, drawable_size.x, drawable_size.y);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	uint32_t scale = std::min(drawable_size.x / Width, drawable_size.y / Height);
	glm::ivec2 size = glm::ivec2(drawable_size);
	if (scale >= 1) size = glm::ivec2(scale * Width, scale * Height);
	glm::ivec2 lower_left = (glm::ivec2(drawable_size) - size) / 2;
	rect = glm::vec4(glm::vec2(lower_left) / glm::vec2(drawable_size), glm::vec2(size) / glm::vec2(drawable_size));

	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDisable(GL_FRAMEBUFFER_SRGB);
	glBlitFramebuffer(0, 0, Width, Height,
		lower_left.x, lower_left.y, lower_left.x + size.x, lower_left.y + size.y,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

glm::vec2 PixelCanvas::from_window(glm::vec2 const &position, glm::uvec2 const &window_size) const {
	glm::vec2 f = glm::vec2(position.x / float(window_size.x), 1.0f - position.y / float(window_size.y));
	glm::vec2 uv = (f - glm::vec2(rect.x, rect.y)) / glm::vec2(rect.z, rect.w);
	return glm::vec2((uv.x * 2.0f - 1.0f) * Aspect, uv.y * 2.0f - 1.0f);
}

glm::vec2 PixelCanvas::world_to_pixel(glm::vec2 const &world) const {
	return glm::vec2((world.x / Aspect + 1.0f) * 0.5f * Width, (world.y + 1.0f) * 0.5f * Height);
}

glm::vec2 PixelCanvas::pixel_to_drawable(glm::vec2 const &pixel, glm::uvec2 const &drawable_size) const {
	glm::vec2 size = glm::vec2(drawable_size);
	return glm::vec2(rect.x, rect.y) * size + pixel / glm::vec2(Width, Height) * glm::vec2(rect.z, rect.w) * size;
}

glm::mat4 PixelCanvas::world_to_clip() const {
	return glm::mat4(
		1.0f / Aspect, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

//---------------- text ----------------

Font::Font(std::string const &path, uint32_t pixel_size_) : pixel_size(pixel_size_) {
	if (FT_Init_FreeType(&ft_library)) throw std::runtime_error("Could not start FreeType.");
	if (FT_New_Face(ft_library, path.c_str(), 0, &ft_face)) throw std::runtime_error("Could not open font '" + path + "'.");
	if (FT_Set_Pixel_Sizes(ft_face, 0, pixel_size)) throw std::runtime_error("Could not set font size " + std::to_string(pixel_size) + ".");
	hb_font = hb_ft_font_create(ft_face, nullptr);
}

Font::~Font() {
	if (hb_font) hb_font_destroy(hb_font);
	if (ft_face) FT_Done_Face(ft_face);
	if (ft_library) FT_Done_FreeType(ft_library);
}

TextImage::~TextImage() {
	if (texture) glDeleteTextures(1, &texture);
}

TextImage::TextImage(TextImage &&other) : texture(other.texture), size(other.size), offset(other.offset), advance(other.advance) {
	other.texture = 0;
}

TextImage &TextImage::operator=(TextImage &&other) {
	if (this != &other) {
		if (texture) glDeleteTextures(1, &texture);
		texture = other.texture;
		size = other.size;
		offset = other.offset;
		advance = other.advance;
		other.texture = 0;
	}
	return *this;
}

TextImage render_text(Font &font, std::string const &text) {
	TextImage image;
	if (text.empty()) return image;

	hb_buffer_t *buffer = hb_buffer_create();
	hb_buffer_add_utf8(buffer, text.c_str(), -1, 0, -1);
	hb_buffer_guess_segment_properties(buffer);
	hb_shape(font.hb_font, buffer, nullptr, 0);
	unsigned int count = 0;
	hb_glyph_info_t *info = hb_buffer_get_glyph_infos(buffer, &count);
	hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(buffer, &count);

	//render every glyph and note where it goes relative to the start of the baseline:
	struct Glyph {
		std::vector< uint8_t > pixels; //top row first, like FreeType hands them out
		int width = 0, rows = 0;
		int left = 0, bottom = 0;
	};
	std::vector< Glyph > glyphs;
	glyphs.reserve(count);
	glm::ivec2 lo = glm::ivec2(INT_MAX), hi = glm::ivec2(INT_MIN);
	int pen_x = 0, pen_y = 0; //HarfBuzz positions are in 1/64 pixel
	for (unsigned int i = 0; i < count; ++i) {
		if (FT_Load_Glyph(font.ft_face, info[i].codepoint, FT_LOAD_RENDER) == 0) {
			FT_GlyphSlot slot = font.ft_face->glyph;
			FT_Bitmap const &bitmap = slot->bitmap;
			Glyph g;
			g.width = int(bitmap.width);
			g.rows = int(bitmap.rows);
			g.left = int(std::lround((pen_x + pos[i].x_offset) / 64.0)) + slot->bitmap_left;
			g.bottom = int(std::lround((pen_y + pos[i].y_offset) / 64.0)) + slot->bitmap_top - g.rows;
			if (g.width > 0 && g.rows > 0 && bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
				g.pixels.resize(size_t(g.width) * g.rows);
				for (int r = 0; r < g.rows; ++r) {
					unsigned char const *src = bitmap.buffer + r * bitmap.pitch;
					std::copy(src, src + g.width, g.pixels.begin() + size_t(r) * g.width);
				}
				lo = glm::min(lo, glm::ivec2(g.left, g.bottom));
				hi = glm::max(hi, glm::ivec2(g.left + g.width, g.bottom + g.rows));
				glyphs.emplace_back(std::move(g));
			}
		}
		pen_x += pos[i].x_advance;
		pen_y += pos[i].y_advance;
	}
	hb_buffer_destroy(buffer);

	image.advance = int(std::lround(pen_x / 64.0));
	if (glyphs.empty()) return image; //only spaces

	//paste the glyphs into one picture, bottom row first because that is how OpenGL reads it:
	image.offset = lo;
	image.size = hi - lo;
	std::vector< uint8_t > pixels(size_t(image.size.x) * image.size.y, 0);
	for (Glyph const &g : glyphs) {
		for (int r = 0; r < g.rows; ++r) {
			int y = (g.bottom - lo.y) + (g.rows - 1 - r);
			uint8_t *dst = pixels.data() + size_t(y) * image.size.x + (g.left - lo.x);
			uint8_t const *src = g.pixels.data() + size_t(r) * g.width;
			for (int c = 0; c < g.width; ++c) dst[c] = std::max(dst[c], src[c]); //glyphs may overlap a little
		}
	}

	glGenTextures(1, &image.texture);
	glBindTexture(GL_TEXTURE_2D, image.texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //rows are not padded to 4 bytes
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, image.size.x, image.size.y, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_ERRORS();

	return image;
}

void draw_text(TextImage const &image, glm::vec2 baseline_start, glm::vec4 const &color, glm::uvec2 const &drawable_size) {
	if (image.texture == 0) return;

	//one shader and one quad buffer shared by all text, made on first use:
	static GLuint program = 0, vertex_array = 0, vertex_buffer = 0;
	static GLint to_clip_loc = -1, color_loc = -1, image_loc = -1;
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
			"uniform vec4 COLOR;\n"
			"in vec2 texCoord;\n"
			"out vec4 fragColor;\n"
			"void main() {\n"
			"	fragColor = vec4(COLOR.rgb, COLOR.a * texture(IMAGE, texCoord).r);\n"
			"}\n"
		);
		to_clip_loc = glGetUniformLocation(program, "TO_CLIP");
		color_loc = glGetUniformLocation(program, "COLOR");
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

	//snap to whole pixels so the glyphs stay sharp:
	glm::vec2 lo = glm::round(baseline_start) + glm::vec2(image.offset);
	glm::vec2 hi = lo + glm::vec2(image.size);
	float const quad[6][4] = {
		{ lo.x, lo.y, 0.0f, 0.0f }, { hi.x, lo.y, 1.0f, 0.0f }, { hi.x, hi.y, 1.0f, 1.0f },
		{ lo.x, lo.y, 0.0f, 0.0f }, { hi.x, hi.y, 1.0f, 1.0f }, { lo.x, hi.y, 0.0f, 1.0f },
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
	glUniform4fv(color_loc, 1, glm::value_ptr(color));
	glUniform1i(image_loc, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image.texture);
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
