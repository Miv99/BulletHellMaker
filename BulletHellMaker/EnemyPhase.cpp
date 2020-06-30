#include "EnemyPhase.h"
#include "LevelPack.h"

EditorEnemyPhase::EditorEnemyPhase(std::shared_ptr<const EditorEnemyPhase> copy) {
	load(copy->format());
}

EditorEnemyPhase::EditorEnemyPhase(const EditorEnemyPhase* copy) {
	load(copy->format());
}

std::shared_ptr<LevelPackObject> EditorEnemyPhase::clone() const {
	return std::static_pointer_cast<LevelPackObject>(std::make_shared<EditorEnemyPhase>(this));
}

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

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EditorEnemyPhase::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	//TODO: legal
	return std::make_pair(LEGAL_STATUS::ILLEGAL, std::vector<std::string>());
}

void EditorEnemyPhase::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	// TODO: compileExpressions
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
