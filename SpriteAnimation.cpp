#include "SpriteAnimation.h"
#include <algorithm>

void FlashWhiteSEA::update(float deltaTime) {
	if (done) {
		return;
	}

	time += deltaTime;
	if (time >= animationDuration) {
		done = true;
		useShader = false;
		return;
	}

	// flashIntensity must be in range [0, 1]
	float flashIntensity;
	// Time since start of flash
	float t = std::fmod(time, flashInterval + flashDuration);
	if (t > flashDuration) {
		// Is not flashing
		flashIntensity = 0;
	}
	else {
		// Is flashing
		// Scale flash intensity to have max of 0.7
		//TODO: put this as a constant somewhere
		flashIntensity = 0.7f * std::min(-2.0f * (t / flashDuration - 0.5f) + 1, -0.3f * (t / flashDuration) + 1);
	}
	shader.setUniform("flashColor", sf::Glsl::Vec4(1, 1, 1, flashIntensity));
}

void FadeAwaySEA::update(float deltaTime) {
	if (time > animationDuration) {
		return;
	}

	time += deltaTime;
	auto color = sprite->getColor();
	sprite->setColor(sf::Color(color.r, color.g, color.b, std::max(minOpacity * 255.0f, 255.0f * (-(maxOpacity - minOpacity) / animationDuration * time + maxOpacity))));
}
