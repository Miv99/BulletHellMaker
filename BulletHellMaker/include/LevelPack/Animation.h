#pragma once
#include <memory>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

class Animation {
public:
	Animation(std::string name, std::vector<std::pair<float, std::shared_ptr<sf::Sprite>>> sprites, bool loops);

	/*
	Returns a pointer to the current sprite.
	If the animation is finished, nullptr is returned.
	*/
	std::shared_ptr<sf::Sprite> update(float deltaTime);

	inline bool isDone() { return done; }
	inline float getTotalDuration() { return totalDuration; }

protected:
	// The name of the animation
	std::string name;
	// Sprite names and for what amount of seconds that sprite will be used
	std::vector<std::pair<float, std::shared_ptr<sf::Sprite>>> sprites;
	int currentSpriteIndex = 0;

	// Total duration of the animation, not including loops
	float totalDuration = 0;

	// True if the animation loops
	bool looping = true;

	// True if animation is finished (can only be true if looping is false)
	bool done = false;

	// Time since the last sprite change
	float time = 0;
};