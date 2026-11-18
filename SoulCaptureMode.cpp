#include "SoulCaptureMode.hpp"

#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

static glm::vec3 const Background = glm::vec3(0.02f, 0.02f, 0.03f);
static glm::u8vec4 const Green = glm::u8vec4(0x44, 0xff, 0x44, 0xff);
static glm::u8vec4 const Red = glm::u8vec4(0xff, 0x44, 0x44, 0xff);
static glm::u8vec4 const Orange = glm::u8vec4(0xff, 0x88, 0x22, 0xff);
static glm::u8vec4 const White = glm::vec4(0xff, 0xff, 0xff, 0xff);

glm::vec2 SoulCaptureMode::wander() const {
	float t = move_clock * difficulty.wander_speed;
	glm::vec2 loop = glm::vec2(std::cos(t + wander_phase.x), std::sin(t + wander_phase.x));
	glm::vec2 wobble = glm::vec2(std::cos(2.3f * t + wander_phase.y), std::sin(1.7f * t + wander_phase.y));
	return difficulty.wander_radius * (loop + 0.5f * wobble) / 1.5f;
}

SoulCaptureMode::Difficulty SoulCaptureMode::level(uint32_t n) {
	Difficulty d; //every difficulty keeps the gentle soul movement unless noted, only the mechanic differs
	if (n <= 1) {
		d.name = "difficulty 1: pauses";
		d.mechanic = Mechanic::Pauses;
		//it darts around more so the sudden stops are noticeable:
		d.wander_radius = 0.12f;
		d.wander_speed = 1.3f;
	} else if (n == 2) {
		d.name = "difficulty 2: flees";
		d.mechanic = Mechanic::Flees;
	} else if (n == 3) {
		d.name = "difficulty 3: shield";
		d.mechanic = Mechanic::Shield;
	} else if (n == 4) {
		d.name = "difficulty 4: planks";
		d.mechanic = Mechanic::Planks;
	} else {
		d.name = "difficulty 5: mends";
		d.mechanic = Mechanic::Mends;
	}
	return d;
}

SoulCaptureMode::SoulCaptureMode(Difficulty const &difficulty_) : difficulty(difficulty_), mt(std::random_device()()) {
	std::uniform_real_distribution< float > turn(0.0f, 2.0f * float(M_PI));
	std::uniform_real_distribution< float > unit(0.0f, 1.0f);
	wander_phase = glm::vec2(turn(mt), turn(mt));
	scythe_tex = sprites.load(data_path("scythe.png"));
	soul_tex = sprites.load(data_path("soul.png"));
	lifeline_base_angle = 0.5f * turn(mt);
	lifeline_angle = lifeline_base_angle;

	if (difficulty.mechanic == Mechanic::Planks) {
		//three planks around the ring between the soul's home area and the arena edge,
		//lying roughly along the circle so they block cuts from that side:
		float start = turn(mt);
		for (uint32_t i = 0; i < 3; ++i) {
			float angle = start + i * 2.0f * float(M_PI) / 3.0f + 0.4f * (unit(mt) - 0.5f);
			glm::vec2 center = glm::mix(0.5f, 0.65f, unit(mt)) * glm::vec2(std::cos(angle), std::sin(angle));
			float tilt = angle + 0.5f * float(M_PI) + 0.6f * (unit(mt) - 0.5f);
			glm::vec2 half = 0.195f * glm::vec2(std::cos(tilt), std::sin(tilt));
			planks.emplace_back(Plank{ center - half, center + half });
		}
	}
}

SoulCaptureMode::~SoulCaptureMode() {
}

void SoulCaptureMode::restart(Difficulty const &difficulty_) {
	auto next = std::make_shared< SoulCaptureMode >(difficulty_);
	next->mouse = mouse;
	next->scythe_at = scythe_at;
	next->blade_angle = blade_angle;
	next->canvas.rect = canvas.rect;
	next->return_to = return_to;
	Mode::set_current(next);
}

