#include "SpriteLoader.h"
#include "TextFileParser.h"
#include "TextMarshallable.h"
#include <fstream>
#include <sys/stat.h>
#include <boost/log/trivial.hpp>
#include <regex>
#include <sstream>
#include <limits>

static const std::string SPRITE_SHEET_NAME_TAG = "SpriteSheetName";
static const std::string SPRITE_TOPLEFT_COORDS_TAG = "TextureTopLeftCoordinates";
static const std::string SPRITE_TEXTURE_BOUNDS_TAG = "BoundingRectangleSize";
static const std::string SPRITE_COLOR_TAG = "Color";
static const std::string SPRITE_SIZE_TAG = "SpriteSize";
static const std::string SPRITE_ORIGIN_TAG = "Origin";

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
	std::shared_ptr<SpriteData> data = spriteData.at(spriteName);
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
	sprite->setOrigin(data->getSpriteOriginX(), data->getSpriteOriginY());
	return sprite;
}

std::unique_ptr<Animation> SpriteSheet::getAnimation(const std::string& animationName, bool loop) {
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

bool SpriteSheet::loadImage(const std::string & imageFileName) {
	std::shared_ptr<sf::Image> image = std::make_shared<sf::Image>();
	if (!(image->loadFromFile(imageFileName))) {
		return false;
	}
	this->image = image;
	return true;
}

void SpriteSheet::preloadTextures() {
	for (auto it = spriteData.begin(); it != spriteData.end(); it++) {
		getSprite(it->first);
	}
}

bool SpriteData::operator==(const SpriteData & other) const {
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

std::unique_ptr<Animation> SpriteLoader::getAnimation(const std::string & animationName, const std::string & spriteSheetName, bool loop) {
	return spriteSheets[spriteSheetName]->getAnimation(animationName, loop);
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
		std::shared_ptr<SpriteSheet> sheet = std::make_shared<SpriteSheet>(spriteSheetName);

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
					} else {
						area.left = temp[0];
						area.top = temp[1];
					}
				} else {
					throw "Sprite \"" + name + "\" is missing property " + SPRITE_TOPLEFT_COORDS_TAG;
				}
				// area's width and height
				if (animationIterator->second->find(SPRITE_TEXTURE_BOUNDS_TAG) != animationIterator->second->end()) {
					auto temp = extractInts(animationIterator->second->at(SPRITE_TEXTURE_BOUNDS_TAG));
					if (temp.size() != 2) {
						throw "Sprite \"" + name + "\"'s " + SPRITE_TEXTURE_BOUNDS_TAG + " property has an invalid format";
					} else {
						area.width = temp[0];
						area.height = temp[1];
					}
				} else {
					throw "Sprite \"" + name + "\" is missing property " + SPRITE_TEXTURE_BOUNDS_TAG;
				}
				// color
				sf::Color color = sf::Color(255, 255, 255, 255);
				if (animationIterator->second->find(SPRITE_COLOR_TAG) != animationIterator->second->end()) {
					auto temp = extractInts(animationIterator->second->at(SPRITE_COLOR_TAG));
					if (temp.size() != 4) {
						throw "Sprite \"" + name + "\"'s " + SPRITE_COLOR_TAG + " property has an invalid format";
					} else {
						color.r = temp[0];
						color.g = temp[1];
						color.g = temp[2];
						color.a = temp[3];
					}
				}
				// sprite size
				float spriteWidth, spriteHeight;
				if (animationIterator->second->find(SPRITE_SIZE_TAG) != animationIterator->second->end()) {
					auto temp = extractInts(animationIterator->second->at(SPRITE_SIZE_TAG));
					if (temp.size() != 2) {
						throw "Sprite \"" + name + "\"'s " + SPRITE_SIZE_TAG + " property has an invalid format";
					} else {
						spriteWidth = temp[0];
						spriteHeight = temp[1];
					}
				} else {
					throw "Sprite \"" + name + "\" is missing property " + SPRITE_SIZE_TAG;
				}
				// sprite origin
				float originX = spriteWidth/2.0f, originY = spriteHeight/2.0f;
				if (animationIterator->second->find(SPRITE_ORIGIN_TAG) != animationIterator->second->end()) {
					auto temp = extractInts(animationIterator->second->at(SPRITE_ORIGIN_TAG));
					if (temp.size() != 2) {
						throw "Sprite \"" + name + "\"'s " + SPRITE_ORIGIN_TAG + " property has an invalid format";
					} else {
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

bool AnimationData::operator==(const AnimationData & other) const {
	return animationName == other.animationName;
}
