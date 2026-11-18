#pragma once

#include "Mode.hpp"
#include "PixelCanvas.hpp"
#include "Sprites.hpp"
#include "text-pipeline.hpp"

#include <glm/glm.hpp>

#include <cmath>

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "Sprites.hpp"

struct SoulCaptureMode : Mode {
	//for playtesting, every difficulty adds one mechanic to the same soul and lifeline:
	enum class Mechanic { None, Pauses, Flees, Shield, Planks, Mends };
	struct Difficulty {
		char const *name = "normal";
		float lifeline_swing = 0.0f; //how far the lifeline swings to each side of its starting angle, in radians
		float wander_radius = 0.05f;
		float wander_speed = 0.6f;
		Mechanic mechanic = Mechanic::None;
	};
	static constexpr uint32_t Levels = 5;
	static Difficulty level(uint32_t n); //difficulty 1 to Levels
	//PlayMode asks for these two: normal after reading the diary, hard otherwise
	static Difficulty normal() { return level(1); }
	static Difficulty hard() { return level(3); }
	static constexpr float AngleTolerance = 0.6109f; //35 degrees
	static constexpr uint32_t HitsNeeded = 5;

	SoulCaptureMode(Difficulty const &difficulty);
	virtual ~SoulCaptureMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	void restart(Difficulty const &difficulty);

	struct SliceCheck {
		bool crosses = false;
		float angle_error = 0.0f;
		bool hit = false;
		bool blocked = false; //a plank stops the blade before it reaches the lifeline
		bool shielded = false; //the soul was shielded, so even a good cut does nothing
		glm::vec2 point = glm::vec2(0.0f);
	};
	SliceCheck check_slice(glm::vec2 a, glm::vec2 b) const;

	void slice();

	Difficulty difficulty;
	std::mt19937 mt;

	glm::vec2 soul_center = glm::vec2(0.0f);
	float soul_radius = 0.1f; //0.1 world units is exactly 24 canvas pixels, which is the size of soul.png
	glm::vec2 soul_offset = glm::vec2(0.0f);
	glm::vec2 soul_velocity = glm::vec2(0.0f);
	glm::vec2 wander_phase = glm::vec2(0.0f);
	float clock = 0.0f;
	//drives the soul's wandering and the lifeline's swing; stops while the soul pauses:
	float move_clock = 0.0f;
	glm::vec2 wander() const;
	glm::vec2 soul_position() const { return soul_center + soul_offset + wander(); }
	bool move_pending = false;
	static constexpr float ArenaRadius = 0.8f;
	static constexpr float HomeRadius = 0.4f;
	float keep_out_radius = 0.3f;
	float lifeline_angle = 0.0f;
	float lifeline_base_angle = 0.0f;
	float lifeline_half_length = 0.25f;
	//the lifeline looks like a heart monitor trace. One beat every 4/3 s (45 per minute),
	//the same pace as the 880 Hz beep in Hospital_Theme.wav:
	static constexpr float BeatSeconds = 4.0f / 3.0f;
	static constexpr float TraceSeconds = 1.0f; //how much time the trace shows along the line, less scrolls faster
	static constexpr float TraceHeight = 0.09f; //height of the spike, the soul is 0.1 across its radius
	//trace height (about -0.3 to 1) at a point of the beat, phase 0 is the spike:
	static float heartbeat(float phase) {
		auto bump = [&](float center, float width, float height) {
			float d = phase - center;
			d -= std::round(d); //wrap around so bumps near the ends of the beat show up too
			return height * std::exp(-0.5f * (d / width) * (d / width));
		};
		//wider than a real ECG: the line is only about 60 canvas pixels long
		return bump(-0.25f, 0.05f, 0.15f) //P wave
			+ bump(-0.06f, 0.018f, -0.2f) //Q
			+ bump(0.0f, 0.022f, 1.0f) //R, the spike
			+ bump(0.06f, 0.018f, -0.35f) //S
			+ bump(0.3f, 0.07f, 0.3f); //T wave
	}
	glm::vec2 lifeline_dir() const { return glm::vec2(std::cos(lifeline_angle), std::sin(lifeline_angle)); }
	glm::vec2 lifeline_end(float side) const { return soul_position() + side * lifeline_half_length * lifeline_dir(); }
	uint32_t hits = 0;
	uint32_t misses = 0;
	float streak_broken_time = 0.0f;