static bool segments_cross(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float *t) {
	auto side = [](glm::vec2 r, glm::vec2 p, glm::vec2 q) {
		return (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
	};
	float side_a = side(a, c, d), side_b = side(b, c, d);
	if (!(side_a * side_b < 0.0f && side(c, a, b) * side(d, a, b) < 0.0f)) return false;
	*t = side_a / (side_a - side_b);
	return true;
}

SoulCaptureMode::SliceCheck SoulCaptureMode::check_slice(glm::vec2 a, glm::vec2 b) const {
	SliceCheck ret;
	float t = 0.0f;
	ret.crosses = segments_cross(a, b, lifeline_end(-1.0f), lifeline_end(1.0f), &t);
	if (ret.crosses) ret.point = a + t * (b - a);
	for (Plank const &p : planks) { //a plank closer along the blade than the lifeline stops it
		float tp = 0.0f;
		if (segments_cross(a, b, p.a, p.b, &tp) && (!ret.crosses || tp < t)) {
			ret.blocked = true;
			ret.crosses = false;
			ret.point = a + tp * (b - a);
			t = tp;
		}
	}
	float angle = std::acos(std::min(1.0f, std::abs(glm::dot(glm::normalize(b - a), lifeline_dir()))));
	ret.angle_error = 0.5f * float(M_PI) - angle;
	ret.shielded = shielded();
	ret.hit = ret.crosses && ret.angle_error < AngleTolerance && !ret.shielded;
	return ret;
}

void SoulCaptureMode::slice() {
	if (captured || cooldown > 0.0f) return;
	cooldown = slice_cooldown;

	glm::vec2 blade_dir = glm::vec2(std::cos(blade_angle), std::sin(blade_angle));
	slice_a = pivot;
	slice_b = pivot + reach * blade_dir;

	SliceCheck check = check_slice(slice_a, slice_b);
	slice_check = check;
	slice_time = 0.0f;
	knocked = false;

	{
		constexpr uint32_t Count = 24;
		std::uniform_real_distribution< float > jitter(-0.12f, 0.12f);
		glm::vec2 side_dir = glm::vec2(-blade_dir.y, blade_dir.x);
		for (uint32_t i = 0; i < Count; ++i) {
			Spark s;
			s.position = slice_a;
			s.velocity = SlashSpeed * blade_dir + jitter(mt) * side_dir;
			s.age = -0.004f * i;
			s.radius = glm::mix(0.016f, 0.005f, i / float(Count - 1));
			sparks.emplace_back(s);
		}
	}
	message_time = 1.5f;
	message_good = check.hit;
	if (check.blocked) {
		message = "Blocked by a plank.";
	} else if (!check.crosses) {
		message = "Missed the lifeline.";
	} else if (check.shielded) {
		message = "The soul is shielded.";
	} else if (!check.hit) {
		message = "Off by " + std::to_string(int(glm::degrees(check.angle_error))) + " degrees.";
	}
	//a miss no longer wipes the streak, so there is nothing to announce:
	//if (!check.hit && hits > 0) message += " Streak reset.";

	if (slice_check.hit) {
		hits += 1;
		mend_clock = 0.0f;
		if (hits >= HitsNeeded) {
			captured = true;
			burst_pending = 80;
			message = return_to ? "Soul captured! Press Space to continue." : "Soul captured!  R: retry   1-5: difficulty";
		} else {
			burst_pending = 12;
			message = "Hit! " + std::to_string(HitsNeeded - hits) + " more to go.";
		}
	} else {
		misses += 1;
		//a miss used to wipe the whole streak, which made the hard soul brutal:
		//if (hits > 0) streak_broken_time = 0.6f;
		//hits = 0;
	}
	if (!captured) move_pending = true;
}

bool SoulCaptureMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	bool go_on = evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN || (evt.type == SDL_EVENT_KEY_DOWN && (evt.key.key == SDLK_SPACE || evt.key.key == SDLK_E));
	if (captured && return_to && go_on && slice_time > 1.0f) {
		auto self = shared_from_this();
		Mode::set_current(std::move(return_to));
		return true;
	}
	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_A) {
			move_left = true;
			return true;
		} else if (evt.key.key == SDLK_D) {
			move_right = true;
			return true;
		} else if (evt.key.key == SDLK_W) {
			move_up = true;
			return true;
		} else if (evt.key.key == SDLK_S) {
			move_down = true;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			slice();
			return true;
		} else if (evt.key.key == SDLK_ESCAPE && return_to && !captured) {
			//step back into the ward: the player may want to look around before trying again
			auto self = shared_from_this();
			Mode::set_current(std::move(return_to));
			return true;
		} else if (evt.key.key >= SDLK_1 && evt.key.key < SDLK_1 + Levels) {
			//playtest: jump to a difficulty and start over
			auto self = shared_from_this();
			restart(level(uint32_t(evt.key.key - SDLK_1) + 1));
			return true;
		} else if (!return_to && evt.key.key == SDLK_R) {
			auto self = shared_from_this();
			restart(difficulty);
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_A) {
			move_left = false;
			return true;
		} else if (evt.key.key == SDLK_D) {
			move_right = false;
			return true;
		} else if (evt.key.key == SDLK_W) {
			move_up = false;
			return true;
		} else if (evt.key.key == SDLK_S) {
			move_down = false;
			return true;
		}
	} else if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
		if (evt.button.button == SDL_BUTTON_LEFT) {
			slice();
			return true;
		}
	} else if (evt.type == SDL_EVENT_MOUSE_MOTION) {
		mouse = canvas.from_window(glm::vec2(evt.motion.x, evt.motion.y), window_size);
		return true;
	}

	return false;
}

