#pragma once
#include <array>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <Game/Systems/RenderSystem/PostEffect.h>
#include <Game/Systems/RenderSystem/ResourceHolder.h>

class BlurEffect : public PostEffect {
public:
	BlurEffect();

	virtual void apply(const sf::RenderTexture& input, sf::RenderTarget& output);

	void setBlendMode(sf::BlendMode blendMode);

private:
	typedef std::array<sf::RenderTexture, 2> RenderTextureArray;

	sf::BlendMode blendMode;
	sf::Vector2u inputTextureSize;

	void prepareTextures(sf::Vector2u size);

	void blurMultipass(RenderTextureArray& renderTextures);
	void blur(const sf::RenderTexture& input, sf::RenderTexture& output, sf::Vector2f offsetFactor);
	void downsample(const sf::RenderTexture& input, sf::RenderTexture& output);

	ResourceHolder<sf::Shader, Shaders::ID> mShaders;

	RenderTextureArray	mFirstPassTextures;
	RenderTextureArray	mSecondPassTextures;
};

