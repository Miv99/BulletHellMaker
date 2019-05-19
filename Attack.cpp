#include "Attack.h"
#include "EditorMovablePoint.h"

EditorAttack::EditorAttack(int id) : id(id) {
	nextEMPID = 0;
	mainEMP = std::make_shared<EditorMovablePoint>(nextEMPID, true);
}

std::string EditorAttack::format() {
	std::string res = "";
	res += "(" + tos(id) + ")" + delim;
	res += "(" + tos(nextEMPID) + ")" + delim;
	res += "(" + name + ")" + delim;
	res += "(" + mainEMP->format() + ")" + delim;
	if (playAttackAnimation) {
		res += "1";
	} else {
		res += "0";
	}
	return res;
}

void EditorAttack::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = std::stoi(items[0]);
	nextEMPID = std::stoi(items[1]);
	name = items[2];
	mainEMP = std::make_shared<EditorMovablePoint>(nextEMPID, false);
	mainEMP->load(items[3]);
	if (items[4] == "1") {
		playAttackAnimation = true;
	} else {
		playAttackAnimation = false;
	}
}

bool EditorAttack::legal(SpriteLoader& spriteLoader, std::string& message) {
	bool good = true;
	if (contains(name, '(') || contains(name, ')')) {
		message += "Attack \"" + name + "\" cannot have the character '(' or ')' in its name\n";
		good = false;
	}
	if (mainEMP) {
		if (!mainEMP->legal(spriteLoader, message)) {
			good = false;
		}
	} else {
		message += "Attack \"" + name + "\" is missing its mainEMP\n";
	}
	//TODO: check that each sound file is a valid file
	return good;
}

void EditorAttack::loadEMPBulletModels(const LevelPack & levelPack) {
	mainEMP->dfsLoadBulletModel(levelPack);
}

void EditorAttack::executeAsEnemy(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t entity, float timeLag, int attackPatternID, int enemyID, int enemyPhaseID) {
	queue.pushBack(std::make_unique<EMPSpawnFromEnemyCommand>(registry, spriteLoader, mainEMP, entity, timeLag, id, attackPatternID, enemyID, enemyPhaseID, playAttackAnimation));
}

void EditorAttack::executeAsPlayer(EntityCreationQueue & queue, SpriteLoader & spriteLoader, entt::DefaultRegistry & registry, uint32_t entity, float timeLag, int attackPatternID) {
	queue.pushBack(std::make_unique<EMPSpawnFromPlayerCommand>(registry, spriteLoader, mainEMP, entity, timeLag, id, attackPatternID, playAttackAnimation));
}

float EditorAttack::searchLargestHitbox() const {
	return mainEMP->searchLargestHitbox();
}

std::shared_ptr<EditorMovablePoint> EditorAttack::searchEMP(int id) {
	if (mainEMP->getID() == id) {
		return mainEMP;
	}
	return mainEMP->searchID(id);
}
