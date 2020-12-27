#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
#include <set>

#include <SFML\Graphics.hpp>

#include <LevelPack/Animation.h>
#include <LevelPack/TextMarshallable.h>
#include <DataStructs/LRUCache.h>

/*
The only purpose of this class is to have an IntRect that can be used as a key for maps
*/
class ComparableIntRect : public sf::IntRect {
public:
	ComparableIntRect();
	ComparableIntRect(sf::IntRect rect);

	// Non-meaningful operator required for map
	inline bool operator<(const ComparableIntRect& rhs) const {
		return this->left == rhs.left ? this->top < rhs.top : this->left < rhs.left;
	}
};

class SpriteData : public TextMarshallable {
public:
	SpriteData();
	SpriteData(std::string spriteName, ComparableIntRect area, int spriteWidth, int spriteHeight, 
		int spriteOriginX, int spriteOriginY, sf::Color color = sf::Color(255, 255, 255, 255));
	/*
	Copy constructor.
	*/
	SpriteData(const SpriteData& other);
	/*
	Copy constructor.
	*/
	SpriteData(std::shared_ptr<SpriteData> other);

	std::string format() const override;
	void load(std::string formattedString) override;

	bool operator==(const SpriteData& other) const;
	inline std::string getSpriteName() const { return spriteName; }
	inline ComparableIntRect getArea() const { return area; }
	inline sf::Color getColor() const { return color; }
	inline int getSpriteWidth() const { return spriteWidth; }
	inline int getSpriteHeight() const { return spriteHeight; }
	inline float getSpriteOriginX() const { return spriteOriginX; }
	inline float getSpriteOriginY() const { return spriteOriginY; }

	void setSpriteName(std::string spriteName);
	void setTextureArea(sf::IntRect area);
	void setColor(sf::Color color);
	void setSpriteSize(int width, int height);
	void setSpriteOrigin(float x, float y);

private:
	std::string spriteName;
	ComparableIntRect area;
	sf::Color color;
	// Size that the sprite will be, without accounting for global scale; not to be confused with the area containing the sprite's texture in the image
	int spriteWidth;
	int spriteHeight;
	// Local position of the sprite's origin
	float spriteOriginX;
	float spriteOriginY;
};

class AnimationData : public TextMarshallable {
public:
	AnimationData();
	AnimationData(std::string animationName, std::vector<std::pair<float, std::string>> spriteNames);
	/*
	Copy constructor.
	*/
	AnimationData(const AnimationData& other);
	/*
	Copy constructor.
	*/
	AnimationData(std::shared_ptr<AnimationData> other);

	std::string format() const override;
	void load(std::string formattedString) override;

	bool operator==(const AnimationData& other) const;
	inline std::string getAnimationName() const { return animationName; }
	inline const std::vector<std::pair<float, std::string>> getSpriteInfo() const { return spriteNames; }
	inline std::pair<float, std::string> getSpriteInfo(int index) const { return spriteNames.at(index); }
	inline float getTotalTime() {
		float total = 0;
		for (auto p : spriteNames) {
			total += p.first;
		}
		return total;
	}
	inline int getNumSprites() {
		return spriteNames.size();
	}

	void setAnimationName(std::string animationName);
	void setSpriteNames(std::vector<std::pair<float, std::string>> spriteNames);
	/*
	Modifies an existing sprite entry.
	*/
	void setSpriteInfo(int index, std::pair<float, std::string> newInfo);
	/*
	Modifies an existing sprite entry.
	*/
	void setSpriteDuration(int index, float newDuration);
	/*
	Modifies an existing sprite entry.
	*/
	void setSpriteName(int index, std::string newName);

	/*
	Inserts a new sprite entry.
	*/
	void insertSpriteInfo(int index, std::pair<float, std::string> newInfo);
	void removeSpriteEntry(int index);

private:
	std::string animationName;
	// How long each sprite appears for, and the name of that sprite
	std::vector<std::pair<float, std::string>> spriteNames;
};

class SpriteSheet : public TextMarshallable {
public:
	SpriteSheet();
	SpriteSheet(std::string name);
	/*
	Copy constructor.
	Does not copy or reload the texture.
	*/
	SpriteSheet(const std::shared_ptr<SpriteSheet> other);

	std::string format() const override;
	void load(std::string formattedString) override;

	void insertSprite(const std::string& spriteName, std::shared_ptr<SpriteData> spriteData);
	void insertAnimation(const std::string& animationName, std::shared_ptr<AnimationData> animationData);

	void deleteSprite(const std::string& spriteName);
	void deleteAnimation(const std::string& animationName);

	bool loadTexture(const std::string& spriteSheetFilePath);

	/*
	Unloads an animation so that the next time it is fetched, changes in its AnimationData
	from before it was unloaded are visible.
	*/
	void unloadAnimation(const std::string& animationName);