void SoulCaptureMode::update(float elapsed) {
	{ //walk the scythe with WASD, keeping it inside the arena
		glm::vec2 move = glm::vec2(0.0f);
		if (move_left) move.x -= 1.0f;
		if (move_right) move.x += 1.0f;
		if (move_down) move.y -= 1.0f;
		if (move_up) move.y += 1.0f;
		if (move != glm::vec2(0.0f)) scythe_at += glm::normalize(move) * move_speed * elapsed;
		if (glm::length(scythe_at) > ArenaRadius) scythe_at = ArenaRadius * glm::normalize(scythe_at);
	}

	clock += elapsed;

	if (difficulty.mechanic == Mechanic::Pauses && !captured) {
		if (pause_left > 0.0f) {
			pause_left = std::max(0.0f, pause_left - elapsed);
		} else {
			next_pause -= elapsed;
			if (next_pause <= 0.0f) {
				std::uniform_real_distribution< float > unit(0.0f, 1.0f);
				pause_left = glm::mix(0.5f, 1.0f, unit(mt));
				next_pause = glm::mix(1.5f, 3.5f, unit(mt));
			}
		}
	}
	if (pause_left == 0.0f) move_clock += elapsed;

	if (difficulty.mechanic == Mechanic::Flees && !captured && !move_pending) {
		glm::vec2 from = soul_center - scythe_at;
		float dist = glm::length(from);
		if (dist < FleeRadius && dist > 1e-4f) {
			soul_center += FleeSpeed * (1.0f - dist / FleeRadius) * elapsed * from / dist;
			if (glm::length(soul_center) > HomeRadius) soul_center = HomeRadius * glm::normalize(soul_center);
		}
	}

	if (difficulty.mechanic == Mechanic::Mends && !captured && hits > 0) {
		mend_clock += elapsed;
		if (mend_clock >= MendSeconds) {
			mend_clock = 0.0f;
			hits -= 1;
			streak_broken_time = 0.6f;
			message = "The soul mends itself.";
			message_time = 1.5f;
			message_good = false;
		}
	}

	if (move_pending && slice_time >= reach / SlashSpeed) {
		std::uniform_real_distribution< float > unit(0.0f, 1.0f);
		float angle = unit(mt) * 2.0f * float(M_PI);
		glm::vec2 home = HomeRadius * std::sqrt(unit(mt)) * glm::vec2(std::cos(angle), std::sin(angle));
		soul_offset -= home - soul_center;
		soul_center = home;
		move_pending = false;
	}

	pivot = scythe_at;
	glm::vec2 away = pivot - soul_position();
	if (glm::length(away) < keep_out_radius) {
		if (glm::length(away) < 1e-4f) away = glm::vec2(0.0f, -1.0f);
		pivot = soul_position() + keep_out_radius * glm::normalize(away);
	}

	{ //the blade points at the mouse; before the mouse has moved it points at the soul
		glm::vec2 aim = mouse - pivot;
		if (glm::length(aim) < 1e-3f) aim = soul_position() - pivot;
		if (glm::length(aim) > 1e-6f) blade_angle = std::atan2(aim.y, aim.x);
	}

	cooldown = std::max(0.0f, cooldown - elapsed);
	streak_broken_time = std::max(0.0f, streak_broken_time - elapsed);
	message_time = std::max(0.0f, message_time - elapsed);

	slice_time += elapsed;
	if (burst_pending > 0 && slash_reached_lifeline()) {
		spawn_particles(burst_pending, lifeline_end(-1.0f), lifeline_end(1.0f), 0.01f, glm::normalize(slice_b - slice_a));
		burst_pending = 0;
	}
	{
		bool lifeline_there = !(captured && slash_reached_lifeline());
		glm::vec2 line_dir = lifeline_dir(), line_a = lifeline_end(-1.0f), line_b = lifeline_end(1.0f);
		for (auto &s : sparks) {
			s.age += elapsed;
			if (s.age < 0.0f) continue;
			float step = std::min(elapsed, s.age);
			glm::vec2 next = s.position + s.velocity * step;
			float t = 0.0f;
			if (lifeline_there && !slice_check.hit && segments_cross(s.position, next, line_a, line_b, &t)) {
				glm::vec2 at = s.position + t * (next - s.position);
				s.velocity = 2.0f * glm::dot(s.velocity, line_dir) * line_dir - s.velocity;
				next = at + s.velocity * ((1.0f - t) * step);
				if (!knocked) {
					soul_velocity += 1.2f * glm::normalize(slice_b - slice_a);
					knocked = true;
				}
			}
			for (Plank const &p : planks) {
				if (segments_cross(s.position, next, p.a, p.b, &t)) {
					glm::vec2 dir = glm::normalize(p.b - p.a);
					glm::vec2 at = s.position + t * (next - s.position);
					s.velocity = 2.0f * glm::dot(s.velocity, dir) * dir - s.velocity;
					next = at + s.velocity * ((1.0f - t) * step);
					break;
				}
			}
			s.position = next;
		}
		sparks.erase(std::remove_if(sparks.begin(), sparks.end(), [](Spark const &s) { return s.age >= SparkLifetime; }), sparks.end());
	}
	soul_velocity += (-200.0f * soul_offset - 14.0f * soul_velocity) * elapsed;
	soul_offset += soul_velocity * elapsed;

	for (auto &p : particles) {
		p.age += elapsed;
		p.position += p.velocity * elapsed;
		p.velocity *= std::exp(-2.0f * elapsed);
		p.velocity.y += 0.3f * elapsed;
	}
	particles.erase(std::remove_if(particles.begin(), particles.end(), [](Particle const &p) { return p.age >= p.lifetime; }), particles.end());

	if (!captured) {
		timer += elapsed;
		//swing around the starting angle by a fixed amount instead of drifting away from it:
		lifeline_angle = lifeline_base_angle + difficulty.lifeline_swing * std::sin(0.9f * move_clock + wander_phase.x);
	}
}

