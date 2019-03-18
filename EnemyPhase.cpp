#include "EnemyPhase.h"

std::string EditorEnemyPhase::format() {
	std::string res = "";
	res += "(" + std::to_string(id) + ")" + delim;
	res += "(" + name + ")" + delim;
	res += "(" + phaseBeginAction->format() + ")" + delim;
	res += "(" + phaseEndAction->format() + ")";
	for (auto p : attackPatternIds) {
		res += delim + "(" + tos(p.first) + ")" + delim + "(" + tos(p.second) + ")";
	}
	return res;
}

void EditorEnemyPhase::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = stoi(items[0]);
	name = items[1];
	phaseBeginAction = EPAFactory::create(items[2]);
	phaseEndAction = EPAFactory::create(items[3]);
	for (int i = 4; i < items.size(); i += 2) {
		attackPatternIds.push_back(std::make_pair(std::stof(items[i]), std::stoi(items[i + 1])));
	}
}

bool EditorEnemyPhase::legal(std::string & message) {
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

void EditorEnemyPhase::addAttackPatternID(float time, int id) {
	auto item = std::make_pair(time, id);
	attackPatternIds.insert(std::upper_bound(attackPatternIds.begin(), attackPatternIds.end(), item), item);
}
