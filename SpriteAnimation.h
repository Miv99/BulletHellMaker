#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <cmath>
#include <algorithm>

/*
Base class for modifying a sf::Sprite over time
SA for short.

See RenderSystem::loadShaders for the list of shaders that can be used.
*/
class SpriteAnimation {
public:
	inline SpriteAnimation(std::shared_ptr<sf::Sprite> sprite) : sprite(sprite) {}
	virtual void update(float deltaTime) = 0;

	bool usesShader() { return useShader; }
	sf::Shader& getShader() { return shader; }

protected:
	// Reference to the sprite being modified
	std::shared_ptr<sf::Sprite> sprite;
	// Shader to be used when drawing the sprite, if any
	sf::Shader shader;
	// Whether or not to use the shader
	bool useShader;

	// Time since start of the animation
	float time = 0;
};

/*
Causes a sprite to flash white.
The first flash will start as soon as the animation starts.
Sprite goes back to how it was originally after the animation.
*/
class FlashWhiteSA : public SpriteAnimation {
public:
	/*
	flashInterval - time after the end of a flash that the next one begins
	flashDuration - time that the sprite will be white in each flash
	animationDuration - total time the sprite will be flashing

	The total interval between each entire flash animation is (flashInterval + flashDuration)
	*/
	inline FlashWhiteSA(std::shared_ptr<sf::Sprite> sprite, float flashInterval, float flashDuration, float animationDuration) : SpriteAnimation(sprite),
		flashInterval(flashInterval), flashDuration(flashDuration), animationDuration(animationDuration) {
		// Load shader
		shader.loadFromFile("Shaders/tint.frag", sf::Shader::Fragment);
		shader.setUniform("flashColor", sf::Glsl::Vec4(1, 1, 1, 0));
		useShader = true;
	}

	void update(float deltaTime) override;

private:
	float flashInterval;
	float flashDuration;
	float animationDuration;
	bool done = false;
};