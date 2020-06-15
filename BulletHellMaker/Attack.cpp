#include "Attack.h"
#include "EditorMovablePoint.h"

EditorAttack::EditorAttack(int id) {
	this->id = id;
	mainEMP = std::make_shared<EditorMovablePoint>(&empIDGen, true, &bulletModelsCount);
}

EditorAttack::EditorAttack(std::shared_ptr<const EditorAttack> copy) {
	// Since EMPs are unique to the EditorAttack and new EMP objects
	// are created in load(), we can just load from the copy's format
	// and there won't be any object conflicts
	load(copy->format());
}

EditorAttack::EditorAttack(const EditorAttack* copy) {
	// Since EMPs are unique to the EditorAttack and new EMP objects
	// are created in load(), we can just load from the copy's format
	// and there won't be any object conflicts
	load(copy->format());
}

std::shared_ptr<LevelPackObject> EditorAttack::clone() const {
	return std::static_pointer_cast<LevelPackObject>(std::make_shared<EditorAttack>(this));
}

std::string EditorAttack::format() const {
	return tos(id) + formatString(name) + formatTMObject(*mainEMP) + formatBool(playAttackAnimation);
}

void EditorAttack::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = std::stoi(items[0]);
	name = items[1];
	bulletModelsCount.clear();
	empIDGen = IDGenerator();
	mainEMP = std::make_shared<EditorMovablePoint>(&empIDGen, false, &bulletModelsCount);
	mainEMP->load(items[2]);
	playAttackAnimation = unformatBool(items[3]);
}

std::pair<bool, std::string> EditorAttack::legal(LevelPack & levelPack, SpriteLoader & spriteLoader) const {
	bool good = true;
	std::string message = "";
	if (contains(name, '(') || contains(name, ')') || contains(name, '\\')) {
		message += "Attack \"" + name + "\" cannot have the character '(', ')', or '\' in its name\n";
		good = false;
	}
	if (mainEMP) {
		auto mainEMPLegal = mainEMP->legal(levelPack, spriteLoader);
		if (!mainEMPLegal.first) {
			good = false;
			message += tabEveryLine(mainEMPLegal.second);
		}
	} else {
		message += "Attack \"" + name + "\" is missing its mainEMP\n";
	}
	return std::make_pair(good, message);
}

void EditorAttack::loadEMPBulletModels(const LevelPack & levelPack) {
	mainEMP->dfsLoadBulletModel(levelPack);
}

void EditorAttack::executeAsEnemy(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t entity, float timeLag, int attackPatternID, int enemyID, int enemyPhaseID) const {
	queue.pushBack(std::make_unique<EMPSpawnFromEnemyCommand>(registry, spriteLoader, mainEMP, true, entity, timeLag, id, attackPatternID, enemyID, enemyPhaseID, playAttackAnimation));
}

void EditorAttack::executeAsPlayer(EntityCreationQueue & queue, SpriteLoader & spriteLoader, entt::DefaultRegistry & registry, uint32_t entity, float timeLag, int attackPatternID) const {
	queue.pushBack(std::make_unique<EMPSpawnFromPlayerCommand>(registry, spriteLoader, mainEMP, true, entity, timeLag, id, attackPatternID, playAttackAnimation));
}

float EditorAttack::searchLargestHitbox() const {
	return mainEMP->searchLargestHitbox();
}

std::shared_ptr<EditorMovablePoint> EditorAttack::searchEMP(int id) const {
	if (mainEMP->getID() == id) {
		return mainEMP;
	}
	return mainEMP->searchID(id);
}

std::vector<std::vector<sf::String>> EditorAttack::generateTreeViewHierarchy(std::function<sf::String(const EditorAttack&)> attackText, std::function<sf::String(const EditorMovablePoint&)> empText) const {
	std::string thisAttackText = attackText(*this);
	auto empTree = mainEMP->generateTreeViewEmpHierarchy(empText, {});
	for (std::vector<sf::String>& v : empTree) {
		v.insert(v.begin(), thisAttackText);
	}
	return empTree;
}

bool EditorAttack::operator==(const EditorAttack& other) const {
	return empIDGen == other.empIDGen && id == other.id && name == other.name
		&& *mainEMP == *other.mainEMP && playAttackAnimation == other.playAttackAnimation
		// Compare bulletModelsCount
		&& bulletModelsCount.size() == other.bulletModelsCount.size()
		&& std::equal(bulletModelsCount.begin(), bulletModelsCount.end(), other.bulletModelsCount.begin());
}