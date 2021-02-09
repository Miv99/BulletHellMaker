#include <DataStructs/SpriteLoader.h>

#include <fstream>
#include <regex>
#include <sstream>
#include <limits>
#include <filesystem>
#include <set>

#include <Constants.h>
#include <Config.h>
#include <Util/Logger.h>
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

SpriteSheet::SpriteSheet() {
}

SpriteSheet::SpriteSheet(std::string name)
	: name(name) {
}

SpriteSheet::SpriteSheet(const std::shared_ptr<SpriteSheet> other) {
	load(other->format());
	this->failedImageLoad = other->failedImageLoad;
	this->failedMetafileLoad = other->failedMetafileLoad;
}

std::string SpriteSheet::format() const {
	std::string result = formatString(name) + tos(spriteData.size());
	for (std::pair<std::string, std::shared_ptr<SpriteData>> entry : spriteData) {
		result += formatTMObject(*entry.second);
	}
	result += tos(animationData.size());
	for (std::pair<std::string, std::shared_ptr<AnimationData>> entry : animationData) {
		result += formatTMObject(*entry.second);
	}
	return result;
}

void SpriteSheet::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	name = items.at(0);

	spriteData.clear();
	int i = 1;
	for (i = 2; i < std::stoi(items.at(1)) + 2; i++) {
		std::shared_ptr<SpriteData> entry = std::make_shared<SpriteData>();
		entry->load(items.at(i));
		spriteData[entry->getSpriteName()] = entry;
	}

	animationData.clear();
	int animationDataStartIndex = i++;
	for (; i < std::stoi(items.at(animationDataStartIndex)) + animationDataStartIndex + 1; i++) {
		std::shared_ptr<AnimationData> entry = std::make_shared<AnimationData>();
		entry->load(items.at(i));
		animationData[entry->getAnimationName()] = entry;
	}
}

nlohmann::json SpriteSheet::toJson() {
	nlohmann::json j = {
		{"name", name}
	};

	nlohmann::json spriteDataJson;
	for (auto p : spriteData) {
		spriteDataJson[p.second->getSpriteName()] = p.second->toJson();
	}
	j["spriteData"] = spriteDataJson;

	nlohmann::json animationDataJson;
	for (auto p : animationData) {
		animationDataJson[p.second->getAnimationName()] = p.second->toJson();
	}
	j["animationData"] = animationDataJson;

	return j;
}

void SpriteSheet::load(const nlohmann::json& j) {
	j.at("name").get_to(name);

	spriteData.clear();
	if (j.contains("spriteData")) {
		for (const nlohmann::json& item : j.at("spriteData")) {
			std::shared_ptr<SpriteData> data = std::make_shared<SpriteData>();
			data->load(item);

			spriteData[data->getSpriteName()] = data;
		}
	}

	animationData.clear();
	if (j.contains("animationData")) {
		for (const nlohmann::json& item : j.at("animationData")) {
			std::shared_ptr<AnimationData> data = std::make_shared<AnimationData>();
			data->load(item);

			animationData[data->getAnimationName()] = data;
		}
	}
}

