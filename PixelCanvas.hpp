#pragma once

//The pixel canvas the soul capture minigame draws into, plus a small text helper
//(Font, TextImage, render_text, draw_text) for drawing text on top of it.

#include "GL.hpp"

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>

#include <string>

struct PixelCanvas {
	static constexpr uint32_t Width = 256, Height = 240;
	static constexpr float Aspect = float(Width) / float(Height);

	PixelCanvas() = default;
	PixelCanvas(PixelCanvas const &) = delete;
	~PixelCanvas();

	void begin(glm::vec3 const &background);
	void end(glm::uvec2 const &drawable_size);
	glm::vec2 from_window(glm::vec2 const &position, glm::uvec2 const &window_size) const;
	glm::mat4 world_to_clip() const;
	//world units to canvas pixels, (0,0) is the lower left pixel:
	glm::vec2 world_to_pixel(glm::vec2 const &world) const;
	//canvas pixels to drawable pixels, for drawing on top of the canvas after end():
	glm::vec2 pixel_to_drawable(glm::vec2 const &pixel, glm::uvec2 const &drawable_size) const;

	GLuint framebuffer = 0, texture = 0;
	glm::vec4 rect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
};

//---------------- text ----------------
// Simple text drawing in three steps, like pygame:
//   Font font(data_path("LowresPixel-Regular.otf"), 32);  //once
//   TextImage image = render_text(font, "hits 3/5");     //only when the text changes
//   draw_text(image, position, color, drawable_size);     //every frame
// The image only stores coverage (how much of each pixel the glyphs cover), the
// color is picked when drawing, so the same image can be drawn green or orange
// or faded out without rendering it again.

struct Font {
	Font(std::string const &path, uint32_t pixel_size);
	~Font();
	Font(Font const &) = delete;
	Font &operator=(Font const &) = delete;

	uint32_t pixel_size = 0;
	FT_Library ft_library = nullptr;
	FT_Face ft_face = nullptr;
	hb_font_t *hb_font = nullptr;
};

struct TextImage {
	TextImage() = default;
	~TextImage();
	//the texture is freed when the image goes away, so copying is not allowed
	//(two copies would free the same texture twice), moving hands it over:
	TextImage(TextImage const &) = delete;
	TextImage &operator=(TextImage const &) = delete;
	TextImage(TextImage &&other);
	TextImage &operator=(TextImage &&other);

	GLuint texture = 0; //one byte per pixel, 0 for an empty string
	glm::ivec2 size = glm::ivec2(0); //in pixels
	glm::ivec2 offset = glm::ivec2(0); //lower left corner of the image, relative to where the baseline starts
	int advance = 0; //how far the pen moved, i.e. the width of the text as typed
};

TextImage render_text(Font &font, std::string const &text);

//baseline_start is where the text starts on its baseline, in drawable pixels with
//(0,0) at the lower left of the window. color.a fades the whole text.
void draw_text(TextImage const &image, glm::vec2 baseline_start, glm::vec4 const &color, glm::uvec2 const &drawable_size);
