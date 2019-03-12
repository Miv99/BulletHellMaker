#include "Enemy.h"

std::string EditorEnemy::format() {
	if (contains(name, '(') || contains(name, ')')) {
		throw "Enemy names cannot have the character '(' or ')'";
	}
	if (contains(spriteName, '(') || contains(spriteName, ')')) {
		throw "Sprite names cannot have the character '(' or ')'";
	}
	if (contains(spriteSheetName, '(') || contains(spriteSheetName, ')')) {
		throw "Sprite sheet names cannot have the character '(' or ')'";
	}

	std::string res = "";
	res += "(" + std::to_string(id) + ")" + delim;
	res += "(" + name + ")" + delim;
	res += "(" + spriteName + ")" + delim;
	res += "(" + spriteSheetName + ")" + delim;
	res += "(" + std::to_string(hitboxRadius) + ")" + delim;
	res += "(" + std::to_string(health) + ")";
	for (auto p : phaseIDs) {
		res += delim + "(" + p.first->format() + ")" + delim + "(" + std::to_string(p.second) + ")";
	}
	return res;
}

void EditorEnemy::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = stoi(items[0]);
	name = items[1];
	spriteName = items[2];
	spriteSheetName = items[3];
	hitboxRadius = stof(items[4]);
	health = stof(items[5]);
	for (int i = 6; i < items.size(); i += 2) {
		phaseIDs.push_back(std::make_pair(EnemyPhaseStartConditionFactory::create(items[i]), std::stoi(items[i + 1])));
	}
}