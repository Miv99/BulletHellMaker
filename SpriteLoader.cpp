#include "SpriteLoader.h"
#include <fstream>
#include <sys/stat.h>
#include "TextFileParser.h"
#include <boost/log/trivial.hpp>
#include <regex>
#include <sstream>

static std::string SPRITE_SHEET_NAME_TAG = "SpriteSheetName";
static std::string SPRITE_TOPLEFT_COORDS_TAG = "TextureTopLeftCoordinates";
static std::string SPRITE_TEXTURE_BOUNDS_TAG = "BoundingRectangleSize";
static std::string SPRITE_COLOR_TAG = "Color";
static std::string SPRITE_SIZE_TAG = "SpriteSize";

std::vector<int> extractInts(const std::string& str) {	std::vector<int> vect;
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

std::shared_ptr<sf::Sprite> SpriteSheet::getSprite(const std::string& spriteName) {
	std::shared_ptr<SpriteSheetData> data = spriteSheetData.at(spriteName);
	ComparableIntRect area = data->getArea();

	// Texture has not been loaded yet
	if (textures.find(area) == textures.end()) {
		// Load texture
		std::shared_ptr<sf::Texture> texture = std::make_shared<sf::Texture>();
		texture->loadFromImage(*image, area);

		// Insert texture into map
		textures[area] = texture;
	}
	
	// Create sprite
	std::shared_ptr<sf::Sprite> sprite = std::make_shared<sf::Sprite>();
	sprite->setTexture(*textures[area]);
	sprite->setColor(data->getColor());
	sprite->setScale(data->getSpriteWidth() / area.width, data->getSpriteHeight() / area.height);
	return sprite;
}

void SpriteSheet::insertDataItem(const std::string& spriteName, std::shared_ptr<SpriteSheetData> spriteData) {
	spriteSheetData[spriteName] = spriteData;
}

bool SpriteSheet::loadImage(const std::string & imageFileName) {
	std::shared_ptr<sf::Image> image = std::make_shared<sf::Image>();
	if (!(image->loadFromFile(imageFileName))) {
		return false;
	}
	this->image = image;
	return true;
}

void SpriteSheet::preloadTextures() {
	for (auto it = spriteSheetData.begin(); it != spriteSheetData.end(); it++) {
		getSprite(it->first);
	}
}

bool SpriteSheetData::operator==(const SpriteSheetData & other) const {
	return this->area == other.area && this->color == other.color;
}

SpriteLoader::SpriteLoader(const std::string& levelPackRelativePath, const std::vector<std::pair<std::string, std::string>>& spriteSheetNamePairs) : levelPackRelativePath(levelPackRelativePath) {
	for (std::pair<std::string, std::string> namesPair : spriteSheetNamePairs) {
		if (!loadSpriteSheet(namesPair.first, namesPair.second)) {
			throw "Unable to load sprite sheet meta file \"" + namesPair.first + "\" and/or sprite sheet \"" + namesPair.second + "\"";
		}
	}
}

std::shared_ptr<sf::Sprite> SpriteLoader::getSprite(const std::string& spriteName, const std::string& spriteSheetName) {
	return spriteSheets[spriteSheetName]->getSprite(spriteName);
}

void SpriteLoader::preloadTextures() {
	for (auto it = spriteSheets.begin(); it != spriteSheets.end(); it++) {
		it->second->preloadTextures();
	}
}

void SpriteLoader::clearSpriteSheets() {
	spriteSheets.clear();
}

bool SpriteLoader::loadSpriteSheet(const std::string& spriteSheetMetaFileName, const std::string& spriteSheetImageFileName) {
	std::ifstream metafile(levelPackRelativePath + "\\" + spriteSheetMetaFileName);
	if (!metafile) {
		return false;
	}

	// Make sure image file exists
	struct stat buffer;
	if (!stat((levelPackRelativePath + "\\" + spriteSheetImageFileName).c_str(), &buffer) == 0) {
		metafile.close();
		return false;
	}

	try {
		// Sprite sheet name is the name of the meta file without the extension
		std::string spriteSheetName = spriteSheetMetaFileName.substr(0, spriteSheetMetaFileName.find_last_of("."));
		/*
		// get line "SpriteSheetName = sheet name here"
		getline(metafile, spriteSheetName);
		// so that name is now just "sheet name here"
		spriteSheetName.erase(0, (SPRITE_SHEET_NAME_TAG + " = ").length());
		spriteSheetName = removeTrailingWhitespace(spriteSheetName);
		*/
		std::shared_ptr<SpriteSheet> sheet = std::make_shared<SpriteSheet>(spriteSheetName);

		std::unique_ptr<std::map<std::string, std::unique_ptr<std::map<std::string, std::string>>>> metadata = TextFileParser(metafile).read('=');
		for (auto spriteIterator = metadata->begin(); spriteIterator != metadata->end(); spriteIterator++) {
			std::string spriteName = spriteIterator->first;

			// Extract sprite data
			ComparableIntRect area;
			// area's top-left coordinates
			if (spriteIterator->second->find(SPRITE_TOPLEFT_COORDS_TAG) != spriteIterator->second->end()) {
				auto temp = extractInts(spriteIterator->second->at(SPRITE_TOPLEFT_COORDS_TAG));
				if (temp.size() != 2) {
					throw "Sprite \"" + spriteName + "\"'s " + SPRITE_TOPLEFT_COORDS_TAG + " property has an invalid format";
				} else {
					area.left = temp[0];
					area.top = temp[1];
				}
			} else {
				throw "Sprite \"" + spriteName + "\" is missing property " + SPRITE_TOPLEFT_COORDS_TAG;
			}
			// area's width and height
			if (spriteIterator->second->find(SPRITE_TEXTURE_BOUNDS_TAG) != spriteIterator->second->end()) {
				auto temp = extractInts(spriteIterator->second->at(SPRITE_TEXTURE_BOUNDS_TAG));
				if (temp.size() != 2) {
					throw "Sprite \"" + spriteName + "\"'s " + SPRITE_TEXTURE_BOUNDS_TAG + " property has an invalid format";
				} else {
					area.width = temp[0];
					area.height = temp[1];
				}
			} else {
				throw "Sprite \"" + spriteName + "\" is missing property " + SPRITE_TEXTURE_BOUNDS_TAG;
			}
			// color
			sf::Color color = sf::Color(255, 255, 255, 255);
			if (spriteIterator->second->find(SPRITE_COLOR_TAG) != spriteIterator->second->end()) {
				auto temp = extractInts(spriteIterator->second->at(SPRITE_COLOR_TAG));
				if (temp.size() != 4) {
					throw "Sprite \"" + spriteName + "\"'s " + SPRITE_COLOR_TAG + " property has an invalid format";
				} else {
					color.r = temp[0];
					color.g = temp[1];
					color.g = temp[2];
					color.a = temp[3];
				}
			}
			// sprite size
			float spriteWidth, spriteHeight;
			if (spriteIterator->second->find(SPRITE_SIZE_TAG) != spriteIterator->second->end()) {
				auto temp = extractInts(spriteIterator->second->at(SPRITE_SIZE_TAG));
				if (temp.size() != 2) {
					throw "Sprite \"" + spriteName + "\"'s " + SPRITE_SIZE_TAG + " property has an invalid format";
				} else {
					spriteWidth = temp[0];
					spriteHeight = temp[1];
				}
			} else {
				throw "Sprite \"" + spriteName + "\" is missing property " + SPRITE_SIZE_TAG;
			}

			// Insert SpriteSheetData into SpriteSheet
			std::shared_ptr<SpriteSheetData> dataItem = std::make_shared<SpriteSheetData>(spriteName, area, spriteWidth, spriteHeight, color);
			sheet->insertDataItem(spriteName, dataItem);
		}

		// Load image file
		if (!sheet->loadImage(levelPackRelativePath + "\\" + spriteSheetImageFileName)) {
			throw "Image file \"" + levelPackRelativePath + "\\" + spriteSheetImageFileName + "\" could not be loaded";
		}

		spriteSheets[spriteSheetName] = sheet;
	} catch (std::exception e) {
		BOOST_LOG_TRIVIAL(error) << "Invalid format in \"" + spriteSheetMetaFileName + "\"; " + e.what();
		metafile.close();
		return false;
	}
	metafile.close();

	return true;
}
