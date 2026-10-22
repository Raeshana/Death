#include <fstream>
#include <cassert>
#include <cstdint>
#include <vector>

#include "load_save_png.hpp"

//code from class (Professor McCann) but with 4 tiles
int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	glm::uvec2 size;
	std::vector< glm::u8vec4 > png;
	load_png("sprites/sprites.png", &size, &png, LowerLeftOrigin);

	std::vector< glm::u8vec4 > palette{
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
		glm::u8vec4(0xbb, 0xa2, 0x0a, 0xff),
	};

	std::ofstream assets_hpp("assets.hpp", std::ios::binary);

	const char *tile_names[] = { "Player_TILE", "Ground_TILE", "Goal_TILE", "Laser_TILE" };
	const int num_tiles = 4;

	auto write_bit = [&](int t, uint8_t bit) {
		for (uint32_t y = 0; y < 8; ++y) {
			assets_hpp << "0b";
			for (int x = 7; x >= 0; --x) {
				glm::u8vec4 col = png[y * size.x + uint32_t(t * 8 + x)];
				size_t found = 0xff;
				for (size_t i = 0; i < palette.size(); ++i) {
					if (col == palette[i]) { found = i; break; }
				}
				assert(found != 0xff && "Pixel not in palette.");
				char bitc = (((found >> bit) & 1) ? '1' : '0');
				assets_hpp << bitc;
			}
			assets_hpp << ", ";
		}
	};

	for (int t = 0; t < num_tiles; ++t) {
		assets_hpp << "#define " << tile_names[t] << " PPU466::Tile{ \\\n";
		assets_hpp << "\t.bit0={";
		write_bit(t, 0);
		assets_hpp << "}, \\\n\t.bit1={";
		write_bit(t, 1);
		assets_hpp << "} \\\n}\n";
	}
}