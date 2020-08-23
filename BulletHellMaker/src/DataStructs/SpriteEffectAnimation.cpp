#include <DataStructs/SpriteEffectAnimation.h>

#include <algorithm>

SpriteEffectAnimation::SpriteEffectAnimation(std::shared_ptr<sf::Sprite> sprite)
	: sprite(sprite) {
}

FlashWhiteSEA::FlashWhiteSEA(std::shared_ptr<sf::Sprite> sprite, float animationDuration, float flashInterval, float flashDuration) 
	: SpriteEffectAnimation(sprite), flashInterval(flashInterval), flashDuration(flashDuration), animationDuration(animationDuration) {
	// Load shader
	if (!shader.loadFromFile("Shaders/tint.frag", sf::Shader::Fragment)) {
		throw "Could not load Shaders/tint.frag";
	}
	shader.setUniform("flashColor", sf::Glsl::Vec4(1, 1, 1, 0));
	useShader = true;
}

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
	} else {
		// Is flashing
		// Scale flash intensity to have max of 0.7
		//TODO: put this as a constant somewhere
		flashIntensity = 0.7f * std::min(-2.0f * (t / flashDuration - 0.5f) + 1, -0.3f * (t / flashDuration) + 1);
	}
	shader.setUniform("flashColor", sf::Glsl::Vec4(1, 1, 1, flashIntensity));
	shader.setUniform("textureModulatedColor", sf::Glsl::Vec4(sprite->getColor()));
}

FadeAwaySEA::FadeAwaySEA(std::shared_ptr<sf::Sprite> sprite, float minOpacity, float maxOpacity, float animationDuration, bool keepEffectAfterEnding) 
	: SpriteEffectAnimation(sprite), minOpacity(minOpacity), maxOpacity(maxOpacity), animationDuration(animationDuration), keepEffectAfterEnding(keepEffectAfterEnding) {
	useShader = false;
}

void FadeAwaySEA::update(float deltaTime) {
	if (time > animationDuration) {
		if (keepEffectAfterEnding) {
			auto color = sprite->getColor();
			sprite->setColor(sf::Color(color.r, color.g, color.b, minOpacity * 255.0f));
		}

		return;
	}

	time += deltaTime;
	auto color = sprite->getColor();
	sprite->setColor(sf::Color(color.r, color.g, color.b, std::max(minOpacity * 255.0f, 255.0f * (-(maxOpacity - minOpacity) / animationDuration * time + maxOpacity))));
}

ChangeSizeSEA::ChangeSizeSEA(std::shared_ptr<sf::Sprite> sprite, float startScale, float endScale, float animationDuration) 
	: SpriteEffectAnimation(sprite), startScale(startScale), endScale(endScale), animationDuration(animationDuration) {
	useShader = false;
}

void ChangeSizeSEA::update(float deltaTime) {
	if (time > animationDuration) {
		return;
	}

	time += deltaTime;
	float scale = (time / animationDuration)*(endScale - startScale) + startScale;
	sprite->setScale(scale, scale);
}
