#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>

#include <SFML\Graphics.hpp>

#include <LevelPack/Animation.h>
#include <LevelPack/TextMarshallable.h>
#include <DataStructs/LRUCache.h>

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

class SpriteData : public TextMarshallable {
public:
	SpriteData();
	SpriteData(std::string spriteName, ComparableIntRect area, int spriteWidth, int spriteHeight, int spriteOriginX, int spriteOriginY, sf::Color color = sf::Color(255, 255, 255, 255));

	std::string format() const override;
	void load(std::string formattedString) override;

	bool operator==(const SpriteData& other) const;
	inline std::string getSpriteName() const { return spriteName; }
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
	// Size that the sprite will be, without accounting for global scale; not to be confused with the area containing the sprite's texture in the image
	int spriteWidth;
	int spriteHeight;
	// Local position of the sprite's origin
	int spriteOriginX;
	int spriteOriginY;
};

class AnimationData : public TextMarshallable {
public:
	AnimationData();
	AnimationData(std::string animationName, std::vector<std::pair<float, std::string>> spriteNames);

	std::string format() const override;
	void load(std::string formattedString) override;

	bool operator==(const AnimationData& other) const;
	inline std::string getAnimationName() const { return animationName; }
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

class SpriteSheet : public TextMarshallable {
public:
	SpriteSheet();
	SpriteSheet(std::string name);

	std::string format() const override;
	void load(std::string formattedString) override;

	/*
	Returns nullptr if the requested sprite does not exist.
	*/
	std::shared_ptr<sf::Sprite> getSprite(const std::string& spriteName);
	/*
	Returns nullptr if the requested animation does not exist.
	*/
	std::unique_ptr<Animation> getAnimation(const std::string& animationName, bool loop);
	void insertSprite(const std::string&, std::shared_ptr<SpriteData>);
	void insertAnimation(const std::string&, std::shared_ptr<AnimationData>);
	bool loadTexture(const std::string& spriteSheetFilePath);
	// Scale all sprites by the same amount
	void setGlobalSpriteScale(float scale);

	inline const std::map<std::string, std::shared_ptr<SpriteData>> getSpriteData() { return spriteData; }
	inline const std::map<std::string, std::shared_ptr<AnimationData>> getAnimationData() { return animationData; }

private:
	// Name of the sprite sheet
	std::string name;
	// Maps a sprite name to SpriteData
	std::map<std::string, std::shared_ptr<SpriteData>> spriteData;
	// Maps an animation name to AnimationData
	std::map<std::string, std::shared_ptr<AnimationData>> animationData;

	// ----------- Attributes below this line are not saved in TextMarshallable format() or loaded in load() -----------------------

	// The entire sprite sheet's texture
	sf::Texture texture;
	// Maps an animation name to a list of pairs of sprites and for how long each sprite appears for
	std::map<std::string, std::vector<std::pair<float, std::shared_ptr<sf::Sprite>>>> animationSprites;

	float globalSpriteScale = 1.0f;
};

/*
Note that if the SpriteLoader object goes out of scope, all Sprites loaded from it will not display correctly.
*/
class SpriteLoader {
public:
	SpriteLoader(const std::string& levelPackName);

	/*
	Saves all currently loaded sprite sheets' metadata into the sprite sheets folder
	of the level pack with the name passed in from SpriteLoader's constructor.

	Since only successfully loaded sprite sheets will be saved, there is no danger
	of saving a corrupted sprite sheet as a result of an unsuccessful load.
	*/
	void saveMetadataFiles();

	/*
	Loads all sprite sheets that have a corresponding metafile from the sprite sheets
	folder of the level pack with the name passed in from SpriteLoader's constructor.
	*/
	void loadFromSpriteSheetsFolder();

	/*
	Returns whether both the image and its metafile were successfully loaded.
	*/
	bool loadSpriteSheet(const std::string& spriteSheetMetaFileName, const std::string& spriteSheetImageFileName);

	/*
	Returns default missing texture if the GUI element could not be loaded.
	*/
	std::shared_ptr<sf::Texture> getGuiElementTexture(const std::string& guiElementFileName);
	/*
	Returns an entirely new sf::Sprite.
	*/
	std::shared_ptr<sf::Sprite> getSprite(const std::string& spriteName, const std::string& spriteSheetName);
	/*
	Returns nullptr if the requested animation does not exist.
	*/
	std::unique_ptr<Animation> getAnimation(const std::string& animationName, const std::string& spriteSheetName, bool loop);
	/*
	Returns default missing texture if the background could not be loaded.
	*/
	std::shared_ptr<sf::Texture> getBackground(const std::string& backgroundFileName);
	inline const std::map<std::string, std::shared_ptr<SpriteSheet>> getSpriteSheets() { return spriteSheets; }
	/*
	Returns a copy of the default missing sprite.
	*/
	const std::shared_ptr<sf::Sprite> getMissingSprite();
	void clearSpriteSheets();
	// Scale all sprites by the same amount
	void setGlobalSpriteScale(float scale);
	std::vector<std::string> getLoadedSpriteSheetNames();

private:
	const static std::size_t BACKGROUNDS_CACHE_MAX_SIZE;
	const static std::size_t GUI_ELEMENTS_CACHE_MAX_SIZE;

	std::string levelPackName;
	// Maps SpriteSheet name (as specified in the meta file) to SpriteSheet
	std::map<std::string, std::shared_ptr<SpriteSheet>> spriteSheets;
	// Cache of backgrounds; key is a pair of the background file name and value is a pair of the texture and the file's last modified time
	std::unique_ptr<Cache<std::string, std::pair<std::shared_ptr<sf::Texture>, std::filesystem::file_time_type>>> backgroundsCache;
	// Cache of GUI elements; key is a pair of the GUI element file name and value is a pair of the texture and the file's last modified time
	std::unique_ptr<Cache<std::string, std::pair<std::shared_ptr<sf::Texture>, std::filesystem::file_time_type>>> guiElementsCache;

	std::shared_ptr<sf::Texture> missingSpriteTexture;
	std::shared_ptr<sf::Sprite> missingSprite;

	float globalSpriteScale = 1.0f;
};