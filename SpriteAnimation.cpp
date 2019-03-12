#include "SpriteAnimation.h"

void FlashWhiteSA::update(float deltaTime) {
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
		flashIntensity = std::min(-2.0f * (t / flashDuration - 0.5f) + 1, -0.3f * (t / flashDuration) + 1);
	}
	shader.setUniform("flashColor", sf::Glsl::Vec4(1, 1, 1, flashIntensity));
}