void SoulCaptureMode::spawn_particles(uint32_t count, glm::vec2 a, glm::vec2 b, float spread, glm::vec2 push) {
	std::uniform_real_distribution< float > unit(0.0f, 1.0f);
	for (uint32_t i = 0; i < count; ++i) {
		float angle = unit(mt) * 2.0f * float(M_PI);
		glm::vec2 dir = glm::vec2(std::cos(angle), std::sin(angle));
		Particle p;
		p.position = glm::mix(a, b, unit(mt)) + dir * spread * std::sqrt(unit(mt));
		p.velocity = dir * glm::mix(0.05f, 0.25f, unit(mt)) + push * glm::mix(0.3f, 1.0f, unit(mt));
		p.age = 0.0f;
		p.lifetime = glm::mix(0.6f, 1.4f, unit(mt));
		p.radius = glm::mix(0.008f, 0.022f, unit(mt));
		particles.emplace_back(p);
	}
}

void SoulCaptureMode::draw(glm::uvec2 const &drawable_size) {
	canvas.begin(Background);
	glm::vec4 blade_tint = glm::vec4(1.0f);
	{
		DrawLines lines(canvas.world_to_clip());

		auto circle = [&](glm::vec2 center, float radius, glm::u8vec4 color) {
			for (uint32_t i = 0; i < 48; ++i) {
				float a0 = i / 48.0f * 2.0f * float(M_PI), a1 = (i + 1) / 48.0f * 2.0f * float(M_PI);
				lines.draw(glm::vec3(center + radius * glm::vec2(std::cos(a0), std::sin(a0)), 0.0f), glm::vec3(center + radius * glm::vec2(std::cos(a1), std::sin(a1)), 0.0f), color);
			}
		};

		glm::vec3 const background = Background * 255.0f;
		auto fade = [&](glm::u8vec4 color, float amount) {
			return glm::u8vec4(glm::mix(glm::vec3(color), background, glm::clamp(amount, 0.0f, 1.0f)), 0xff);
		};
		glm::u8vec4 const soul_color = glm::u8vec4(0x99, 0xdd, 0xff, 0xff);

		circle(glm::vec2(0.0f), ArenaRadius, glm::u8vec4(0x44, 0x44, 0x55, 0xff));
		circle(soul_position(), keep_out_radius, glm::u8vec4(0x2a, 0x2a, 0x36, 0xff));
		for (Plank const &p : planks) {
			lines.fill_line(p.a, p.b, PlankWidth + 0.012f, glm::u8vec4(0x5c, 0x3a, 0x24, 0xff));
			lines.fill_line(p.a, p.b, PlankWidth, glm::u8vec4(0x9a, 0x66, 0x40, 0xff));
		}
		bool soul_gone = captured && slash_reached_lifeline();
		if (!soul_gone && shielded()) { //a white glow around the shielded soul
			circle(soul_position(), 1.25f * soul_radius, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
			circle(soul_position(), 1.35f * soul_radius, glm::u8vec4(0xaa, 0xaa, 0xbb, 0xff));
		}
		if (!soul_gone) {
			glm::vec2 size = glm::vec2(2.0f * soul_radius);
			sprites.begin_world(canvas.world_to_clip());
			sprites.draw(soul_tex, soul_position() - 0.5f * size, size);
			sprites.end_world();
		}

		for (auto const &p : particles) {
			float t = p.age / p.lifetime;
			glm::u8vec4 bright = glm::u8vec4(glm::mix(glm::vec3(soul_color), glm::vec3(255.0f), 0.6f * (1.0f - t)), 0xff);
			lines.fill_circle(p.position, p.radius * (1.0f - 0.5f * t), fade(bright, t), 8);
		}

		if (!soul_gone) {
			//the lifeline is drawn as a heart monitor trace scrolling along it; hits are
			//still checked against the straight line, so this only changes the look
			glm::vec2 a = lifeline_end(-1.0f), b = lifeline_end(1.0f);
			glm::vec2 across = glm::vec2(-lifeline_dir().y, lifeline_dir().x);
			glm::u8vec4 trace = glm::u8vec4(0x80, 0xcc, 0xff, 0xff); //a little deeper than the soul's 0x99ddff
			if (shielded()) trace = glm::u8vec4(0x77, 0x77, 0x88, 0xff); //greyed out while it can't be cut
			constexpr uint32_t Steps = 96;
			glm::vec2 prev = a;
			for (uint32_t i = 0; i <= Steps; ++i) {
				float s = i / float(Steps); //0 is the oldest end of the trace, 1 the newest
				float beat = (clock - (1.0f - s) * TraceSeconds) / BeatSeconds;
				glm::vec2 at = glm::mix(a, b, s) + TraceHeight * heartbeat(beat - std::floor(beat)) * across;
				if (i > 0) {
					glm::u8vec4 color = fade(trace, 0.5f * (1.0f - s));
					lines.fill_line(prev, at, 0.02f, color);
					lines.fill_circle(at, 0.01f, color, 8);
				}
				prev = at;
			}
			for (uint32_t i = 0; i < HitsNeeded; ++i) {
				float angle = 0.5f * float(M_PI) - i / float(HitsNeeded) * 2.0f * float(M_PI);
				glm::vec2 at = soul_position() + keep_out_radius * glm::vec2(std::cos(angle), std::sin(angle));
				if (streak_broken_time > 0.0f) {
					lines.fill_circle(at, 0.022f, fade(Red, 1.0f - streak_broken_time / 0.6f), 12);
				} else if (i < hits) {
					lines.fill_circle(at, 0.022f, Green, 12);
				} else {
					circle(at, 0.02f, glm::u8vec4(0x55, 0x55, 0x66, 0xff));
				}
			}
		}

		if (!captured) {
			glm::vec2 d = glm::vec2(std::cos(blade_angle), std::sin(blade_angle));
			SliceCheck check = check_slice(pivot, pivot + reach * d);
			glm::u8vec4 color = glm::u8vec4(0xff, 0xff, 0xff, 0xff);
			if (cooldown > 0.0f) color = glm::u8vec4(0x66, 0x66, 0x66, 0xff);
			else if (check.blocked) color = Red;
			else if (check.crosses && check.shielded) color = Orange;
			else if (check.hit) color = Green;
			else if (check.crosses) {
				float off = (check.angle_error - AngleTolerance) / (0.5f * float(M_PI) - AngleTolerance);
				color = glm::u8vec4(glm::mix(glm::vec3(Orange), glm::vec3(Green), 0.7f * (1.0f - glm::clamp(off, 0.0f, 1.0f))), 0xff);
			}
			blade_tint = glm::vec4(glm::mix(glm::vec3(1.0f), glm::vec3(color) / 255.0f, 0.5f), 1.0f);
		}

		for (auto const &s : sparks) {
			if (s.age < 0.0f) continue;
			float t = s.age / SparkLifetime;
			lines.fill_circle(s.position, s.radius * (1.0f - 0.6f * t), fade(slice_check.hit ? Green : Red, t * t), 8);
		}

		//the canvas is only 240 pixels tall, so small glyphs lose their shape: keep them big
	}

	if (!captured) {
		glm::vec2 d = glm::vec2(std::cos(blade_angle), std::sin(blade_angle));
		float height = reach * float(scythe_tex.size.y) / float(scythe_tex.size.x);
		glm::vec2 size = glm::vec2(reach, height);
		glm::vec2 center = pivot + 0.5f * reach * d;
		sprites.begin_world(canvas.world_to_clip());
		sprites.draw(scythe_tex, center - 0.5f * size, size, blade_tint, blade_angle);
		sprites.end_world();
	}

	canvas.end(drawable_size);

	//text is drawn after the canvas is on screen, so it gets full resolution and the
	//same pixel font as the dialogue box instead of 15 pixel tall line art:
	{
		//the font follows the size of the canvas on screen; a new font means every line renders again:
		float canvas_height = canvas.rect.w * drawable_size.y;
		uint32_t font_size = std::max(8u, uint32_t(std::lround(canvas_height / LinesPerCanvas)));
		bool new_font = !font || font->pixel_size != font_size;
		if (new_font) font = std::make_unique< Font >(data_path("LowresPixel-Regular.otf"), font_size);
		auto set_text = [&](TextLine &line, std::string const &text) {
			if (new_font || line.text != text) {
				line.image = render_text(*font, text);
				line.text = text;
			}
		};

		//positions below are in canvas pixels: 256 x 240 with (0,0) at the lower left
		float const line_height = PixelCanvas::Height / LinesPerCanvas;
		float const margin = 0.5f * line_height;
		auto draw_line = [&](TextLine const &line, glm::vec2 baseline_start, glm::u8vec4 color, float alpha = 1.0f) {
			glm::vec4 rgba = glm::vec4(glm::vec3(color) / 255.0f, alpha);
			draw_text(line.image, canvas.pixel_to_drawable(baseline_start, drawable_size), rgba, drawable_size);
		};
		auto width_in_pixels = [&](TextLine const &line) {
			return line.image.advance / (canvas.rect.z * drawable_size.x) * PixelCanvas::Width;
		};

		char buf[128];
		snprintf(buf, sizeof(buf), "%s  misses %u  %.1fs", difficulty.name, misses, timer);
		set_text(status_line, buf);
		draw_line(status_line, glm::vec2(margin, PixelCanvas::Height - 1.2f * line_height), White);
		snprintf(buf, sizeof(buf), "hits %u/%u", hits, HitsNeeded);
		set_text(hits_line, buf);
		draw_line(hits_line, glm::vec2(margin, PixelCanvas::Height - 2.4f * line_height), Green);
		set_text(keys_line, "playtest: keys 1-5 switch the difficulty");
		draw_line(keys_line, glm::vec2(margin, PixelCanvas::Height - 3.6f * line_height), glm::u8vec4(0x99, 0x99, 0xaa, 0xff));

		if (captured || message_time > 0.0f) {
			//show the result just above where the cut started, kept inside the canvas
			//and below the two lines at the top:
			set_text(message_line, message);
			float width = width_in_pixels(message_line);
			glm::vec2 at = canvas.world_to_pixel(slice_a) + glm::vec2(-0.5f * width, 1.5f * line_height);
			at.x = std::clamp(at.x, margin, std::max(margin, PixelCanvas::Width - margin - width));
			at.y = std::clamp(at.y, 5.5f * line_height, PixelCanvas::Height - 4.8f * line_height);
			//fade out over the last half second:
			float alpha = captured ? 1.0f : std::min(1.0f, message_time / 0.5f);
			draw_line(message_line, at, message_good ? Green : Orange, alpha);
		}

		static char const *const help[4] = {
			"Cut ACROSS the blue lifeline.",
			"WASD moves, mouse aims, Space cuts.",
			"Try exploring the room if it's too hard!",
			"Esc steps back into the ward.",
		};
		uint32_t help_count = captured ? 2 : 4;
		for (uint32_t i = 0; i < help_count; ++i) {
			set_text(help_lines[i], help[i]);
			draw_line(help_lines[i], glm::vec2(margin, margin + (help_count - 1 - i) * 1.2f * line_height), White);
		}
	}

	GL_ERRORS();
}