	/*
	Scale all sprites by the same amount
	*/
	void setGlobalSpriteScale(float scale);

	void markFailedImageLoad();
	void markFailedMetafileLoad();

	/*
	Throws std::runtime_error.
	*/
	void renameSprite(std::string oldSpriteName, std::string newSpriteName);
	/*
	Throws std::runtime_error.
	*/
	void renameAnimation(std::string oldAnimationName, std::string newAnimationName);

	/*
	Returns nullptr if the requested sprite does not exist.
	*/
	std::shared_ptr<sf::Sprite> getSprite(const std::string& spriteName);
	/*
	Returns nullptr if the requested animation does not exist.
	*/
	std::unique_ptr<Animation> getAnimation(const std::string& animationName, bool loop);
	inline std::string getName() const { return name; }
	inline const std::map<std::string, std::shared_ptr<SpriteData>>& getSpriteData() { return spriteData; }
	std::shared_ptr<SpriteData> getSpriteData(std::string name) const;
	inline const std::map<std::string, std::shared_ptr<AnimationData>>& getAnimationData() { return animationData; }
	std::shared_ptr<AnimationData> getAnimationData(std::string animationName) const;
	inline bool isFailedImageLoad() const { return failedImageLoad; }
	inline bool isFailedMetafileLoad() const { return failedMetafileLoad; }
	inline sf::Texture* getTexture() { return &texture; }

	bool hasSpriteData(std::string spriteName) const;
	bool hasAnimationData(std::string animationName) const;

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

	// failedImageLoad and failedMetafileLoad indicate whether this SpriteSheet's image/metafile was loaded successfully by SpriteLoader.
	// These cannot be set back to false; SpriteLoader::loadFromSpriteSheetsFolder() must be called again to create
	// a new SpriteSheet and attempt to load it again.
	bool failedImageLoad = false;
	bool failedMetafileLoad = false;
};

/*
Note that if the SpriteLoader object goes out of scope, all Sprites loaded from it will not display correctly.
*/
class SpriteLoader {
public:
	struct LoadMetrics {
		int spriteSheetsFailed = 0;
		int spriteSheetsTotal = 0;

		std::string formatForUser();
		bool containsFailedLoads();
	};

	SpriteLoader(const std::string& levelPackName);

	/*
	Saves all currently loaded sprite sheets' metadata into the sprite sheets folder
	of the level pack with the name passed in from SpriteLoader's constructor.

	Since only successfully loaded sprite sheets will be saved, there is no danger
	of saving a corrupted sprite sheet as a result of an unsuccessful load.
	*/
	void saveMetadataFiles();

	/*
	Loads all sprite sheets from the sprite sheets folder of the level pack with the name 
	passed in from SpriteLoader's constructor. Sprite sheets without a corresponding
	metafile are still loaded but will not contain any usable sprites or animations.
	*/
	LoadMetrics loadFromSpriteSheetsFolder();

	/*
	Returns whether both the image and its metafile were successfully loaded.
	*/
	bool loadSpriteSheet(const std::string& spriteSheetMetaFileName, const std::string& spriteSheetImageFileName);
	/*
	Returns whether both the image and its metafile were successfully loaded.
	*/
	bool loadSpriteSheet(const std::string& spriteSheetName);

	/*
	Unloads an animation so that the next time it is fetched, changes in its AnimationData
	from before it was unloaded are visible.
	*/
	void unloadAnimation(const std::string& spriteSheetName, const std::string& animationName);

	void clearSpriteSheets();

	/*
	Updates an sprite sheet.
	If the sprite sheet name is already in the SpriteLoader, overwrite the sprite sheet.
	If the sprite sheet name is not in the SpriteLoader, add in the sprite sheet.
	*/
	void updateSpriteSheet(std::shared_ptr<SpriteSheet> spriteSheet);

	/*
	Scale all sprites by the same amount
	*/
	void setGlobalSpriteScale(float scale);

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
	bool spriteSheetFailedImageLoad(std::string spriteSheetName);
	bool spriteSheetFailedMetafileLoad(std::string spriteSheetName);
	/*
	Returns a copy of the default missing sprite.
	*/
	const std::shared_ptr<sf::Sprite> getMissingSprite();
	std::vector<std::string> getLoadedSpriteSheetNames();
	std::set<std::string> getLoadedSpriteSheetNamesAsSet();
	std::shared_ptr<SpriteSheet> getSpriteSheet(std::string spriteSheetName) const;
	bool hasSpriteSheet(std::string spriteSheetName) const;

	std::string formatPathToSpriteSheetImage(std::string imageFileNameWithExtension);
	std::string formatPathToSpriteSheetMetafile(std::string imageFileNameWithExtension);

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