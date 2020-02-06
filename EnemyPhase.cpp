#include "EnemyPhase.h"
#include "LevelPack.h"

std::string EditorEnemyPhase::format() const {
	std::string res = tos(id) + formatString(name) + formatTMObject(*phaseBeginAction) + formatTMObject(*phaseEndAction) + tos(attackPatternIds.size());
	for (auto p : attackPatternIds) {
		res += tos(p.first) + tos(p.second);
	}
	res += formatBool(playMusic) + formatTMObject(musicSettings);
	return res;
}

void EditorEnemyPhase::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = stoi(items[0]);
	name = items[1];
	phaseBeginAction = EPAFactory::create(items[2]);
	phaseEndAction = EPAFactory::create(items[3]);
	int i = 5;
	for (int a = 0; a < std::stoi(items[4]); a++) {
		attackPatternIds.push_back(std::make_pair(std::stof(items[i]), std::stoi(items[i + 1])));
		i += 2;
	}
	playMusic = unformatBool(items[i++]);
	musicSettings.load(items[i++]);
}

bool EditorEnemyPhase::legal(std::string & message) const {
	bool good = true;
	if (contains(name, '(') || contains(name, ')')) {
		message += "Enemy phase \"" + name + "\" cannot have the character '(' or ')' in its name\n";
		good = false;
	}
	if (attackPatternLoopDelay < 0) {
		message += "Enemy phase \"" + name + "\" must have a non-negative attack pattern loop delay\n";
		good = false;
	}
	return good;
}

std::pair<float, int> EditorEnemyPhase::getAttackPatternData(const LevelPack & levelPack, int index) const {
	int size = attackPatternIds.size();
	auto item = attackPatternIds[index % size];
	// Increase time of the attack pattern at some index by the loop count multiplied by total time for all attack patterns to finish
	item.first += (attackPatternIds[size - 1].first + levelPack.getAttackPattern(attackPatternIds[size - 1].second)->getActionsTotalTime() + attackPatternLoopDelay) * (int)(index / size);
	return item;
}

void EditorEnemyPhase::addAttackPatternID(float time, int id) {
	auto item = std::make_pair(time, id);
	attackPatternIds.insert(std::upper_bound(attackPatternIds.begin(), attackPatternIds.end(), item), item);
}
