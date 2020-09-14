#include <DataStructs/SpriteLoader.h>

#include <fstream>
#include <regex>
#include <sstream>
#include <limits>
#include <filesystem>
#include <set>

#include <Config.h>
#include <Util/TextFileParser.h>
#include <LevelPack/TextMarshallable.h>
#include <Util/IOUtils.h>

static const std::string SPRITE_SHEET_NAME_TAG = "SpriteSheetName";
static const std::string SPRITE_TOPLEFT_COORDS_TAG = "TextureTopLeftCoordinates";
static const std::string SPRITE_TEXTURE_BOUNDS_TAG = "BoundingRectangleSize";
static const std::string SPRITE_COLOR_TAG = "Color";
static const std::string SPRITE_SIZE_TAG = "SpriteSize";
static const std::string SPRITE_ORIGIN_TAG = "Origin";
const std::size_t SpriteLoader::BACKGROUNDS_CACHE_MAX_SIZE = 10;
const std::size_t SpriteLoader::GUI_ELEMENTS_CACHE_MAX_SIZE = 10;

std::vector<int> extractInts(const std::string& str) {
	std::vector<int> vect;
	std::stringstream ss(str);

	int i;
	while (ss >> i) {
		vect.push_back(i);

		if (ss.peek() == ',') {
			ss.ignore();
		}
	}

	return vect;
}

SpriteSheet::SpriteSheet(std::string name) 
	: name(name) {
}

std::shared_ptr<sf::Sprite> SpriteSheet::getSprite(const std::string& spriteName) {
	if (spriteData.count(spriteName) == 0) {
		// Missing sprite
		return nullptr;
	}
	std::shared_ptr<SpriteData> data = spriteData.at(spriteName);
	ComparableIntRect area = data->getArea();

	// Create sprite
	std::shared_ptr<sf::Sprite> sprite = std::make_shared<sf::Sprite>();
	sprite->setTexture(texture);
	sprite->setTextureRect(area);
	sprite->setColor(data->getColor());
	sprite->setScale((float)data->getSpriteWidth() / area.width * globalSpriteScale, (float)data->getSpriteHeight() / area.height * globalSpriteScale);
	sprite->setOrigin(data->getSpriteOriginX(), data->getSpriteOriginY());
	return sprite;
}

std::unique_ptr<Animation> SpriteSheet::getAnimation(const std::string& animationName, bool loop) {
	if (animationData.count(animationName) == 0) {
		// Missing animation
		return nullptr;
	}
	std::shared_ptr<AnimationData> data = animationData.at(animationName);

	// Animation has not been loaded yet
	if (animationSprites.find(animationName) == animationSprites.end()) {
		std::vector<std::pair<float, std::shared_ptr<sf::Sprite>>> sprites;
		for (auto p : data->getSpriteNames()) {
			sprites.push_back(std::make_pair(p.first, getSprite(p.second)));
		}
		animationSprites[animationName] = sprites;
	}

	return std::make_unique<Animation>(animationName, animationSprites[animationName], loop);
}

void SpriteSheet::insertSprite(const std::string& spriteName, std::shared_ptr<SpriteData> sprite) {
	spriteData[spriteName] = sprite;
}

void SpriteSheet::insertAnimation(const std::string & animationName, std::shared_ptr<AnimationData> animation) {
	animationData[animationName] = animation;
}

bool SpriteSheet::loadTexture(const std::string& spriteSheetFilePath) {
	if (!(texture.loadFromFile(spriteSheetFilePath))) {
		return false;
	}
	return true;
}

void SpriteSheet::setGlobalSpriteScale(float scale) {
	globalSpriteScale = scale;
	for (auto it = spriteData.begin(); it != spriteData.end(); it++) {
		ComparableIntRect area = it->second->getArea();
		getSprite(it->first)->setScale((float)it->second->getSpriteWidth() / area.width * globalSpriteScale, (float)it->second->getSpriteHeight() / area.height * globalSpriteScale);

	}
}

SpriteData::SpriteData(std::string spriteName, ComparableIntRect area, int spriteWidth, int spriteHeight, int spriteOriginX, int spriteOriginY, sf::Color color) 
	: spriteName(spriteName), area(area), color(color), spriteWidth(spriteWidth), spriteHeight(spriteHeight), spriteOriginX(spriteOriginX), spriteOriginY(spriteOriginY) {
}