std::shared_ptr<sf::Sprite> SpriteSheet::getSprite(const std::string& spriteName) {
	if (spriteData.find(spriteName) == spriteData.end()) {
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
	if (animationData.find(animationName) == animationData.end()) {
		// Missing animation
		return nullptr;
	}
	std::shared_ptr<AnimationData> data = animationData.at(animationName);

	// Animation has not been loaded yet
	if (animationSprites.find(animationName) == animationSprites.end()) {
		std::vector<std::pair<float, std::shared_ptr<sf::Sprite>>> sprites;
		for (auto p : data->getSpriteInfo()) {
			sprites.push_back(std::make_pair(p.first, getSprite(p.second)));
		}
		animationSprites[animationName] = sprites;
	}

	return std::make_unique<Animation>(animationName, animationSprites[animationName], loop);
}

std::shared_ptr<SpriteData> SpriteSheet::getSpriteData(std::string spriteName) const {
	return spriteData.at(spriteName);
}

std::shared_ptr<AnimationData> SpriteSheet::getAnimationData(std::string animationName) const {
	return animationData.at(animationName);
}

bool SpriteSheet::hasSpriteData(std::string spriteName) const {
	return spriteData.find(spriteName) != spriteData.end();
}

bool SpriteSheet::hasAnimationData(std::string animationName) const {
	return animationData.find(animationName) != animationData.end();
}

void SpriteSheet::insertSprite(const std::string& spriteName, std::shared_ptr<SpriteData> sprite) {
	spriteData[spriteName] = sprite;
}

void SpriteSheet::insertAnimation(const std::string & animationName, std::shared_ptr<AnimationData> animation) {
	animationData[animationName] = animation;
}

void SpriteSheet::deleteSprite(const std::string& spriteName) {
	if (hasSpriteData(spriteName)) {
		spriteData.erase(spriteName);
	}
}

void SpriteSheet::deleteAnimation(const std::string& animationName) {
	if (hasAnimationData(animationName)) {
		animationData.erase(animationName);
		animationSprites.erase(animationName);
	}
}

bool SpriteSheet::loadTexture(const std::string& spriteSheetFilePath) {
	if (!(texture.loadFromFile(spriteSheetFilePath))) {
		return false;
	}
	return true;
}

void SpriteSheet::unloadAnimation(const std::string& animationName) {
	animationSprites.erase(animationName);
}

void SpriteSheet::setGlobalSpriteScale(float scale) {
	globalSpriteScale = scale;
	for (auto it = spriteData.begin(); it != spriteData.end(); it++) {
		ComparableIntRect area = it->second->getArea();
		getSprite(it->first)->setScale((float)it->second->getSpriteWidth() / area.width * globalSpriteScale, (float)it->second->getSpriteHeight() / area.height * globalSpriteScale);

	}
}

void SpriteSheet::markFailedImageLoad() {
	failedImageLoad = true;
}

void SpriteSheet::markFailedMetafileLoad() {
	failedMetafileLoad = true;
}

void SpriteSheet::renameSprite(std::string oldSpriteName, std::string newSpriteName) {
	if (spriteData.find(oldSpriteName) == spriteData.end()) {
		return;
	}

	if (spriteData.find(newSpriteName) != spriteData.end()) {
		throw std::runtime_error("The new sprite name already exists.");
	}

	std::shared_ptr<SpriteData> data = spriteData[oldSpriteName];
	spriteData.erase(oldSpriteName);
	data->setSpriteName(newSpriteName);
	spriteData[newSpriteName] = data;
}

void SpriteSheet::renameAnimation(std::string oldAnimationName, std::string newAnimationName) {
	if (animationData.find(oldAnimationName) == animationData.end()) {
		return;
	}

	if (animationData.find(oldAnimationName) != animationData.end()) {
		throw std::runtime_error("The new animation name already exists.");
	}

	std::shared_ptr<AnimationData> data = animationData[oldAnimationName];
	animationData.erase(oldAnimationName);
	data->setAnimationName(newAnimationName);
	animationData[newAnimationName] = data;
}

SpriteData::SpriteData() {
}

SpriteData::SpriteData(std::string spriteName, ComparableIntRect area, int spriteWidth, int spriteHeight, int spriteOriginX, int spriteOriginY, sf::Color color)
	: spriteName(spriteName), area(area), color(color), spriteWidth(spriteWidth), spriteHeight(spriteHeight), spriteOriginX(spriteOriginX), spriteOriginY(spriteOriginY) {
}

SpriteData::SpriteData(const SpriteData& other) {
	load(other.format());
}

SpriteData::SpriteData(std::shared_ptr<SpriteData> other) {
	load(other->format());
}

std::string SpriteData::format() const {
	return formatString(spriteName) + tos(area.left) + tos(area.top) + tos(area.width) + tos(area.height)
		+ tos(color.r) + tos(color.g) + tos(color.b) + tos(color.a)
		+ tos(spriteWidth) + tos(spriteHeight) + tos(spriteOriginX) + tos(spriteOriginY);
}

void SpriteData::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	spriteName = items.at(0);
	area.left = std::stoi(items.at(1));
	area.top = std::stoi(items.at(2));
	area.width = std::stoi(items.at(3));
	area.height = std::stoi(items.at(4));
	color.r = std::stoi(items.at(5));
	color.g = std::stoi(items.at(6));
	color.b = std::stoi(items.at(7));
	color.a = std::stoi(items.at(8));
	spriteWidth = std::stoi(items.at(9));
	spriteHeight = std::stoi(items.at(10));
	spriteOriginX = std::stoi(items.at(11));
	spriteOriginY = std::stoi(items.at(12));
}

