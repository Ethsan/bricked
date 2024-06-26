#pragma once

#include "exception.h"

#include <array>
#include <fstream>
#include <iostream>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

class Paddle {
    public:
	enum dir {
		left,
		right,
		none,
	};

	Paddle(float x, float y)
		: x(x)
		, y(y)
	{
	}
	float get_x() const
	{
		return x;
	}
	float get_y() const
	{
		return y;
	}

	dir get_dir() const
	{
		return direction;
	}

	friend class Logic;
	static constexpr float w = 56, h = 30;

    private:
	float x, y;
	dir direction = none;
};

class Ball {
	float x, y;
	float vx, vy;
	bool alive;

    public:
	Ball(float x, float y, float vx, float vy)
		: x(x)
		, y(y)
		, vx(vx)
		, vy(vy)
		, alive(true)

	{
	}
	float get_x() const
	{
		return x;
	}
	float get_y() const
	{
		return y;
	}
	float get_vx() const
	{
		return vx;
	}
	float get_vy() const
	{
		return vy;
	}
	bool is_alive() const
	{
		return alive;
	}

	friend class Logic;
	static constexpr float r = 8;
};

class Powerup {
    public:
	enum type {
		slow_ball,
		fast_ball,
		extra_ball,
		extra_life,
		small_ball,
		big_small,
		strong_ball,
	};

	Powerup(float x, float y, type t)
		: x(x)
		, y(y)
		, power(t)
		, alive(true)

	{
	}
	float get_x() const
	{
		return x;
	}
	float get_y() const
	{
		return y;
	}

	bool is_alive() const
	{
		return alive;
	}

	enum type get_power() const
	{
		return power;
	}

	static constexpr float r = 8;
	friend class Logic;

    private:
	float x, y;
	type power;
	bool alive;
};

class Brick {
    public:
	enum Shape { rect, hex };

	Brick(float x, float y, Shape shape, uint dura = 1, std::optional<Powerup::type> powerup = std::nullopt)
		: x(x)
		, y(y)
		, dura(dura)
		, powerup(powerup)
		, shape(shape)
	{
	}

	float get_x() const
	{
		return x;
	}
	float get_y() const
	{
		return y;
	}
	uint get_durability() const
	{
		return dura;
	}
	int get_last_hit() const
	{
		return last_hit;
	}
	std::optional<Powerup::type> get_powerup() const
	{
		return powerup;
	}

	Shape get_form() const
	{
		return shape;
	}

	static constexpr float rect_w = 48, rect_h = 16;
	static constexpr std::array<std::pair<float, float>, 4> rect_points = {
		std::make_pair(-rect_w / 2, -rect_h / 2),
		std::make_pair(rect_w / 2, -rect_h / 2),
		std::make_pair(rect_w / 2, rect_h / 2),
		std::make_pair(-rect_w / 2, rect_h / 2),
	};

	static constexpr float hex_r = 16;
	static constexpr std::array<std::pair<float, float>, 6> hex_points = {
		std::make_pair(0, -hex_r),
		std::make_pair(hex_r * 0.866, -hex_r / 2),
		std::make_pair(hex_r * 0.866, hex_r / 2),
		std::make_pair(0, hex_r),
		std::make_pair(-hex_r * 0.866, hex_r / 2),
		std::make_pair(-hex_r * 0.866, -hex_r / 2),
	};

	std::vector<std::pair<float, float> > get_points() const
	{
		switch (shape) {
		case rect: {
			std::vector<std::pair<float, float> > res;
			res.reserve(rect_points.size());
			for (auto &p : rect_points) {
				res.push_back(std::make_pair(x + p.first, y + p.second));
			}
			return res;
		}
		case hex: {
			std::vector<std::pair<float, float> > res;
			res.reserve(hex_points.size());
			for (auto &p : hex_points) {
				res.push_back(std::make_pair(x + p.first, y + p.second));
			}
			return res;
		}
		}
		return {};
	}
	friend class Logic;

    private:
	float x, y;
	uint dura;
	int last_hit = -1;
	std::optional<Powerup::type> powerup;
	Shape shape;
};

class Logic {
    public:
	enum GameState {
		RUNNING,
		WIN,
		LOST,
	};

	Logic(float width, float height, bool default_stage = false)
		: w(width)
		, h(height)
	{
		if (default_stage)
			init();
	}

	static Logic load(const std::string &save_file)
	{
		std::ifstream save_import(save_file, std::ios::in);
		if (!save_import.is_open())
			throw Bad_format();
		Logic l(load(save_import));
		save_import.close();
		return l;
	}

	static Logic load(std::istream &save);

	void step(float dt);

	void set_paddle_dir(Paddle::dir d)
	{
		paddle.direction = d;
	}

	float get_width() const
	{
		return w;
	}

	float get_height() const
	{
		return h;
	}

	int get_tick() const
	{
		return tick;
	}

	int get_score() const
	{
		return score;
	}

	float get_speed() const
	{
		return base_speed + bonus_speed + bounce_count * 1;
	}

	int get_lives() const
	{
		return lives;
	}

	int get_ball_count() const
	{
		return ball_count;
	}

	template <typename T> void visit(T &&visitor)
	{
		for (auto &ball : balls) {
			visitor(ball);
		}
		for (auto &brick : bricks) {
			visitor(brick);
		}

		for (auto &powerup : powerups) {
			visitor(powerup);
		}
		visitor(paddle);
	}

	void launch_ball();

	Paddle &get_paddle()
	{
		return paddle;
	}

	Brick &get_brick(std::size_t index)
	{
		return bricks.at(index);
	}

	std::optional<std::pair<std::size_t, Brick &> > get_brick(float x, float y);

	std::optional<std::size_t> add_brick_safe(float x, float y, uint durability);

	void replace_brick_safe(std::size_t index, float x, float y);

	void remove_brick(std::size_t index);

	GameState get_state() const
	{
		return state;
	}

	void save(std::ostream &output);

    private:
	float w, h;

	GameState state = RUNNING;

	std::vector<Ball> balls{};
	std::vector<Brick> bricks{};
	std::vector<Powerup> powerups{};

	Paddle paddle{ w / 2, h - Paddle::h };

	int brick_count = 0;
	int ball_count = 0;
	int tick = 0;

	int score = 0;
	int combo = 0;
	const int brick_points = 100;

	float bonus_speed = 0;
	int bounce_count = 0;
	const float base_speed = w / 2;

	const float paddle_speed = w / 2;

	int lives = 3;

	int add_brick(float x, float y, Brick::Shape shape, uint durability = 1,
		      std::optional<Powerup::type> type = std::nullopt);

	int add_powerup(float x, float y, Powerup::type type);
	int add_ball(float x, float y, float vx = 0, float vy = 1);

	template <typename T> void move(T &obj, float dt);

	template <typename T> void collide(Ball &ball, T &object);

	void init();
};
