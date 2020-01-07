#pragma once
#include "Components.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <utility>
#include <vector>
#include <functional>
#include <memory>
#include "TextMarshallable.h"
#include "SpriteLoader.h"

class Level;

// Default blend mode
static sf::BlendMode DEFAULT_BLEND_MODE = sf::BlendMode(sf::BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, sf::BlendMode::Equation::Add, sf::BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, sf::BlendMode::Equation::Add);

class BloomSettings : public TextMarshallable {
public:
	inline BloomSettings() {}
	// For debug only
	inline BloomSettings(float glowStrength, float minBright, sf::BlendMode blendMode = DEFAULT_BLEND_MODE) : useBloom(true), glowStrength(glowStrength), minBright(minBright), blendMode(blendMode) {}

	std::string format() const override;
	void load(std::string formattedString);

	inline bool usesBloom() { return useBloom; }
	inline float getGlowStrength() { return glowStrength; }
	inline float getMinBright() { return minBright; }
	inline sf::BlendMode getBlendMode() { return blendMode; }

	inline void setUsesBloom(bool useBloom) { this->useBloom = useBloom; }
	inline float setGlowStrength(float glowStrength) { this->glowStrength = glowStrength; }
	inline float setMinBright(float minBright) { this->minBright = minBright; }
	inline sf::BlendMode setBlendMode(sf::BlendMode blendMode) { this->blendMode = blendMode; }

private:
	bool useBloom = false;

	// [1, inf]; determines how much the sprite and its "bloom shadow" glows
	float glowStrength;
	// [0, 1]; determines how much to darken only the sprite
	float minBright;
	// blend mode used when drawing the unblurred texture; saved as an int
	sf::BlendMode blendMode = DEFAULT_BLEND_MODE;
};

/*
Shaders are implemented here with the assumption that there will never be user-customizable shaders that can
be unique to a sprite.
*/
class RenderSystem {
public:
	/*
	Window's view should already be set and should not be changed.

	background - the background of the map
	*/
	RenderSystem(entt::DefaultRegistry& registry, sf::RenderWindow& window, SpriteLoader& spriteLoader, float resolutionMultiplier = 1.0f, bool initShaders = true);
	virtual void update(float deltaTime);

	/*
	Load bloom shaders to match a level's bloom settings.
	If level is a nullptr, no bloom settings are used.
	*/
	void loadLevelRenderSettings(std::shared_ptr<Level> level);

	/*
	Sets the resolution of the game.
	This doesn't change the size of the window. It only affects the gameplay quality.
	Resolutions that are too small won't work. 1600x900, 1024x768, and anything higher will work.
	*/
	virtual void setResolution(SpriteLoader& spriteLoader, float resolutionMultiplier);
	
	void setBackground(sf::Texture background);
	inline void setBackgroundScrollSpeedX(float backgroundScrollSpeedX) { this->backgroundScrollSpeedX = backgroundScrollSpeedX; }
	inline void setBackgroundScrollSpeedY(float backgroundScrollSpeedY) { this->backgroundScrollSpeedY = backgroundScrollSpeedY; }
	inline void setBackgroundTextureWidth(float backgroundTextureWidth) {
		this->backgroundTextureWidth = backgroundTextureWidth;
		backgroundSprite.setScale(MAP_WIDTH / backgroundTextureWidth * resolutionMultiplier, MAP_HEIGHT / backgroundTextureHeight * resolutionMultiplier);
	}
	inline void setBackgroundTextureHeight(float backgroundTextureHeight) {
		this->backgroundTextureHeight = backgroundTextureHeight;
		backgroundSprite.setScale(MAP_WIDTH / backgroundTextureWidth * resolutionMultiplier, MAP_HEIGHT / backgroundTextureHeight * resolutionMultiplier);
	}

	sf::Vector2u getResolution();
	std::shared_ptr<entt::SigH<void()>> getOnResolutionChange();

protected:
	entt::DefaultRegistry& registry;
	sf::RenderWindow& window;

	struct SubLayerComparator {
		int operator()(const SpriteComponent& a, const SpriteComponent& b) {
			return a.getSubLayer() < b.getSubLayer();
		}
	};

	// Pairs of layer and entities in that layer
	// Entites in the same layer are sorted by sublayer
	std::vector<std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>> layers;
	// Maps layer to the texture, onto which all sprites in a layer on drawn
	// All textures are the same size
	std::map<int, sf::RenderTexture> layerTextures;
	// Maps layer to the global shaders applied on that layer
	std::map<int, std::vector<std::unique_ptr<sf::Shader>>> globalShaders;

	// Bloom settings for each layer
	std::vector<BloomSettings> bloom;
	// Shaders used to blur layers
	std::vector<std::unique_ptr<sf::Shader>> bloomBlurShaders;
	// Shader used to brighten textures after blurring them
	sf::Shader bloomGlowShader;
	// Shader used to darken textures before blurring them
	sf::Shader bloomDarkShader;

	// Temporary layer texture for using multiple shaders
	sf::RenderTexture tempLayerTexture;
	sf::RenderTexture tempLayerTexture2;
	// Temporary layer texture just for the background
	sf::RenderTexture backgroundTempLayerTexture;

	// Black magic needed to scale texture conversion into sprites correctly since views do not match the texture size
	// I actually don't know why this is needed only when using 2 or more global shaders on a texture
	float spriteHorizontalScale;
	float spriteVerticalScale;

	float resolutionMultiplier = 1.0f;

	sf::Texture background;
	// The background as a sprite
	sf::Sprite backgroundSprite;
	float backgroundScrollSpeedX, backgroundScrollSpeedY;
	float backgroundTextureWidth, backgroundTextureHeight;
	// Current position of top-left corner of the screen relative to the top-left corner of the background
	float backgroundX = 0, backgroundY = 0;
	float backgroundTextureSizeX, backgroundTextureSizeY;

	std::shared_ptr<entt::SigH<void()>> onResolutionChange;
};