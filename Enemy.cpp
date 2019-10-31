#include "Enemy.h"

std::string EditorEnemy::format() const {
	std::string res = "";
	res += "(" + tos(id) + ")" + tm_delim;
	res += "(" + name + ")" + tm_delim;
	res += "(" + tos(hitboxRadius) + ")" + tm_delim;
	res += "(" + tos(health) + ")" + tm_delim;
	res += "(" + tos(despawnTime) + ")" + tm_delim;
	res += "(" + tos(phaseIDs.size()) + ")";
	for (auto t : phaseIDs) {
		res += tm_delim + "(" + std::get<0>(t)->format() + ")" + tm_delim + "(" + tos(std::get<1>(t)) + ")" + tm_delim + "(" + std::get<2>(t).format() + ")";
	}
	res += tm_delim + "(" + tos(deathActions.size()) + ")";
	for (auto action : deathActions) {
		res += tm_delim + "(" + action->format() + ")";
	}
	if (isBoss) {
		res += tm_delim + "1";
	} else {
		res += tm_delim + "0";
	}
	res += tm_delim + "(" + hurtSound.format() + ")";
	res += tm_delim + "(" + deathSound.format() + ")";
	return res;
}

void EditorEnemy::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = std::stoi(items[0]);
	name = items[1];
	hitboxRadius = std::stof(items[2]);
	health = std::stof(items[3]);
	despawnTime = std::stof(items[4]);
	int i = 6;
	for (int a = 0; a < std::stoi(items[5]); a++) {
		EntityAnimatableSet animatableSet;
		animatableSet.load(items[i + 2]);
		phaseIDs.push_back(std::make_tuple(EnemyPhaseStartConditionFactory::create(items[i]), std::stoi(items[i + 1]), animatableSet));
		i += 3;
	}
	int next = i;
	for (i = next + 1; i < std::stoi(items[next]) + next + 1; i++) {
		deathActions.push_back(DeathActionFactory::create(items[i]));
	}
	if (std::stoi(items[i++])) {
		isBoss = true;
	} else {
		isBoss = false;
	}
	hurtSound.load(items[i++]);
	deathSound.load(items[i++]);
}

bool EditorEnemy::legal(std::string& message) const {
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
