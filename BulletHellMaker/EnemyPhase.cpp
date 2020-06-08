#include "EnemyPhase.h"
#include "LevelPack.h"

std::string EditorEnemyPhase::format() const {
	std::string res = tos(id) + formatString(name) + formatTMObject(*phaseBeginAction) + formatTMObject(*phaseEndAction) + tos(attackPatternIDs.size());
	for (auto p : attackPatternIDs) {
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

	attackPatternIDCount.clear();
	int i = 5;
	for (int a = 0; a < std::stoi(items[4]); a++) {
		int attackPatternID = std::stoi(items[i + 1]);
		attackPatternIDs.push_back(std::make_pair(std::stof(items[i]), attackPatternID));

		if (attackPatternIDCount.count(attackPatternID) == 0) {
			attackPatternIDCount[attackPatternID] = 1;
		} else {
			attackPatternIDCount[attackPatternID]++;

		}
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
	int size = attackPatternIDs.size();
	auto item = attackPatternIDs[index % size];
	// Increase time of the attack pattern at some index by the loop count multiplied by total time for all attack patterns to finish
	item.first += (attackPatternIDs[size - 1].first + levelPack.getAttackPattern(attackPatternIDs[size - 1].second)->getActionsTotalTime() + attackPatternLoopDelay) * (int)(index / size);
	return item;
}

void EditorEnemyPhase::addAttackPatternID(float time, int id) {
	auto item = std::make_pair(time, id);
	attackPatternIDs.insert(std::upper_bound(attackPatternIDs.begin(), attackPatternIDs.end(), item), item);

	if (attackPatternIDCount.count(id) == 0) {
		attackPatternIDCount[id] = 1;
	} else {
		attackPatternIDCount[id]++;
	}
}

void EditorEnemyPhase::removeAttackPattern(int index) {
	int attackPatternID = attackPatternIDs[index].second;
	attackPatternIDs.erase(attackPatternIDs.begin() + index);
	attackPatternIDCount[attackPatternID]--;
}
