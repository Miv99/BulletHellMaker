#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <cmath>
#include <algorithm>

/*
Base class for modifying a sf::Sprite over time.
SEA for short.

See RenderSystem::loadShaders for the list of shaders that can be used.
*/
class SpriteEffectAnimation {
public:
	inline SpriteEffectAnimation(std::shared_ptr<sf::Sprite> sprite) : sprite(sprite) {}
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
class FlashWhiteSEA : public SpriteEffectAnimation {
public:
	/*
	flashInterval - time after the end of a flash that the next one begins
	flashDuration - time that the sprite will be white in each flash
	animationDuration - total time the sprite will be flashing

	The total interval between each entire flash animation is (flashInterval + flashDuration)
	*/
	inline FlashWhiteSEA(std::shared_ptr<sf::Sprite> sprite, float flashInterval, float flashDuration, float animationDuration) : SpriteEffectAnimation(sprite),
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

/*
Causes a sprite to fade away (lowers opacity).
*/
class FadeAwaySEA : public SpriteEffectAnimation {
public:
	/*
	minOpacity - minimum opacity in range [0, 1]
	maxOpacity - maximum opacity in range [0, 1]
	animationDuration - total time for the sprite to fade from maxOpacity to minOpacity
	*/
	inline FadeAwaySEA(std::shared_ptr<sf::Sprite> sprite, float minOpacity, float maxOpacity, float animationDuration) : SpriteEffectAnimation(sprite), minOpacity(minOpacity), 
		maxOpacity(maxOpacity), animationDuration(animationDuration) {
		useShader = false;
	}

	void update(float deltaTime) override;

private:
	float minOpacity;
	float maxOpacity;
	float animationDuration;
};