bool SpriteData::operator==(const SpriteData & other) const {
	return this->area == other.area && this->color == other.color;
}

SpriteLoader::SpriteLoader(const std::string& levelPackName) 
	: levelPackName(levelPackName) {

	backgroundsCache = std::make_unique<Cache<std::string, std::pair<std::shared_ptr<sf::Texture>, std::filesystem::file_time_type>>>(BACKGROUNDS_CACHE_MAX_SIZE);
	guiElementsCache = std::make_unique<Cache<std::string, std::pair<std::shared_ptr<sf::Texture>, std::filesystem::file_time_type>>>(GUI_ELEMENTS_CACHE_MAX_SIZE);
	
	loadFromSpriteSheetsFolder();

	// Create default missing sprite
	sf::Image missingSpriteImage;
	sf::Uint8 pixels[16];
	pixels[0] = 255;
	pixels[1] = 0;
	pixels[2] = 255;
	pixels[3] = 255;
	pixels[4] = 0;
	pixels[5] = 0;
	pixels[6] = 0;
	pixels[7] = 255;
	pixels[8] = 0;
	pixels[9] = 0;
	pixels[10] = 0;
	pixels[11] = 255;
	pixels[12] = 255;
	pixels[13] = 0;
	pixels[14] = 255;
	pixels[15] = 255;
	missingSpriteImage.create(2, 2, pixels);
	missingSpriteTexture = std::make_shared<sf::Texture>();
	missingSpriteTexture->loadFromImage(missingSpriteImage);
	missingSprite = std::make_shared<sf::Sprite>(*missingSpriteTexture);
}

void SpriteLoader::loadFromSpriteSheetsFolder() {
	spriteSheets.clear();

	std::string spriteSheetsFolderPath = format(getPathToFolderContainingExe() + "\\" + RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH, levelPackName.c_str());
	std::vector<std::pair<std::string, std::string>> spriteSheetPairs = findAllSpriteSheetsWithMetafiles(spriteSheetsFolderPath);
	for (std::pair<std::string, std::string> pairs : spriteSheetPairs) {
		// TODO: log that this sprite sheet is being loaded

		bool loadIsSuccessful = loadSpriteSheet(pairs.first, pairs.second);
		if (!loadIsSuccessful) {
			// TODO: log that sprite sheet meta file or sprite sheet couldn't be loaded
		}
	}
}

std::shared_ptr<sf::Texture> SpriteLoader::getGuiElementTexture(const std::string& guiElementFileName) {
	std::string filePath = format(RELATIVE_LEVEL_PACK_GUI_FOLDER_PATH + "\\%s", levelPackName.c_str(), guiElementFileName.c_str());
	if (!fileExists(filePath)) {
		return missingSpriteTexture;
	}
	std::filesystem::file_time_type fileLastModified = std::filesystem::last_write_time(filePath);
	if (guiElementsCache->contains(guiElementFileName)) {
		std::pair<std::shared_ptr<sf::Texture>, std::filesystem::file_time_type> guiElementData = guiElementsCache->get(guiElementFileName);
		if (guiElementData.second == fileLastModified) {
			// File has not been modified, so return the cached texture
			return guiElementData.first;
		} else {
			// File has been modified, so remove the old texture from the cache
			guiElementsCache->remove(guiElementFileName);
		}
	}
	// Load the GUI element from file and insert into cache
	std::shared_ptr<sf::Texture> guiElement = std::make_shared<sf::Texture>();
	if (!guiElement->loadFromFile(filePath)) {
		return nullptr;
	}
	guiElement->setRepeated(true);
	guiElement->setSmooth(true);
	guiElementsCache->insert(guiElementFileName, std::make_pair(guiElement, fileLastModified));
	return guiElement;
}

std::shared_ptr<sf::Sprite> SpriteLoader::getSprite(const std::string& spriteName, const std::string& spriteSheetName) {
	if (spriteSheets.count(spriteSheetName) == 0) {
		// Missing sprite sheet
		return getMissingSprite();
	}
	std::shared_ptr<sf::Sprite> sprite = spriteSheets[spriteSheetName]->getSprite(spriteName);
	if (sprite) {
		return sprite;
	} else {
		return getMissingSprite();
	}
}

std::unique_ptr<Animation> SpriteLoader::getAnimation(const std::string & animationName, const std::string & spriteSheetName, bool loop) {
	if (spriteSheets.count(spriteSheetName) == 0) {
		// Missing sprite sheet
		return nullptr;
	}
	return spriteSheets[spriteSheetName]->getAnimation(animationName, loop);
}