nlohmann::json SpriteData::toJson() {
	return {
		{"spriteName", spriteName},
		{"area", nlohmann::json{ {"left", area.left}, {"top", area.top}, 
			{"width", area.width}, {"height", area.height} }},
		{"color", nlohmann::json{ {"r", color.r}, {"g", color.g},
			{"b", color.b}, {"a", color.a} }},
		{"spriteWidth", spriteWidth},
		{"spriteHeight", spriteHeight},
		{"spriteOriginX", spriteOriginX},
		{"spriteOriginY", spriteOriginY}
	};
}

void SpriteData::load(const nlohmann::json& j) {
	j.at("spriteName").get_to(spriteName);

	j.at("area").at("left").get_to(area.left);
	j.at("area").at("top").get_to(area.top);
	j.at("area").at("width").get_to(area.width);
	j.at("area").at("height").get_to(area.height);

	j.at("color").at("r").get_to(color.r);
	j.at("color").at("g").get_to(color.g);
	j.at("color").at("b").get_to(color.b);
	j.at("color").at("a").get_to(color.a);

	j.at("spriteWidth").get_to(spriteWidth);
	j.at("spriteHeight").get_to(spriteHeight);
	j.at("spriteOriginX").get_to(spriteOriginX);
	j.at("spriteOriginY").get_to(spriteOriginY);
}

bool SpriteData::operator==(const SpriteData & other) const {
	return this->area == other.area && this->color == other.color;
}

void SpriteData::setSpriteName(std::string spriteName) {
	this->spriteName = spriteName;
}

void SpriteData::setTextureArea(sf::IntRect area) {
	this->area = ComparableIntRect(area);
}

void SpriteData::setColor(sf::Color color) {
	this->color = color;
}

void SpriteData::setSpriteSize(int width, int height) {
	this->spriteWidth = width;
	this->spriteHeight = height;
}

void SpriteData::setSpriteOrigin(float x, float y) {
	this->spriteOriginX = x;
	this->spriteOriginY = y;
}

