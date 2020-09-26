#include <LevelPack/Attack.h>

#include <LevelPack/EditorMovablePoint.h>

EditorAttack::EditorAttack() {
}

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
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	id = std::stoi(items.at(0));
	name = items.at(1);
	bulletModelsCount.clear();
	empIDGen = IDGenerator();
	mainEMP = std::make_shared<EditorMovablePoint>(&empIDGen, false, &bulletModelsCount);
	mainEMP->load(items.at(2));
	playAttackAnimation = unformatBool(items.at(3));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EditorAttack::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LevelPackObject::LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	if (mainEMP) {
		auto mainEMPLegal = mainEMP->legal(levelPack, spriteLoader, symbolTables);
		if (mainEMPLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, mainEMPLegal.first);
			tabEveryLine(mainEMPLegal.second);
			messages.insert(messages.end(), mainEMPLegal.second.begin(), mainEMPLegal.second.end());
		}
	} else {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Missing mainEMP.");
	}
	return std::make_pair(status, messages);
}

void EditorAttack::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	mainEMP->compileExpressions(symbolTables);
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

const bool EditorAttack::usesBulletModel(int bulletModelID) const {
	return bulletModelsCount.find(bulletModelID) != bulletModelsCount.end() && bulletModelsCount.at(bulletModelID) > 0;
}

bool EditorAttack::getPlayAttackAnimation() const {
	return playAttackAnimation;
}

std::shared_ptr<EditorMovablePoint> EditorAttack::getMainEMP() const {
	return mainEMP;
}

IDGenerator* EditorAttack::getNextEMPID() {
	return &empIDGen;
}

std::map<int, int>* EditorAttack::getBulletModelIDsCount() {
	return &bulletModelsCount;
}

void EditorAttack::setPlayAttackAnimation(bool playAttackAnimation) {
	this->playAttackAnimation = playAttackAnimation;
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

bool EditorAttack::operator==(const EditorAttack& other) const {
	return empIDGen == other.empIDGen && id == other.id && name == other.name
		&& *mainEMP == *other.mainEMP && playAttackAnimation == other.playAttackAnimation
		// Compare bulletModelsCount
		&& bulletModelsCount.size() == other.bulletModelsCount.size()
		&& std::equal(bulletModelsCount.begin(), bulletModelsCount.end(), other.bulletModelsCount.begin());
}