std::shared_ptr<sf::Texture> SpriteLoader::getBackground(const std::string& backgroundFileName) {
	std::string filePath = format(RELATIVE_LEVEL_PACK_BACKGROUNDS_FOLDER_PATH + "\\%s", levelPackName.c_str(), backgroundFileName.c_str());
	if (!fileExists(filePath)) {
		return missingSpriteTexture;
	}
	std::filesystem::file_time_type fileLastModified = std::filesystem::last_write_time(filePath);
	if (backgroundsCache->contains(backgroundFileName)) {
		std::pair<std::shared_ptr<sf::Texture>, std::filesystem::file_time_type> backgroundData = backgroundsCache->get(backgroundFileName);
		if (backgroundData.second == fileLastModified) {
			// File has not been modified, so return the cached texture
			return backgroundData.first;
		} else {
			// File has been modified, so remove the old texture from the cache
			backgroundsCache->remove(backgroundFileName);
		}
	}
	// Load the background from file and insert into cache
	std::shared_ptr<sf::Texture> background = std::make_shared<sf::Texture>();
	if (!background->loadFromFile(filePath)) {
		return nullptr;
	}
	background->setRepeated(true);
	background->setSmooth(true);
	backgroundsCache->insert(backgroundFileName, std::make_pair(background, fileLastModified));
	return background;
}

const std::shared_ptr<sf::Sprite> SpriteLoader::getMissingSprite() {
	std::shared_ptr<sf::Sprite> sprite = std::make_shared<sf::Sprite>(*missingSprite);
	// Default 100x100
	sprite->setScale(50.0f * globalSpriteScale, 50.0f * globalSpriteScale);
	sprite->setOrigin(1, 1);
	return sprite;
}

void SpriteLoader::clearSpriteSheets() {
	spriteSheets.clear();
}

void SpriteLoader::setGlobalSpriteScale(float scale) {
	this->globalSpriteScale = scale;
	for (auto it = spriteSheets.begin(); it != spriteSheets.end(); it++) {
		it->second->setGlobalSpriteScale(scale);
	}
}

std::vector<std::string> SpriteLoader::getLoadedSpriteSheetNames() {
	std::vector<std::string> results;

	for (std::pair<std::string, std::shared_ptr<SpriteSheet>> spriteSheet : spriteSheets) {
		results.push_back(spriteSheet.first);
	}

	return results;
}

