#include "BlurEffect.h"
#include <SFML/Graphics/Sprite.hpp>

BlurEffect::BlurEffect() : mShaders(), mFirstPassTextures(), mSecondPassTextures() {
	mShaders.load(Shaders::DownSamplePass, "Shaders/Fullpass.vert", "Shaders/DownSample.frag");
	mShaders.load(Shaders::GaussianBlurPass, "Shaders/Fullpass.vert", "Shaders/GuassianBlur.frag");
}

void BlurEffect::apply(const sf::RenderTexture& input, sf::RenderTarget& output) {
	prepareTextures(input.getSize());

	downsample(input, mFirstPassTextures[0]);
	blurMultipass(mFirstPassTextures);

	downsample(mFirstPassTextures[0], mSecondPassTextures[0]);
	blurMultipass(mSecondPassTextures);

	sf::Sprite sprite(mSecondPassTextures[0].getTexture());
	sprite.setScale(4.0f, 4.0f);
	sf::RenderStates states;
	states.blendMode = blendMode;
	output.draw(sprite, states);
}

void BlurEffect::prepareTextures(sf::Vector2u size) {
	if (inputTextureSize != size) {
		mFirstPassTextures[0].create(size.x / 2, size.y / 2);
		mFirstPassTextures[0].setSmooth(true);
		mFirstPassTextures[1].create(size.x / 2, size.y / 2);
		mFirstPassTextures[1].setSmooth(true);

		mSecondPassTextures[0].create(size.x / 4, size.y / 4);
		mSecondPassTextures[0].setSmooth(true);
		mSecondPassTextures[1].create(size.x / 4, size.y / 4);
		mSecondPassTextures[1].setSmooth(true);
	}
	this->inputTextureSize = size;
}

void BlurEffect::blurMultipass(RenderTextureArray& renderTextures) {
	sf::Vector2u textureSize = renderTextures[0].getSize();

	for (std::size_t count = 0; count < 2; ++count) {
		blur(renderTextures[0], renderTextures[1], sf::Vector2f(0.f, 1.f / textureSize.y));
		blur(renderTextures[1], renderTextures[0], sf::Vector2f(1.f / textureSize.x, 0.f));
	}
}

void BlurEffect::blur(const sf::RenderTexture& input, sf::RenderTexture& output, sf::Vector2f offsetFactor) {
	sf::Shader& gaussianBlur = mShaders.get(Shaders::GaussianBlurPass);

	gaussianBlur.setUniform("source", input.getTexture());
	gaussianBlur.setUniform("offsetFactor", offsetFactor);
	applyShader(gaussianBlur, output, sf::BlendNone);
	output.display();
}

void BlurEffect::downsample(const sf::RenderTexture& input, sf::RenderTexture& output) {
	sf::Shader& downSampler = mShaders.get(Shaders::DownSamplePass);

	downSampler.setUniform("source", input.getTexture());
	downSampler.setUniform("sourceSize", sf::Vector2f(input.getSize()));
	applyShader(downSampler, output, sf::BlendNone);
	output.display();
}

void BlurEffect::setBlendMode(sf::BlendMode blendMode) {
	this->blendMode = blendMode;
}