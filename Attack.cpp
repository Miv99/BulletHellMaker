#include "Attack.h"
#include "EditorMovablePoint.h"

EditorAttack::EditorAttack(int id) : id(id) {
	nextEMPID = 0;
	mainEMP = std::make_shared<EditorMovablePoint>(nextEMPID);
}

std::string EditorAttack::format() {
	if (contains(name, '(') || contains(name, ')')) {
		throw "Attack names cannot have the character '(' or ')'";
	}
	for (auto p : soundEffectNames) {
		if (contains(p.second, '(') || contains(p.second, ')')) {
			throw "Sound effect names cannot have the character '(' or ')'";
		}
	}

	std::string res = "";
	res += "(" + tos(id) + ")" + delim;
	res += "(" + name + ")" + delim;
	res += "(" + mainEMP->format() + ")" + delim;
	for (auto p : soundEffectNames) {
		res += delim + "(" + tos(p.first) + ")" + delim + "(" + p.second + ")";
	}
	return res;
}

void EditorAttack::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = stoi(items[0]);
	name = items[1];
	mainEMP = std::make_shared<EditorMovablePoint>(nextEMPID);
	mainEMP->load(items[2]);
	for (int i = 3; i < items.size(); i += 2) {
		soundEffectNames.push_back(std::make_pair(std::stof(items[i]), items[i + 1]));
	}
}

void EditorAttack::executeAsEnemy(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t entity, float timeLag, int attackPatternID, int enemyID, int enemyPhaseID) {
	queue.addCommand(std::make_unique<EMPSpawnFromEnemyCommand>(registry, spriteLoader, mainEMP, entity, timeLag, id, attackPatternID, enemyID, enemyPhaseID));
}

float EditorAttack::searchLargestHitbox() {
	return mainEMP->searchLargestHitbox();
}

std::shared_ptr<EditorMovablePoint> EditorAttack::searchEMP(int id) {
	if (mainEMP->getID() == id) {
		return mainEMP;
	}
	return mainEMP->searchID(id);
}
