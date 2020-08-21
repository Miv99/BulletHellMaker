#pragma once
#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/BlendMode.hpp>

namespace Shaders {
	enum ID {
		BrightnessPass,
		DownSamplePass,
		GaussianBlurPass,
		AddPass,
	};
}

namespace sf {
	class RenderTarget;
	class RenderTexture;
	class Shader;
}

class PostEffect : sf::NonCopyable {
public:
	virtual	~PostEffect();
	virtual void apply(const sf::RenderTexture& input, sf::RenderTarget& output) = 0;

	static bool	isSupported();

	void setBlendMode(sf::BlendMode blendMode);

protected:
	void applyShader(const sf::Shader& shader, sf::RenderTarget& output, sf::BlendMode blendMode);

private:
	sf::BlendMode blendMode;
};
