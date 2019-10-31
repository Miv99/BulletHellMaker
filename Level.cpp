#include "Level.h"

std::string Level::format() const {
	std::string res = "";
	res += name + tm_delim;
	res += tos(enemyGroups.size()) + tm_delim;
	for (std::pair<std::shared_ptr<EnemySpawnCondition>, std::vector<EnemySpawnInfo>> p : enemyGroups) {
		res += "(" + p.first->format() + ")" + tm_delim + tos(p.second.size()) + tm_delim;
		for (EnemySpawnInfo info : p.second) {
			res += "(" + info.format() + ")" + tm_delim;
		}
	}
	res += "(" + healthPack->format() + ")" + tm_delim;
	res += "(" + pointPack->format() + ")" + tm_delim;
	res += "(" + powerPack->format() + ")" + tm_delim;
	res += "(" + bombItem->format() + ")" + tm_delim;
	res += "(" + musicSettings.format() + ")" + tm_delim;
	res += "(" + backgroundFileName + ")" + tm_delim + tos(backgroundScrollSpeedX) + tm_delim + tos(backgroundScrollSpeedY) + tm_delim;
	res += tos(bossNameColor.r) + tm_delim + tos(bossNameColor.g) + tm_delim + tos(bossNameColor.b) + tm_delim + tos(bossNameColor.a) + tm_delim;
	res += tos(bossHPBarColor.r) + tm_delim + tos(bossHPBarColor.g) + tm_delim + tos(bossHPBarColor.b) + tm_delim + tos(bossHPBarColor.a) = tm_delim;
	res += tos(bloomLayerSettings.size());
	for (auto settings : bloomLayerSettings) {
		res += tm_delim + "(" + settings.format() + ")";
	}
	return res;
}

void Level::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	name = items[0];
	int i;
	for (i = 2; i < std::stoi(items[1]) + 2;) {
		std::shared_ptr<EnemySpawnCondition> condition = EnemySpawnConditionFactory::create(items[i++]);
		std::vector<EnemySpawnInfo> enemies;
		int numEnemies = std::stoi(items[i++]);
		for (int a = 0; a < numEnemies; a++) {
			EnemySpawnInfo info;
			info.load(items[i++]);
			enemies.push_back(info);
		}
		enemyGroups.push_back(std::make_pair(condition, enemies));
	}
	if (!healthPack) healthPack = std::make_shared<HealthPackItem>();
	healthPack->load(items[i++]);
	if (!pointPack) pointPack = std::make_shared<PointsPackItem>();
	pointPack->load(items[i++]);
	if (!powerPack) powerPack = std::make_shared<PowerPackItem>();
	powerPack->load(items[i++]);
	if (!bombItem) bombItem = std::make_shared<BombItem>();
	bombItem->load(items[i++]);
	musicSettings.load(items[i++]);
	backgroundFileName = items[i++];
	backgroundScrollSpeedX = std::stof(items[i++]);
	backgroundScrollSpeedY = std::stof(items[i++]);
	bossNameColor = sf::Color(std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]));
	bossHPBarColor = sf::Color(std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]));
	bloomLayerSettings = std::vector<BloomSettings>(HIGHEST_RENDER_LAYER + 1, BloomSettings());
	for (int a = 0; a < std::stoi(items[i++]); a++) {
		BloomSettings settings;
		settings.load(items[i++]);
		bloomLayerSettings.push_back(settings);
	}
}

bool Level::legal(std::string & message) const {
	bool good = true;	
	//TODO
	//TODO check packs' animatables can be opened
	return good;
}