	glm::vec2 mouse = glm::vec2(0.0f, -0.5f);
	//WASD walks the scythe around, the mouse points it:
	glm::vec2 scythe_at = glm::vec2(0.0f, -0.5f);
	glm::vec2 pivot = glm::vec2(0.0f, -0.5f);
	float blade_angle = 0.0f;
	bool move_up = false;
	bool move_down = false;
	bool move_left = false;
	bool move_right = false;

	float slice_time = 10.0f;
	static constexpr float SlashSpeed = 4.0f;
	bool slash_reached_lifeline() const {
		return slice_time * SlashSpeed >= glm::length(slice_check.point - slice_a);
	}
	SliceCheck slice_check;
	bool knocked = false;
	glm::vec2 slice_a = glm::vec2(0.0f), slice_b = glm::vec2(0.0f);

	struct Spark {
		glm::vec2 position;
		glm::vec2 velocity;
		float age;
		float radius;
	};
	std::vector< Spark > sparks;
	static constexpr float SparkLifetime = 0.45f;

	struct Particle {
		glm::vec2 position;
		glm::vec2 velocity;
		float age;
		float lifetime;
		float radius;
	};
	std::vector< Particle > particles;
	uint32_t burst_pending = 0;
	void spawn_particles(uint32_t count, glm::vec2 a, glm::vec2 b, float spread, glm::vec2 push);

	std::string message;
	float message_time = 0.0f;

	bool message_good = false; //green or orange

	float cooldown = 0.0f;
	float timer = 0.0f;
	bool captured = false;

	//---- difficulty 1: the soul freezes for a moment now and then ----
	float pause_left = 0.0f, next_pause = 2.0f;
	//---- difficulty 2: the soul slowly backs away from the scythe ----
	static constexpr float FleeRadius = 0.9f; //starts backing away when the scythe is this close
	static constexpr float FleeSpeed = 0.5f; //world units per second when the scythe is right on it
	//---- difficulty 3: the soul is shielded for ShieldSeconds out of every ShieldPeriod ----
	static constexpr float ShieldPeriod = 5.0f, ShieldSeconds = 2.0f;
	bool shielded() const { return difficulty.mechanic == Mechanic::Shield && !captured && std::fmod(clock, ShieldPeriod) > ShieldPeriod - ShieldSeconds; }
	//---- difficulty 4: planks that stop the blade (but not the scythe walking) ----
	struct Plank { glm::vec2 a, b; };
	std::vector< Plank > planks;
	static constexpr float PlankWidth = 0.03f;
	//---- difficulty 5: the soul mends one hit for every MendSeconds without a new hit ----
	static constexpr float MendSeconds = 4.0f;
	float mend_clock = 0.0f;

	PixelCanvas canvas;
	std::shared_ptr< Mode > return_to;

	//text is drawn at screen resolution on top of the canvas. The font size follows the
	//size of the canvas on screen, so text keeps its size relative to the pixel art:
	static constexpr float LinesPerCanvas = 24.0f; //font size is canvas height / this
	std::unique_ptr< Font > font;
	struct TextLine {
		std::string text; //what image was rendered from, to skip rendering it again
		TextImage image;
	};
	TextLine status_line, hits_line, keys_line, message_line, help_lines[4];

	Sprites sprites;
	Sprites::Texture scythe_tex;
	Sprites::Texture soul_tex;

	float reach = 0.6f;
	float move_speed = 1.2f; //world units per second, the arena is 0.8 across its radius
	float slice_cooldown = 0.4f;
};