bool SpriteLoader::loadSpriteSheet(const std::string& spriteSheetMetaFileName, const std::string& spriteSheetImageFileName) {
	std::ifstream metafile(format(RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH + "\\%s", levelPackName.c_str(), spriteSheetMetaFileName.c_str()));
	if (!metafile) {
		return false;
	}

	// Make sure image file exists
	struct stat buffer;
	if (!stat((format(RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH + "\\%s", levelPackName.c_str(), spriteSheetImageFileName.c_str())).c_str(), &buffer) == 0) {
		metafile.close();
		return false;
	}

	try {
		// Sprite sheet name is the name of the image file
		std::shared_ptr<SpriteSheet> sheet = std::make_shared<SpriteSheet>(spriteSheetImageFileName);

		std::unique_ptr<std::map<std::string, std::unique_ptr<std::map<std::string, std::string>>>> metadata = TextFileParser(metafile).read('=');
		for (auto animationIterator = metadata->begin(); animationIterator != metadata->end(); animationIterator++) {
			std::string name = animationIterator->first;
			std::string type = animationIterator->second->at("Type");

			if (type == "Sprite") {
				// Extract sprite data
				ComparableIntRect area;
				// area's top-left coordinates
				if (animationIterator->second->find(SPRITE_TOPLEFT_COORDS_TAG) != animationIterator->second->end()) {
					auto temp = extractInts(animationIterator->second->at(SPRITE_TOPLEFT_COORDS_TAG));
					if (temp.size() != 2) {
						throw "Sprite \"" + name + "\"'s " + SPRITE_TOPLEFT_COORDS_TAG + " property has an invalid format";
					}
					else {
						area.left = temp[0];
						area.top = temp[1];
					}
				}
				else {
					throw "Sprite \"" + name + "\" is missing property " + SPRITE_TOPLEFT_COORDS_TAG;
				}
				// area's width and height
				if (animationIterator->second->find(SPRITE_TEXTURE_BOUNDS_TAG) != animationIterator->second->end()) {
					auto temp = extractInts(animationIterator->second->at(SPRITE_TEXTURE_BOUNDS_TAG));
					if (temp.size() != 2) {
						throw "Sprite \"" + name + "\"'s " + SPRITE_TEXTURE_BOUNDS_TAG + " property has an invalid format";
					}
					else {
						area.width = temp[0];
						area.height = temp[1];
					}
				}
				else {
					throw "Sprite \"" + name + "\" is missing property " + SPRITE_TEXTURE_BOUNDS_TAG;
				}
				// color
				sf::Color color = sf::Color(255, 255, 255, 255);
				if (animationIterator->second->find(SPRITE_COLOR_TAG) != animationIterator->second->end()) {
					auto temp = extractInts(animationIterator->second->at(SPRITE_COLOR_TAG));
					if (temp.size() != 4) {
						throw "Sprite \"" + name + "\"'s " + SPRITE_COLOR_TAG + " property has an invalid format";
					}
					else {
						color.r = temp[0];
						color.g = temp[1];
						color.b = temp[2];
						color.a = temp[3];
					}
				}
				// sprite size
				float spriteWidth, spriteHeight;
				if (animationIterator->second->find(SPRITE_SIZE_TAG) != animationIterator->second->end()) {
					auto temp = extractInts(animationIterator->second->at(SPRITE_SIZE_TAG));
					if (temp.size() != 2) {
						throw "Sprite \"" + name + "\"'s " + SPRITE_SIZE_TAG + " property has an invalid format";
					}
					else {
						spriteWidth = temp[0];
						spriteHeight = temp[1];
					}
				}
				else {
					throw "Sprite \"" + name + "\" is missing property " + SPRITE_SIZE_TAG;
				}
				// sprite origin
				float originX = area.width / 2.0f, originY = area.height / 2.0f;
				if (animationIterator->second->find(SPRITE_ORIGIN_TAG) != animationIterator->second->end()) {
					auto temp = extractInts(animationIterator->second->at(SPRITE_ORIGIN_TAG));
					if (temp.size() != 2) {
						throw "Sprite \"" + name + "\"'s " + SPRITE_ORIGIN_TAG + " property has an invalid format";
					}
					else {
						originX = temp[0];
						originY = temp[1];
					}
				}

				// Insert SpriteSheetData into SpriteSheet
				std::shared_ptr<SpriteData> dataItem = std::make_shared<SpriteData>(name, area, spriteWidth, spriteHeight, originX, originY, color);
				sheet->insertSprite(name, dataItem);

				// Also create a looping animation that consists of only that sprite with the same name
				/*
				std::vector<std::pair<float, std::string>> spriteNames = { std::make_pair(std::numeric_limits<float>::max(), name) };
				std::shared_ptr<AnimationData> animation = std::make_shared<AnimationData>(name, ANIMATION_TYPE_LOOPING, spriteNames);
				sheet->insertAnimation(name, animation);
				*/
			}
			// Type is animation
			else {
				std::vector<std::pair<float, std::string>> spriteNames;
				auto strs = split(animationIterator->second->at("FrameData"), '|');
				for (int i = 0; i < strs.size(); i += 2) {
					spriteNames.push_back(std::make_pair(std::stof(strs[i]), strs[i + 1]));
				}
				std::shared_ptr<AnimationData> animation = std::make_shared<AnimationData>(name, spriteNames);
				sheet->insertAnimation(name, animation);
			}
		}

		// Load image file
		if (!sheet->loadTexture(format(RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH + "\\%s", levelPackName.c_str(), spriteSheetImageFileName.c_str()))) {
			// TODO: log that image couldn't be loaded
		}

		spriteSheets[spriteSheetImageFileName] = sheet;
	}
	catch (std::exception e) {
		// TODO: log this
		// BOOST_LOG_TRIVIAL(error) << "Invalid format in \"" + spriteSheetMetaFileName + "\"; " + e.what();
		metafile.close();
		return false;
	}
	metafile.close();

	return true;
}

AnimationData::AnimationData(std::string animationName, std::vector<std::pair<float, std::string>> spriteNames) 
	: animationName(animationName), spriteNames(spriteNames) {
}

bool AnimationData::operator==(const AnimationData & other) const {
	return animationName == other.animationName;
}