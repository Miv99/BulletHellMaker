#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <utility>
#include <vector>

class Animation {
public:
	inline Animation(std::string name, std::vector<std::pair<float, std::shared_ptr<sf::Sprite>>> sprites, bool loops) : name(name), sprites(sprites), looping(loops) {}

	/*
	Returns a pointer to the current sprite.
	If the animation is finished, nullptr is returned.
	*/
	std::shared_ptr<sf::Sprite> update(float deltaTime);

protected:
	// The name of the animation
	std::string name;
	// Sprite names and for what amount of seconds that sprite will be used
	std::vector<std::pair<float, std::shared_ptr<sf::Sprite>>> sprites;
	int currentSpriteIndex = 0;

	// True if the animation loops
	bool looping;

	// True if animation is finished (can only be true if looping is false)
	bool done = false;

	// Time since the last sprite change
	float time = 0;
};