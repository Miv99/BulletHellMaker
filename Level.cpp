#include "Level.h"

std::string Level::format() {
	std::string res = "";
	res += name + delim;
	res += tos(enemyGroups.size()) + delim;
	for (std::pair<std::shared_ptr<EnemySpawnCondition>, std::vector<EnemySpawnInfo>> p : enemyGroups) {
		res += "(" + p.first->format() + ")" + delim + tos(p.second.size()) + delim;
		for (EnemySpawnInfo info : p.second) {
			res += "(" + info.format() + ")" + delim;
		}
	}
	res += "(" + healthPack->format() + ")" + delim;
	res += "(" + pointPack->format() + ")" + delim;
	res += "(" + powerPack->format() + ")" + delim;
	res += "(" + musicSettings.format() + ")" + delim;
	res += "(" + backgroundFileName + ")" + delim + tos(backgroundScrollSpeedX) + delim + tos(backgroundScrollSpeedY);
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
	healthPack = std::make_shared<HealthPackItem>();
	healthPack->load(items[i++]);
	pointPack = std::make_shared<PointsPackItem>();
	pointPack->load(items[i++]);
	powerPack = std::make_shared<PowerPackItem>();
	powerPack->load(items[i++]);
	musicSettings.load(items[i++]);
	backgroundFileName = items[i++];
	backgroundScrollSpeedX = std::stof(items[i++]);
	backgroundScrollSpeedY = std::stof(items[i++]);
}

bool Level::legal(std::string & message) {
	bool good = true;	
	//TODO
	//TODO check packs' animatables can be opened
	return good;
}
