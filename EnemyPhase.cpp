#include "EnemyPhase.h"

std::string EditorEnemyPhase::format() {
	std::string res = "";
	res += "(" + std::to_string(id) + ")" + tm_delim;
	res += "(" + name + ")" + tm_delim;
	res += "(" + phaseBeginAction->format() + ")" + tm_delim;
	res += "(" + phaseEndAction->format() + ")" + tm_delim;
	res += tos(attackPatternIds.size());
	for (auto p : attackPatternIds) {
		res += tm_delim + "(" + tos(p.first) + ")" + tm_delim + "(" + tos(p.second) + ")";
	}
	if (playMusic) {
		res += tm_delim + "1";
	} else {
		res += tm_delim + "0";
	}
	res += tm_delim + "(" + musicSettings.format() + ")";
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
	if (std::stoi(items[i++]) == 1) {
		playMusic = true;
	} else {
		playMusic = false;
	}
	musicSettings.load(items[i++]);
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
