#pragma once
#include <SFML\Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Animation.h"

/*
The only purpose of this class is to have an IntRect that can be used as a key for maps
*/
class ComparableIntRect : public sf::IntRect {
public:
	// Non-meaningful operator required for map
	inline bool operator<(const ComparableIntRect& rhs) const {
		return this->left == rhs.left ? this->top < rhs.top : this->left < rhs.left;
	}
};

class SpriteData {
public:
	inline SpriteData(std::string spriteName, ComparableIntRect area, int spriteWidth, int spriteHeight, int spriteOriginX, int spriteOriginY, sf::Color color = sf::Color(255, 255, 255, 255)) : spriteName(spriteName), area(area), color(color), spriteWidth(spriteWidth), spriteHeight(spriteHeight), spriteOriginX(spriteOriginX), spriteOriginY(spriteOriginY) {}

	bool operator==(const SpriteData& other) const;
	inline ComparableIntRect getArea() const { return area; }
	inline sf::Color getColor() const { return color; }
	inline int getSpriteWidth() const { return spriteWidth; }
	inline int getSpriteHeight() const { return spriteHeight; }
	inline int getSpriteOriginX() const { return spriteOriginX; }
	inline int getSpriteOriginY() const { return spriteOriginY; }

private:
	std::string spriteName;
	ComparableIntRect area;
	sf::Color color;
	// Size that the sprite will be; not to be confused with the area containing the sprite's texture in the image
	int spriteWidth;
	int spriteHeight;
	// Local position of the sprite's origin
	int spriteOriginX;
	int spriteOriginY;
};

class AnimationData {
public:
	inline AnimationData(std::string animationName, std::vector<std::pair<float, std::string>> spriteNames) : animationName(animationName), spriteNames(spriteNames) {}

	bool operator==(const AnimationData& other) const;
	inline const std::vector<std::pair<float, std::string>> getSpriteNames() const { return spriteNames; }
	inline float getTotalTime() {
		float total = 0;
		for (auto p : spriteNames) {
			total += p.first;
		}
		return total;
	}

private:
	std::string animationName;
	std::vector<std::pair<float, std::string>> spriteNames;
};

class SpriteSheet {
public:
	inline SpriteSheet(std::string name) : name(name) {}
	std::shared_ptr<sf::Sprite> getSprite(const std::string& spriteName);
	std::unique_ptr<Animation> getAnimation(const std::string& animationName, bool loop);
	void insertSprite(const std::string&, std::shared_ptr<SpriteData>);
	void insertAnimation(const std::string&, std::shared_ptr<AnimationData>);
	bool loadImage(const std::string& imageFileName);
	void preloadTextures();

	inline const std::map<std::string, std::shared_ptr<SpriteData>> getSpriteData() { return spriteData; }
	inline const std::map<std::string, std::shared_ptr<AnimationData>> getAnimationData() { return animationData; }

private:
	// Name of the sprite sheet
	std::string name;
	std::shared_ptr<sf::Image> image;
	// Maps an area on the image to a Texture
	std::map<ComparableIntRect, std::shared_ptr<sf::Texture>> textures;
	// Maps a sprite name to SpriteData
	std::map<std::string, std::shared_ptr<SpriteData>> spriteData;
	// Maps an animation name to AnimationData
	std::map<std::string, std::shared_ptr<AnimationData>> animationData;
	// Maps an animation name to a list of pairs of sprites and for how long each sprite appears for
	std::map<std::string, std::vector<std::pair<float, std::shared_ptr<sf::Sprite>>>> animationSprites;
};

class SpriteLoader {
public:
	// spriteSheetNames - vector of pairs of SpriteSheet meta file names and SpriteSheet image file names
	SpriteLoader(const std::string& levelPackRelativePath, const std::vector<std::pair<std::string, std::string>>& spriteSheetNamePairs);

	std::shared_ptr<sf::Sprite> getSprite(const std::string& spriteName, const std::string& spriteSheetName);
	std::unique_ptr<Animation> getAnimation(const std::string& animationName, const std::string& spriteSheetName, bool loop);
	inline const std::map<std::string, std::shared_ptr<SpriteSheet>> getSpriteSheets() { return spriteSheets; }
	void preloadTextures();
	void clearSpriteSheets();

private:
	// Relative path to the level path containing the files
	std::string levelPackRelativePath;
	// Maps SpriteSheet name (as specified in the meta file) to SpriteSheet
	std::map<std::string, std::shared_ptr<SpriteSheet>> spriteSheets;
	// Returns true if the meta file and image file were successfully loaded
	bool loadSpriteSheet(const std::string& spriteSheetMetaFileName, const std::string& spriteSheetImageFileName);
};