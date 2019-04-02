#include "Level.h"

std::string Level::format() {
	std::string res = "";
	res += name + delim;
	res += "(" + player.format() + ")" + delim;
	res += tos(enemyGroups.size()) + delim;
	for (std::pair<std::shared_ptr<EnemySpawnCondition>, std::vector<EnemySpawnInfo>> p : enemyGroups) {
		res += "(" + p.first->format() + ")" + delim + tos(p.second.size()) + delim;
		for (EnemySpawnInfo info : p.second) {
			res += "(" + info.format() + ")" + delim;
		}
	}
	return res;
}

void Level::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	name = items[0];
	player.load(items[1]);
	for (int i = 3; i < std::stoi(items[2]) + 3; i++) {
		std::shared_ptr<EnemySpawnCondition> condition = EnemySpawnConditionFactory::create(items[i]);
		std::vector<EnemySpawnInfo> enemies;
		for (int a = 0; a < std::stoi(items[i + 1]); a++) {
			EnemySpawnInfo info;
			info.load(items[i + 2 + a]);
			enemies.push_back(info);
		}
		enemyGroups.push_back(std::make_pair(condition, enemies));
	}
}

bool Level::legal(std::string & message) {
	bool good = true;	
	//TODO
	return good;
}
