#include "Level.h"

std::string Level::format() const {
	std::string res = formatString(name) + tos(enemyGroups.size());
	for (std::pair<std::shared_ptr<EnemySpawnCondition>, std::vector<EnemySpawnInfo>> p : enemyGroups) {
		res += formatTMObject(*p.first) + tos(p.second.size());
		for (EnemySpawnInfo info : p.second) {
			res += formatTMObject(info);
		}
	}
	res += formatTMObject(*healthPack) + formatTMObject(*pointPack) + formatTMObject(*powerPack) + formatTMObject(*bombItem) + formatTMObject(musicSettings)
		+ formatString(backgroundFileName) + tos(backgroundScrollSpeedX) + tos(backgroundScrollSpeedY) + tos(backgroundTextureWidth)
		+ tos(backgroundTextureHeight) + tos(bossNameColor.r) + tos(bossNameColor.g) + tos(bossNameColor.b) + tos(bossNameColor.a)
		+ tos(bossHPBarColor.r) + tos(bossHPBarColor.g) + tos(bossHPBarColor.b) + tos(bossHPBarColor.a) + tos(bloomLayerSettings.size());
	for (auto settings : bloomLayerSettings) {
		res += formatTMObject(settings);
	}
	return res;
}

void Level::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	name = items[0];

	enemyIDCount.clear();
	int i;
	for (i = 2; i < std::stoi(items[1]) + 2;) {
		std::shared_ptr<EnemySpawnCondition> condition = EnemySpawnConditionFactory::create(items[i++]);
		std::vector<EnemySpawnInfo> enemies;
		int numEnemies = std::stoi(items[i++]);
		for (int a = 0; a < numEnemies; a++) {
			EnemySpawnInfo info;
			info.load(items[i++]);
			enemies.push_back(info);

			int enemyID = info.getEnemyID();
			if (enemyIDCount.count(enemyID) == 0) {
				enemyIDCount[enemyID] = 1;
			} else {
				enemyIDCount[enemyID]++;
			}
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
	backgroundTextureWidth = std::stof(items[i++]);
	backgroundTextureHeight = std::stof(items[i++]);
	bossNameColor = sf::Color(std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]));
	bossHPBarColor = sf::Color(std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]));
	bloomLayerSettings = std::vector<BloomSettings>(HIGHEST_RENDER_LAYER + 1, BloomSettings());
	int temp = i++;
	for (int a = 0; a < std::stoi(items[temp]); a++) {
		BloomSettings settings;
		settings.load(items[i++]);
		bloomLayerSettings[a] = settings;
	}
}

bool Level::legal(std::string & message) const {
	bool good = true;
	//TODO
	//TODO check packs' animatables can be opened
	return good;
}