SpriteLoader::SpriteLoader(const std::string& levelPackName) 
	: levelPackName(levelPackName) {

	backgroundsCache = std::make_unique<Cache<std::string, std::pair<std::shared_ptr<sf::Texture>, std::filesystem::file_time_type>>>(BACKGROUNDS_CACHE_MAX_SIZE);
	guiElementsCache = std::make_unique<Cache<std::string, std::pair<std::shared_ptr<sf::Texture>, std::filesystem::file_time_type>>>(GUI_ELEMENTS_CACHE_MAX_SIZE);
	
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

void SpriteLoader::saveMetadataFiles() {
	std::string spriteSheetsFolderPath = format(RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH, levelPackName.c_str());

	for (std::pair<std::string, std::shared_ptr<SpriteSheet>> spriteSheet : spriteSheets) {
		// Only save SpriteSheets that were initially loaded successfully
		if (!spriteSheet.second->isFailedMetafileLoad()) {
			std::ofstream metafile(format(spriteSheetsFolderPath + "\\%s%s", spriteSheet.first.c_str(), LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str()));
			metafile << std::setw(4) << spriteSheet.second->toJson();
			metafile.close();
		}
	}
}

SpriteLoader::LoadMetrics SpriteLoader::loadFromSpriteSheetsFolder() {
	spriteSheets.clear();

	LoadMetrics loadMetrics;

	std::string spriteSheetsFolderPath = format(RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH, levelPackName.c_str());
	std::vector<std::pair<std::string, std::string>> spriteSheetPairs = findAllSpriteSheets(spriteSheetsFolderPath);
	for (std::pair<std::string, std::string> pair : spriteSheetPairs) {
		if (pair.first.size() == 0) {
			L_(linfo) << "Loading sprite sheet image file \"" << pair.second << "\" with no metafile";
		} else {
			L_(linfo) << "Loading sprite sheet image file \"" << pair.second << "\" with metafile \"" << pair.first << "\"";
		}

		bool loadIsSuccessful = loadSpriteSheet(pair.first, pair.second);
		if (!loadIsSuccessful) {
			L_(lerror) << "Failed to load sprite sheet";
			loadMetrics.spriteSheetsFailed++;
		}
		loadMetrics.spriteSheetsTotal++;
	}

	return loadMetrics;
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
	if (spriteSheets.find(spriteSheetName) == spriteSheets.end()) {
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
	if (spriteSheets.find(spriteSheetName) == spriteSheets.end()) {
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

bool SpriteLoader::spriteSheetFailedImageLoad(std::string spriteSheetName) {
	return spriteSheets[spriteSheetName]->isFailedImageLoad();
}

bool SpriteLoader::spriteSheetFailedMetafileLoad(std::string spriteSheetName) {
	return spriteSheets[spriteSheetName]->isFailedMetafileLoad();
}

const std::shared_ptr<sf::Sprite> SpriteLoader::getMissingSprite() {
	std::shared_ptr<sf::Sprite> sprite = std::make_shared<sf::Sprite>(*missingSprite);
	// Default 100x100
	sprite->setScale(50.0f * globalSpriteScale, 50.0f * globalSpriteScale);
	sprite->setOrigin(1, 1);
	return sprite;
}

bool SpriteLoader::loadSpriteSheet(const std::string& spriteSheetName) {
	std::string spriteSheetMetafileName = spriteSheetName + LEVEL_PACK_SERIALIZED_DATA_FORMAT;
	return loadSpriteSheet(spriteSheetMetafileName, spriteSheetName);
}

void SpriteLoader::unloadAnimation(const std::string& spriteSheetName, const std::string& animationName) {
	spriteSheets[spriteSheetName]->unloadAnimation(animationName);
}

void SpriteLoader::clearSpriteSheets() {
	spriteSheets.clear();
}

void SpriteLoader::updateSpriteSheet(std::shared_ptr<SpriteSheet> spriteSheet) {
	spriteSheets[spriteSheet->getName()] = spriteSheet;
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

std::set<std::string> SpriteLoader::getLoadedSpriteSheetNamesAsSet() {
	std::set<std::string> results;

	for (std::pair<std::string, std::shared_ptr<SpriteSheet>> spriteSheet : spriteSheets) {
		results.insert(spriteSheet.first);
	}

	return results;
}

std::shared_ptr<SpriteSheet> SpriteLoader::getSpriteSheet(std::string spriteSheetName) const {
	return spriteSheets.at(spriteSheetName);
}

bool SpriteLoader::hasSpriteSheet(std::string spriteSheetName) const {
	return spriteSheets.find(spriteSheetName) != spriteSheets.end();
}

std::string SpriteLoader::formatPathToSpriteSheetImage(std::string imageFileNameWithExtension) {
	return format(RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH + "\\%s", levelPackName.c_str(), imageFileNameWithExtension.c_str());
}

std::string SpriteLoader::formatPathToSpriteSheetMetafile(std::string imageFileNameWithExtension) {
	return format(RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH + "\\%s%s", levelPackName.c_str(), imageFileNameWithExtension.c_str(), LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str());
}

bool SpriteLoader::loadSpriteSheet(const std::string& spriteSheetMetaFileName, const std::string& spriteSheetImageFileName) {
	// Sprite sheet name is the name of the image file
	std::shared_ptr<SpriteSheet> sheet = std::make_shared<SpriteSheet>(spriteSheetImageFileName);
	
	// Make sure image file exists
	if (!fileExists(formatPathToSpriteSheetImage(spriteSheetImageFileName))) {
		L_(lerror) << "Image file \"" << spriteSheetImageFileName << "\" does not exist.";

		return false;
	}

	std::ifstream metafile(format(RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH + "\\%s", levelPackName.c_str(), spriteSheetMetaFileName.c_str()));
	try {
		if (metafile) {
			// If metafile can be opened, load the SpriteSheet using it

			nlohmann::json j;
			metafile >> j;
			sheet->load(j);
		}

		// Load image file
		if (!sheet->loadTexture(formatPathToSpriteSheetImage(spriteSheetImageFileName))) {
			L_(lerror) << "Image file \"" << spriteSheetImageFileName << "\" failed to load.";

			// If image couldn't be loaded, mark the SpriteSheet as having a failed image load
			sheet->markFailedImageLoad();
		}

		spriteSheets[spriteSheetImageFileName] = sheet;
	} catch (const std::exception& ex) {
		L_(lerror) << "Exception when loading image file \"" << spriteSheetImageFileName << "\": " << ex.what();

		// If some exception occurred, mark the SpriteSheet as having a failed metafile load
		sheet->markFailedMetafileLoad();
		spriteSheets[spriteSheetImageFileName] = sheet;

		metafile.close();
		return false;
	}
	metafile.close();

	return true;
}

AnimationData::AnimationData() {
}

AnimationData::AnimationData(std::string animationName, std::vector<std::pair<float, std::string>> spriteNames)
	: animationName(animationName), spriteNames(spriteNames) {
}

AnimationData::AnimationData(const AnimationData& other) {
	load(other.format());
}

AnimationData::AnimationData(std::shared_ptr<AnimationData> other) {
	load(other->format());
}

std::string AnimationData::format() const {
	std::string result = formatString(animationName);
	for (std::pair<float, std::string> spriteName : spriteNames) {
		result += tos(spriteName.first) + formatString(spriteName.second);
	}
	return result;
}

void AnimationData::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	animationName = items.at(0);

	spriteNames.clear();
	for (int i = 1; i < items.size(); i += 2) {
		spriteNames.push_back(std::make_pair(std::stof(items.at(i)), items.at(i + 1)));
	}
}

nlohmann::json AnimationData::toJson() {
	nlohmann::json j = {
		{"animationName", animationName}
	};

	nlohmann::json spriteNamesJson;
	for (auto p : spriteNames) {
		spriteNamesJson.push_back(nlohmann::json{ {"duration", p.first}, {"spriteName", p.second} });
	}
	j["spriteNames"] = spriteNamesJson;

	return j;
}

void AnimationData::load(const nlohmann::json& j) {
	j.at("animationName").get_to(animationName);

	spriteNames.clear();
	if (j.contains("spriteNames")) {
		for (const nlohmann::json& item : j.at("spriteNames")) {
			float duration;
			std::string spriteName;

			item.at("duration").get_to(duration);
			item.at("spriteName").get_to(spriteName);

			spriteNames.push_back(std::make_pair(duration, spriteName));
		}
	}
}

bool AnimationData::operator==(const AnimationData & other) const {
	return animationName == other.animationName;
}

void AnimationData::setAnimationName(std::string animationName) {
	this->animationName = animationName;
}

void AnimationData::setSpriteNames(std::vector<std::pair<float, std::string>> spriteNames) {
	this->spriteNames = spriteNames;
}

void AnimationData::setSpriteInfo(int index, std::pair<float, std::string> newInfo) {
	spriteNames.at(index) = newInfo;
}

void AnimationData::setSpriteDuration(int index, float newDuration) {
	spriteNames.at(index).first = newDuration;
}

void AnimationData::setSpriteName(int index, std::string newName) {
	spriteNames.at(index).second = newName;
}

void AnimationData::insertSpriteInfo(int index, std::pair<float, std::string> newInfo) {
	spriteNames.insert(spriteNames.begin() + index, newInfo);
}

void AnimationData::removeSpriteEntry(int index) {
	spriteNames.erase(spriteNames.begin() + index);
}

std::string SpriteLoader::LoadMetrics::formatForUser() {
	return format("%d/%d sprite sheets failed to load.\n", spriteSheetsFailed, spriteSheetsTotal);
}

bool SpriteLoader::LoadMetrics::containsFailedLoads() {
	return spriteSheetsFailed > 0;
}

ComparableIntRect::ComparableIntRect() {
}

ComparableIntRect::ComparableIntRect(sf::IntRect rect) : sf::IntRect(rect) {
}
