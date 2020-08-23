#pragma once
#include <utility>
#include <vector>
#include <functional>
#include <memory>

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include <LevelPack/TextMarshallable.h>
#include <DataStructs/SpriteLoader.h>
#include <Game/Components/Components.h>
#include <Game/Systems/RenderSystem/BlurEffect.h>

class Level;

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
	Sets the resolution of the game.
	This doesn't change the size of the window. It only affects the gameplay quality.
	Resolutions that are too small won't work. 1600x900, 1024x768, and anything higher will work.
	*/
	virtual void setResolution(SpriteLoader& spriteLoader, float resolutionMultiplier);
	
	void setBackground(std::shared_ptr<sf::Texture> background);
	inline void setBackgroundScrollSpeedX(float backgroundScrollSpeedX) { this->backgroundScrollSpeedX = backgroundScrollSpeedX; }
	inline void setBackgroundScrollSpeedY(float backgroundScrollSpeedY) { this->backgroundScrollSpeedY = backgroundScrollSpeedY; }
	void setBackgroundTextureWidth(float backgroundTextureWidth);
	void setBackgroundTextureHeight(float backgroundTextureHeight);

	sf::Vector2u getResolution();
	std::shared_ptr<entt::SigH<void()>> getOnResolutionChange();

protected:
	// Default blend mode
	const static sf::BlendMode DEFAULT_BLEND_MODE;

	entt::DefaultRegistry& registry;
	sf::RenderWindow& window;

	struct SubLayerComparator {
		int operator()(const SpriteComponent& a, const SpriteComponent& b) {
			return a.getSubLayer() < b.getSubLayer();
		}
	};

	// Pairs of layer and entities in that layer
	// Entites in the same layer are sorted by sublayer
	std::vector<std::vector<std::reference_wrapper<SpriteComponent>>> layers;
	// Maps layer to the texture, onto which all sprites in a layer on drawn
	// All textures are the same size
	std::map<int, sf::RenderTexture> layerTextures;

	BlurEffect blurEffect;

	// Temporary layer texture just for the background
	sf::RenderTexture backgroundTempLayerTexture;

	float resolutionMultiplier = 1.0f;

	std::shared_ptr<sf::Texture> background;
	// The background as a sprite
	sf::Sprite backgroundSprite;
	float backgroundScrollSpeedX, backgroundScrollSpeedY;
	float backgroundTextureWidth, backgroundTextureHeight;
	// Current position of top-left corner of the screen relative to the top-left corner of the background
	float backgroundX = 0, backgroundY = 0;
	float backgroundTextureSizeX, backgroundTextureSizeY;

	std::shared_ptr<entt::SigH<void()>> onResolutionChange;
};