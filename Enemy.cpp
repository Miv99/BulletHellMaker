#include "Enemy.h"

std::string EditorEnemy::format() {
	std::string res = "";
	res += "(" + std::to_string(id) + ")" + delim;
	res += "(" + name + ")" + delim;
	res += "(" + std::to_string(hitboxRadius) + ")" + delim;
	res += "(" + std::to_string(health) + ")";
	for (auto t : phaseIDs) {
		res += delim + "(" + std::get<0>(t)->format() + ")" + delim + "(" + tos(std::get<1>(t)) + ")" + delim + "(" + std::get<2>(t).format() + ")";
	}
	return res;
}

void EditorEnemy::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = stoi(items[0]);
	name = items[1];
	hitboxRadius = stof(items[2]);
	health = stof(items[3]);
	for (int i = 4; i < items.size(); i += 3) {
		EntityAnimatableSet animatableSet;
		animatableSet.load(items[i + 2]);
		phaseIDs.push_back(std::make_tuple(EnemyPhaseStartConditionFactory::create(items[i]), std::stoi(items[i + 1]), animatableSet));
	}
}

bool EditorEnemy::legal(std::string& message) {
	bool good = true;
	if (contains(name, '(') || contains(name, ')')) {
		message += "Enemy \"" + name + "\" cannot have the character '(' or ')' in its name\n";
		good = false;
	}
	if (hitboxRadius < 0) {
		message += "Enemy \"" + name + "\" has a negative hitbox radius\n";
		good = false;
	}
	if (health < 0) {
		message += "Enemy \"" + name + "\" has a negative max health\n";
		good = false;
	}
	if (phaseIDs.size() == 0) {
		message += "Enemy \"" + name + "\" must not have an empty list of phases\n";
		good = false;
	} else {
		auto startCondition = std::get<0>(phaseIDs[0]);
		if (!std::dynamic_pointer_cast<TimeBasedEnemyPhaseStartCondition>(startCondition)) {
			message += "Enemy \"" + name + "\"'s first phase start condition must be time-based with t=0\n";
			good = false;
		} else if (std::dynamic_pointer_cast<TimeBasedEnemyPhaseStartCondition>(startCondition)->getTime() != 0) {
			message += "Enemy \"" + name + "\"'s first phase start condition must be time-based with t=0\n";
			good = false;
		}
	}
	for (auto t : phaseIDs) {
		if (std::get<2>(t).getAttackAnimatable().isSprite()) {
			message += "Enemy \"" + name + "\" cannot have a sprite as an attack animation.\n";
		}
	}
	return good;
}
