#pragma once
#include <memory>
#include <string>
#include <cmath>
#include <algorithm>

#include <SFML/Graphics.hpp>

/*
Base class for modifying a sf::Sprite over time.
SEA for short.

See RenderSystem::loadShaders for the list of shaders that can be used.
*/
class SpriteEffectAnimation {
public:
	/*
	sprite - the pointer to the sprite that will be modified
	*/
	SpriteEffectAnimation(std::shared_ptr<sf::Sprite> sprite);
	virtual void update(float deltaTime) = 0;

	bool usesShader() { return useShader; }
	sf::Shader& getShader() { return shader; }
	void setSpritePointer(std::shared_ptr<sf::Sprite> sprite) { this->sprite = sprite; }

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
	FlashWhiteSEA(std::shared_ptr<sf::Sprite> sprite, float animationDuration, float flashInterval = 0.3f, float flashDuration = 0.2f);

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
	keepEffectAfterEnding - if true, the sprite will maintain minOpacity even after the effect ends
	*/
	FadeAwaySEA(std::shared_ptr<sf::Sprite> sprite, float minOpacity, float maxOpacity, float animationDuration, bool keepEffectAfterEnding = false);

	void update(float deltaTime) override;

private:
	float minOpacity;
	float maxOpacity;
	float animationDuration;
	bool keepEffectAfterEnding;
};

/*
Causes a sprite to change in size while maintaining aspect ratio.
Warning: this changes the size of only the sprite, not the hitbox or anything else.
*/
class ChangeSizeSEA : public SpriteEffectAnimation {
public:
	/*
	startScale - starting sprite scale with a value of 1 being original size
	endScale - ending sprite scale
	animationDuration - total time for the sprite to fade from maxOpacity to minOpacity
	*/
	ChangeSizeSEA(std::shared_ptr<sf::Sprite> sprite, float startScale, float endScale, float animationDuration);

	void update(float deltaTime) override;

private:
	float startScale;
	float endScale;
	float animationDuration;
};