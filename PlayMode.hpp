#include "Mode.hpp"
#include "Sprites.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <string>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	static constexpr float ScreenWidth = 176.0f;
	static constexpr float ScreenHeight = 160.0f;


	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, up, down, interact;

	glm::vec2 player_at = glm::vec2(128.0f, 60.0f);
	float hover_time = 0.0f;

	std::vector<std::string> room;

	struct Interactable {
		std::string name;
		glm::vec2 pos;
		float radius;
	};
	std::vector<Interactable> interactables;
	int nearby = -1;

	bool tile_is_solid(int tx, int ty);
	bool can_move_to(glm::vec2 pos);
	void on_interact(Interactable const &thing);

	Sprites sprites;
	Sprites::Texture reaper_tex;
	Sprites::Texture floor_tex;
	Sprites::Texture patient1_tex;
	Sprites::Texture man1_tex;
	Sprites::Texture woman1_tex;
	Sprites::Texture partition_tex;
	Sprites::Texture cardiogram_tex;
	Sprites::Texture iv_tex;
	Sprites::Texture desk_tex;
	Sprites::Texture interact_tex;

	struct Prop {
		Sprites::Texture tex;
		glm::vec2 pos;
	};
	std::vector<Prop> props;

	bool diary_read[2] = { false, false };
	bool soul_taken[2] = { false, false };

	void reset_buttons();
};