#include <LevelPack/EditorMovablePoint.h>

#include <LevelPack/EditorMovablePointSpawnType.h>
#include <LevelPack/Attack.h>
#include <LevelPack/LevelPack.h>

EditorMovablePoint::EditorMovablePoint(IDGenerator* idGen, bool setID, std::map<int, int>* bulletModelsCount) 
	: idGen(idGen), bulletModelsCount(bulletModelsCount) {
	this->idGen = idGen;
	if (setID) {
		id = idGen->generateID();
		idResolved = true;
	} else {
		idResolved = false;
	}
	spawnType = std::make_shared<EntityRelativeEMPSpawn>("0", "0", "0");
}

EditorMovablePoint::EditorMovablePoint(IDGenerator* idGen, std::weak_ptr<EditorMovablePoint> parent, std::map<int, int>* bulletModelsCount, bool setID) 
	: idGen(idGen), parent(parent), bulletModelsCount(bulletModelsCount) {
	if (setID) {
		id = idGen->generateID();
		idResolved = true;
	} else {
		idResolved = false;
	}
	spawnType = std::make_shared<EntityRelativeEMPSpawn>("0", "0", "0");
}

EditorMovablePoint::EditorMovablePoint(std::shared_ptr<const EditorMovablePoint> copy) : idGen(copy->idGen), bulletModelsCount(copy->bulletModelsCount) {
	copyConstructorLoad(copy->format());
}

EditorMovablePoint::EditorMovablePoint(const EditorMovablePoint* copy) {
	copyConstructorLoad(copy->format());
}

std::shared_ptr<LevelPackObject> EditorMovablePoint::clone() const {
	return std::static_pointer_cast<LevelPackObject>(std::make_shared<EditorMovablePoint>(this));
}

std::string EditorMovablePoint::format() const {
	std::string res = "";

	res += tos(id) + formatString(hitboxRadius) + tos(despawnTime) + tos(children.size());
	for (auto emp : children) {
		res += formatTMObject(*emp);
	}

	res += tos(actions.size());
	for (auto action : actions) {
		res += formatTMObject(*action);
	}

	res += formatTMObject(*spawnType) + formatString(shadowTrailInterval) + formatString(shadowTrailLifespan) + formatTMObject(animatable)
		+ formatBool(loopAnimation) + formatTMObject(baseSprite) + formatString(damage) + tos(static_cast<int>(onCollisionAction))
		+ formatString(pierceResetTime) + formatTMObject(soundSettings) + tos(bulletModelID) + formatBool(inheritRadius)
		+ formatBool(inheritDespawnTime) + formatBool(inheritShadowTrailInterval) + formatBool(inheritShadowTrailLifespan)
		+ formatBool(inheritAnimatables) + formatBool(inheritDamage) + formatBool(inheritPierceResetTime) + formatBool(inheritSoundSettings) + formatBool(isBullet)
		+ formatTMObject(symbolTable);

	return res;
}

void EditorMovablePoint::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);

	// Delete IDs from idGen for if load() is overwriting some existing data
	recursiveDeleteID();
	id = std::stoi(items.at(0));
	idResolved = false;
	// Resolve any possible conflicts from loading this ID
	resolveIDConflicts(false);

	// Update bulletModelsCount
	updateBulletModelToBulletModelsCount(false);

	hitboxRadius = items.at(1);
	despawnTime = std::stof(items.at(2));

	int i;
	children.clear();
	for (i = 4; i < stoi(items.at(3)) + 4; i++) {
		std::shared_ptr<EditorMovablePoint> emp = std::make_shared<EditorMovablePoint>(idGen, weak_from_this(), bulletModelsCount, false);
		emp->load(items.at(i));
		children.push_back(emp);
	}

	int last = i;
	int actionsSize = stoi(items.at(i++));
	actions.clear();
	for (i = last + 1; i < actionsSize + last + 1; i++) {
		actions.push_back(EMPActionFactory::create(items.at(i)));
	}

	spawnType = EMPSpawnTypeFactory::create(items.at(i++));

	shadowTrailInterval = items.at(i++);
	shadowTrailLifespan = items.at(i++);

	animatable.load(items.at(i++));
	if (std::stoi(items.at(i++)) == 0) {
		loopAnimation = false;
	} else {
		loopAnimation = true;
	}
	baseSprite.load(items.at(i++));

	damage = items.at(i++);

	onCollisionAction = static_cast<BULLET_ON_COLLISION_ACTION>(std::stoi(items.at(i++)));
	pierceResetTime = items.at(i++);

	soundSettings.load(items.at(i++));

	setBulletModelID(std::stoi(items.at(i++)));
	inheritRadius = unformatBool(items.at(i++));
	inheritDespawnTime = unformatBool(items.at(i++));
	inheritShadowTrailInterval = unformatBool(items.at(i++));
	inheritShadowTrailLifespan = unformatBool(items.at(i++));
	inheritAnimatables = unformatBool(items.at(i++));
	inheritDamage = unformatBool(items.at(i++));
	inheritPierceResetTime = unformatBool(items.at(i++));
	inheritSoundSettings = unformatBool(items.at(i++));
	isBullet = unformatBool(items.at(i++));
	symbolTable.load(items.at(i++));
}

nlohmann::json EditorMovablePoint::toJson() {
	nlohmann::json j = {
		{"id", id},
		{"valueSymbolTable", symbolTable.toJson()},
		{"hitboxRadius", hitboxRadius},
		{"despawnTime", despawnTime},
		{"spawnType", spawnType->toJson()},
		{"shadowTrailInterval", shadowTrailInterval},
		{"shadowTrailLifespan", shadowTrailLifespan},
		{"animatable", animatable.toJson()},
		{"loopAnimation", loopAnimation},
		{"baseSprite", baseSprite.toJson()},
		{"damage", damage},
		{"onCollisionAction", onCollisionAction},
		{"pierceResetTime", pierceResetTime},
		{"soundSettings", soundSettings.toJson()},
		{"bulletModelID", bulletModelID},
		{"inheritRadius", inheritRadius},
		{"inheritDespawnTime", inheritDespawnTime},
		{"inheritShadowTrailInterval", inheritShadowTrailInterval},
		{"inheritShadowTrailLifespan", inheritShadowTrailLifespan},
		{"inheritAnimatables", inheritAnimatables},
		{"inheritDamage", inheritDamage},
		{"inheritPierceResetTime", inheritPierceResetTime},
		{"inheritSoundSettings", inheritSoundSettings},
		{"isBullet", isBullet}
	};

	nlohmann::json childrenJson;
	for (auto child : children) {
		childrenJson.push_back(child->toJson());
	}
	j["children"] = childrenJson;

	nlohmann::json actionsJson;
	for (auto action : actions) {
		actionsJson.push_back(action->toJson());
	}
	j["actions"] = actionsJson;

	return j;
}

void EditorMovablePoint::load(const nlohmann::json& j) {
	j.at("id").get_to(id);

	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	j.at("hitboxRadius").get_to(hitboxRadius);
	j.at("despawnTime").get_to(despawnTime);
	spawnType = EMPSpawnTypeFactory::create(j.at("spawnType"));
	j.at("shadowTrailInterval").get_to(shadowTrailInterval);
	j.at("shadowTrailLifespan").get_to(shadowTrailLifespan);

	if (j.contains("animatable")) {
		animatable.load(j.at("animatable"));
	} else {
		animatable = Animatable();
	}

	j.at("loopAnimation").get_to(loopAnimation);

	if (j.contains("baseSprite")) {
		baseSprite.load(j.at("baseSprite"));
	} else {
		baseSprite = Animatable();
	}

	j.at("damage").get_to(damage);
	j.at("onCollisionAction").get_to(onCollisionAction);
	j.at("pierceResetTime").get_to(pierceResetTime);

	if (j.contains("soundSettings")) {
		soundSettings.load(j.at("soundSettings"));
	} else {
		soundSettings = SoundSettings();
	}

	j.at("bulletModelID").get_to(bulletModelID);
	j.at("inheritRadius").get_to(inheritRadius);
	j.at("inheritDespawnTime").get_to(inheritDespawnTime);
	j.at("inheritShadowTrailInterval").get_to(inheritShadowTrailInterval);
	j.at("inheritShadowTrailLifespan").get_to(inheritShadowTrailLifespan);
	j.at("inheritAnimatables").get_to(inheritAnimatables);
	j.at("inheritDamage").get_to(inheritDamage);
	j.at("inheritPierceResetTime").get_to(inheritPierceResetTime);
	j.at("inheritSoundSettings").get_to(inheritSoundSettings);
	j.at("isBullet").get_to(isBullet);

	children.clear();
	if (j.contains("children")) {
		for (nlohmann::json childJson : j.at("children")) {
			std::shared_ptr<EditorMovablePoint> emp = std::make_shared<EditorMovablePoint>(idGen, weak_from_this(), bulletModelsCount, false);
			emp->load(childJson);
			children.push_back(emp);
		}
	}

	actions.clear();
	if (j.contains("actions")) {
		for (nlohmann::json actionJson : j.at("actions")) {
			actions.push_back(EMPActionFactory::create(actionJson));
		}
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EditorMovablePoint::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	// Put EMP info here instead of in EditorAttack because of the tree nature of EMPs making it hard to display illegal EMPs otherwise
	std::vector<std::string> messages = { "Movable point ID " + std::to_string(id) };

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(hitboxRadius, hitbox radius)
	LEGAL_CHECK_EXPRESSION(shadowTrailInterval, shadow interval)
	LEGAL_CHECK_EXPRESSION(shadowTrailLifespan, shadow lifespan)
	LEGAL_CHECK_EXPRESSION(damage, damage)
	LEGAL_CHECK_EXPRESSION(pierceResetTime, pierce reset time)

	if (actions.size() == 0) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("[" + std::to_string(id) + "] List of actions is empty.");
	} else {
		int i = 0;
		for (auto child : actions) {
			auto childLegal = child->legal(levelPack, spriteLoader, symbolTables);
			if (childLegal.first != LEGAL_STATUS::LEGAL) {
				status = std::max(status, childLegal.first);
				tabEveryLine(childLegal.second);
				messages.push_back("[" + std::to_string(id) + "] Action index " + std::to_string(i) + ":");
				messages.insert(messages.end(), childLegal.second.begin(), childLegal.second.end());
			}

			i++;
		}
	}
	if (!spawnType) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("[" + std::to_string(id) + "] Spawn type is missing.");
	}
	if (!animatable.isSprite() && !loopAnimation && !baseSprite.isSprite()) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("[" + std::to_string(id) + "] Base sprite is missing.");
	} else {
		// Make sure base sprite can be loaded
		if (!animatable.isSprite() && !loopAnimation) {
			try {
				if (baseSprite.isSprite()) {
					spriteLoader.getSprite(baseSprite.getAnimatableName(), baseSprite.getSpriteSheetName());
				} else {
					status = std::max(status, LEGAL_STATUS::ILLEGAL);
					messages.push_back("[" + std::to_string(id) + "] Base sprite is an animation.");
				}
			} catch (const std::exception& e) {
				status = std::max(status, LEGAL_STATUS::ILLEGAL);
				messages.push_back(std::string(e.what()));
			}
		}
	}

	// Make sure animatable can be loaded
	try {
		if (animatable.getAnimatableName() != "") {
			if (animatable.isSprite()) {
				spriteLoader.getSprite(animatable.getAnimatableName(), animatable.getSpriteSheetName());
			} else {
				spriteLoader.getAnimation(animatable.getAnimatableName(), animatable.getSpriteSheetName(), false);
			}
		}
	} catch (const std::exception& e) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back(std::string(e.what()));
	}
	for (auto child : children) {
		auto childLegal = child->legal(levelPack, spriteLoader, symbolTables);
		if (childLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, childLegal.first);
			// Don't tab the result messages or else the root EMP legal() messages will have a bunch of tabs

			messages.insert(messages.end(), childLegal.second.begin(), childLegal.second.end());
		}
	}

	// Make sure bullet model is valid
	if (usesBulletModel() && !levelPack.hasBulletModel(bulletModelID)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("[" + std::to_string(id) + "] Non-existent bullet model.");
	}

	return std::make_pair(status, messages);
}

void EditorMovablePoint::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(hitboxRadius)
	COMPILE_EXPRESSION_FOR_FLOAT(shadowTrailInterval)
	COMPILE_EXPRESSION_FOR_FLOAT(shadowTrailLifespan)
	COMPILE_EXPRESSION_FOR_INT(damage)
	COMPILE_EXPRESSION_FOR_FLOAT(pierceResetTime)

	spawnType->compileExpressions(symbolTables);

	for (auto action : actions) {
		action->compileExpressions(symbolTables);
	}

	for (auto child : children) {
		child->compileExpressions(symbolTables);
	}
}

void EditorMovablePoint::dfsLoadBulletModel(const LevelPack & levelPack) {
	loadBulletModel(levelPack);
	for (auto emp : children) {
		emp->dfsLoadBulletModel(levelPack);
	}
}

void EditorMovablePoint::loadBulletModel(const LevelPack & levelPack) {
	if (!usesBulletModel()) return;

	std::shared_ptr<BulletModel> model = levelPack.getBulletModel(bulletModelID);
	// Add this EMP to the model's set of model users
	model->addModelUser(shared_from_this());

	if (inheritRadius) hitboxRadius = std::to_string(model->getHitboxRadius());
	if (inheritDespawnTime) despawnTime = model->getDespawnTime();
	if (inheritShadowTrailInterval) shadowTrailInterval = std::to_string(model->getShadowTrailInterval());
	if (inheritShadowTrailLifespan) shadowTrailLifespan = std::to_string(model->getShadowTrailLifespan());
	if (inheritAnimatables) {
		animatable = model->getAnimatable();
		loopAnimation = model->getLoopAnimation();
		baseSprite = model->getBaseSprite();
	}
	if (inheritDamage) damage = std::to_string(model->getDamage());
	if (inheritPierceResetTime) pierceResetTime = std::to_string(model->getPierceResetTime());
	if (inheritSoundSettings) {
		soundSettings = model->getSoundSettings();
	}
}

void EditorMovablePoint::setBulletModel(std::shared_ptr<BulletModel> model) {
	setBulletModelID(model->getID());

	// Add this EMP to the model's set of model users
	model->addModelUser(shared_from_this());

	if (inheritRadius) hitboxRadius = std::to_string(model->getHitboxRadius());
	if (inheritDespawnTime) despawnTime = model->getDespawnTime();
	if (inheritShadowTrailInterval) shadowTrailInterval = std::to_string(model->getShadowTrailInterval());
	if (inheritShadowTrailLifespan) shadowTrailLifespan = std::to_string(model->getShadowTrailLifespan());
	if (inheritAnimatables) {
		animatable = model->getAnimatable();
		loopAnimation = model->getLoopAnimation();
		baseSprite = model->getBaseSprite();
	}
	if (inheritDamage) damage = std::to_string(model->getDamage());
	if (inheritPierceResetTime) pierceResetTime = std::to_string(model->getPierceResetTime());
	if (inheritSoundSettings) {
		soundSettings = model->getSoundSettings();
	}
}

void EditorMovablePoint::removeBulletModel() {
	setBulletModelID(-1);
}

float EditorMovablePoint::getTotalPathTime() const {
	float total = 0;
	for (auto action : actions) {
		total += action->getTime();
	}
	return total;
}

std::vector<int> EditorMovablePoint::getChildrenIDs() const {
	std::vector<int> ids;
	for (auto child : children) {
		getChildrenIDsHelper(ids);
	}
	return ids;
}

bool EditorMovablePoint::isMainEMP() const {
	// Since the main EMP of an EditorAttack is created when the EditorAttack is constructed and cannot be deleted,
	// the ID of the main EMP is always the first ID generated by IDGenerator
	return id == IDGenerator::FIRST_ID;
}

void EditorMovablePoint::setID(int id) {
	if (idResolved) {
		idResolved = false;
		idGen->deleteID(this->id);
	}
	this->id = id;
	resolveIDConflicts(false);
}

void EditorMovablePoint::setSpawnType(std::shared_ptr<EMPSpawnType> spawnType) {
	this->spawnType = spawnType;

	// If this EMP has a parent, re-insert this EMP into the parent so that
	// the list of children maintains order, since it is sorted by EMPSpawnType time
	if (!parent.expired()) {
		auto parentPtr = parent.lock();
		parentPtr->removeChild(id);
		parentPtr->addChild(shared_from_this());
	}
}

void EditorMovablePoint::setSpawnTypeTime(std::string time) {
	this->spawnType->setTime(time);

	// If this EMP has a parent, re-insert this EMP into the parent so that
	// the list of children maintains order, since it is sorted by EMPSpawnType time
	if (!parent.expired()) {
		auto parentPtr = parent.lock();
		parentPtr->removeChild(id);
		parentPtr->addChild(shared_from_this());
	}
}

void EditorMovablePoint::setInheritShadowTrailInterval(bool inheritShadowTrailInterval, const LevelPack& levelPack) {
	this->inheritShadowTrailInterval = inheritShadowTrailInterval; 
	loadBulletModel(levelPack);
}

void EditorMovablePoint::setInheritShadowTrailLifespan(bool inheritShadowTrailLifespan, const LevelPack& levelPack) {
	this->inheritShadowTrailLifespan = inheritShadowTrailLifespan;
	loadBulletModel(levelPack);
}

void EditorMovablePoint::setInheritRadius(bool inheritRadius, const LevelPack& levelPack) {
	this->inheritRadius = inheritRadius;
	loadBulletModel(levelPack);
}

void EditorMovablePoint::setInheritDespawnTime(bool inheritDespawnTime, const LevelPack& levelPack) {
	this->inheritDespawnTime = inheritDespawnTime;
	loadBulletModel(levelPack);
}

void EditorMovablePoint::setShadowTrailInterval(std::string shadowTrailInterval) {
	this->shadowTrailInterval = shadowTrailInterval;
}

void EditorMovablePoint::setShadowTrailLifespan(std::string shadowTrailLifespan) {
	this->shadowTrailLifespan = shadowTrailLifespan;
}

void EditorMovablePoint::setInheritAnimatables(bool inheritAnimatables, const LevelPack& levelPack) {
	this->inheritAnimatables = inheritAnimatables;
	loadBulletModel(levelPack);
}

void EditorMovablePoint::setInheritDamage(bool inheritDamage, const LevelPack& levelPack) {
	this->inheritDamage = inheritDamage;
	loadBulletModel(levelPack);
}

void EditorMovablePoint::setInheritPierceResetTime(bool inheritPierceResetTime, const LevelPack& levelPack) {
	this->inheritPierceResetTime = inheritPierceResetTime;
	loadBulletModel(levelPack);
}

void EditorMovablePoint::setInheritSoundSettings(bool inheritSoundSettings, const LevelPack& levelPack) {
	this->inheritSoundSettings = inheritSoundSettings;
	loadBulletModel(levelPack);
}

void EditorMovablePoint::insertAction(int index, std::shared_ptr<EMPAction> action) {
	actions.insert(actions.begin() + index, action);
}

void EditorMovablePoint::removeAction(int index) {
	actions.erase(actions.begin() + index);
}

void EditorMovablePoint::replaceAction(int index, std::shared_ptr<EMPAction> action) {
	actions[index] = action;
}

void EditorMovablePoint::removeChild(int id) {
	int pos = 0;
	for (pos = 0; pos < children.size(); pos++) {
		if (children[pos]->id == id) {
			break;
		}
	}
	children[pos]->recursiveDeleteID();
	children[pos]->removeBulletModelFromBulletModelsCount(true);
	children.erase(children.begin() + pos);
}

void EditorMovablePoint::detachFromParent() {
	if (!parent.expired()) {
		parent.lock()->removeChild(id);
	}
}

std::shared_ptr<EditorMovablePoint> EditorMovablePoint::createChild() {
	// Don't set ID for child because addChild() will take care of it
	std::shared_ptr<EditorMovablePoint> child = std::make_shared<EditorMovablePoint>(idGen, shared_from_this(), bulletModelsCount, false);
	addChild(child);
	return child;
}

void EditorMovablePoint::addChild(std::shared_ptr<EditorMovablePoint> child) {
	// Insert while maintaining order
	int indexToInsert = children.size();
	for (int i = 0; i < children.size(); i++) {
		if (child->getSpawnType()->getTime() <= children[i]->getSpawnType()->getTime()) {
			indexToInsert = i;
			break;
		}
	}
	children.insert(children.begin() + indexToInsert, child);
	child->resolveIDConflicts(true);
	child->updateBulletModelToBulletModelsCount(true);
	child->parent = shared_from_this();
}

void EditorMovablePoint::onNewParentEditorAttack(std::shared_ptr<EditorAttack> newAttack) {
	// Update the references to this EMP's parent EditorAttack values
	idGen = newAttack->getNextEMPID();
	bulletModelsCount = newAttack->getBulletModelIDsCount();

	// Update bulletModelsCount; no need to recurse because onNewParentEditorAttack() is recursive already
	updateBulletModelToBulletModelsCount(false);

	// Update idGen; no need to recurse because onNewParentEditorAttack() is recursive already
	resolveIDConflicts(false);

	for (auto child : children) {
		child->onNewParentEditorAttack(newAttack);
	}
}

int EditorMovablePoint::getTreeSize() const {
	int count = 1;
	for (auto child : children) {
		count += child->getTreeSize();
	}
	return count;
}

float EditorMovablePoint::searchLargestHitbox() const {
	float childrenMax = 0;
	for (auto emp : children) {
		childrenMax = std::max(childrenMax, emp->searchLargestHitbox());
	}
	return std::max(hitboxRadiusExprCompiledValue, childrenMax);
}

std::shared_ptr<EditorMovablePoint> EditorMovablePoint::searchID(int id) const {
	for (auto child : children) {
		if (child->id == id) {
			return child;
		}
		auto subresult = child->searchID(id);
		if (subresult != nullptr) {
			return subresult;
		}
	}
	return nullptr;
}

bool EditorMovablePoint::operator==(const EditorMovablePoint& other) const {
	if (children.size() != other.children.size()) {
		return false;
	}
	for (int i = 0; i < children.size(); i++) {
		if (!(*children[i] == *other.children[i])) {
			return false;
		}
	}
	if (actions.size() != other.actions.size()) {
		return false;
	}
	for (int i = 0; i < actions.size(); i++) {
		if (!(*actions[i] == *other.actions[i])) {
			return false;
		}
	}
	return id == other.id && isBullet == other.isBullet && hitboxRadius == other.hitboxRadius
		&& despawnTime == other.despawnTime && *spawnType == *other.spawnType
		&& shadowTrailInterval == other.shadowTrailInterval && shadowTrailLifespan == other.shadowTrailLifespan
		&& animatable == other.animatable && loopAnimation == other.loopAnimation
		&& baseSprite == other.baseSprite && damage == other.damage && onCollisionAction == other.onCollisionAction
		&& pierceResetTime == other.pierceResetTime && soundSettings == other.soundSettings
		&& bulletModelID == other.bulletModelID 
		&& inheritRadius == other.inheritRadius && inheritDespawnTime == other.inheritDespawnTime
		&& inheritShadowTrailInterval == other.inheritShadowTrailInterval && inheritShadowTrailLifespan == other.inheritShadowTrailLifespan
		&& inheritAnimatables == other.inheritAnimatables && inheritDamage == other.inheritDamage && inheritPierceResetTime == other.inheritPierceResetTime
		&& inheritSoundSettings == other.inheritSoundSettings
		&& bulletModelsCount->size() == other.bulletModelsCount->size()
		&& std::equal(bulletModelsCount->begin(), bulletModelsCount->end(), other.bulletModelsCount->begin());
}

void EditorMovablePoint::copyConstructorLoad(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);

	// Use some dummy object for now; idGen shouldn't need to be used until a real idGen is set by onNewParentEditorAttack()
	IDGenerator dummyID;
	idGen = &dummyID;
	// Same thing with bulletModelsCount
	std::map<int, int> dummyBulletModelsCount;
	bulletModelsCount = &dummyBulletModelsCount;

	id = -1;
	idResolved = false;
	hitboxRadius = items.at(1);
	despawnTime = std::stof(items.at(2));

	int i;
	children.clear();
	for (i = 4; i < stoi(items.at(3)) + 4; i++) {
		std::shared_ptr<EditorMovablePoint> emp = std::make_shared<EditorMovablePoint>(idGen, weak_from_this(), bulletModelsCount, false);
		emp->copyConstructorLoad(items.at(i));
		children.push_back(emp);
	}

	int last = i;
	int actionsSize = stoi(items.at(i++));
	actions.clear();
	for (i = last + 1; i < actionsSize + last + 1; i++) {
		actions.push_back(EMPActionFactory::create(items.at(i)));
	}

	spawnType = EMPSpawnTypeFactory::create(items.at(i++));

	shadowTrailInterval = items.at(i++);
	shadowTrailLifespan = items.at(i++);

	animatable.load(items.at(i++));
	if (std::stoi(items.at(i++)) == 0) {
		loopAnimation = false;
	} else {
		loopAnimation = true;
	}
	baseSprite.load(items.at(i++));

	damage = items.at(i++);

	onCollisionAction = static_cast<BULLET_ON_COLLISION_ACTION>(std::stoi(items.at(i++)));
	pierceResetTime = items.at(i++);

	soundSettings.load(items.at(i++));

	bulletModelID = std::stoi(items.at(i++));
	inheritRadius = unformatBool(items.at(i++));
	inheritDespawnTime = unformatBool(items.at(i++));
	inheritShadowTrailInterval = unformatBool(items.at(i++));
	inheritShadowTrailLifespan = unformatBool(items.at(i++));
	inheritAnimatables = unformatBool(items.at(i++));
	inheritDamage = unformatBool(items.at(i++));
	inheritPierceResetTime = unformatBool(items.at(i++));
	inheritSoundSettings = unformatBool(items.at(i++));
	isBullet = unformatBool(items.at(i++));
}

void EditorMovablePoint::getChildrenIDsHelper(std::vector<int>& arr) const {
	arr.push_back(id);
	for (auto child : children) {
		child->getChildrenIDsHelper(arr);
	}
}

void EditorMovablePoint::resolveIDConflicts(bool recurseOnChildren) {
	if (!idResolved) {
		if (id < 0 || idGen->idInUse(id)) {
			id = idGen->generateID();
		} else {
			idGen->markIDAsUsed(id);
		}
		idResolved = true;
	}

	if (recurseOnChildren) {
		for (auto child : children) {
			child->resolveIDConflicts(true);
		}
	}
}

void EditorMovablePoint::recursiveDeleteID() {
	if (idResolved) {
		idGen->deleteID(id);
		idResolved = false;


	}
	for (auto child : children) {
		child->recursiveDeleteID();
	}
}

void EditorMovablePoint::setBulletModelID(int newID) {
	// Remove the currently used bullet model ID from bulletModelsCount
	removeBulletModelFromBulletModelsCount(false);

	// Set the new ID
	bulletModelID = newID;

	// Update bulletModelsCount with the new ID
	updateBulletModelToBulletModelsCount(false);
}

void EditorMovablePoint::removeBulletModelFromBulletModelsCount(bool recurseOnChildren) {
	if (bulletModelID >= 0 && bulletModelsCount->count(bulletModelID) > 0) {
		bulletModelsCount->at(bulletModelID)--;
		if (bulletModelsCount->at(bulletModelID) == 0) {
			bulletModelsCount->erase(bulletModelID);
		}
	}

	if (recurseOnChildren) {
		for (auto child : children) {
			child->removeBulletModelFromBulletModelsCount(true);
		}
	}
}

void EditorMovablePoint::updateBulletModelToBulletModelsCount(bool recurseOnChildren) {
	if (bulletModelID >= 0) {
		if (bulletModelsCount->count(bulletModelID) == 0) {
			bulletModelsCount->emplace(bulletModelID, 1);
		} else {
			bulletModelsCount->at(bulletModelID)++;
		}
	}

	if (recurseOnChildren) {
		for (auto child : children) {
			child->updateBulletModelToBulletModelsCount(true);
		}
	}
}

BulletModel::BulletModel() {
}

BulletModel::BulletModel(int id) {
	this->id = id;
}

BulletModel::BulletModel(std::shared_ptr<const BulletModel> copy) {
	load(copy->format());
}

BulletModel::BulletModel(const BulletModel* copy) {
	load(copy->format());
}

std::shared_ptr<LevelPackObject> BulletModel::clone() const {
	return std::static_pointer_cast<LevelPackObject>(std::make_shared<BulletModel>(this));
}

std::string BulletModel::format() const {
	return tos(id) + formatString(name) + tos(hitboxRadius) + tos(despawnTime) + tos(shadowTrailInterval) + tos(shadowTrailLifespan)
		+ formatTMObject(animatable) + formatBool(loopAnimation) + formatTMObject(baseSprite) + tos(damage) + tos(pierceResetTime)
		+ formatBool(playSoundOnSpawn) + formatTMObject(soundSettings);
}

void BulletModel::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);

	id = std::stoi(items.at(0));
	name = items.at(1);
	hitboxRadius = std::stof(items.at(2));
	despawnTime = std::stof(items.at(3));

	shadowTrailInterval = std::stof(items.at(4));
	shadowTrailLifespan = std::stof(items.at(5));

	animatable.load(items.at(6));
	loopAnimation = unformatBool(items.at(7));
	baseSprite.load(items.at(8));

	damage = std::stoi(items.at(9));
	pierceResetTime = std::stof(items.at(10));

	playSoundOnSpawn = unformatBool(items.at(11));
	soundSettings.load(items.at(12));
}

nlohmann::json BulletModel::toJson() {
	return {
		{"id", id},
		{"name", name},
		{"hitboxRadius", hitboxRadius},
		{"despawnTime", despawnTime},
		{"shadowTrailInterval", shadowTrailInterval},
		{"shadowTrailLifespan", shadowTrailLifespan},
		{"animatable", animatable.toJson()},
		{"loopAnimation", loopAnimation},
		{"baseSprite", baseSprite.toJson()},
		{"damage", damage},
		{"pierceResetTime", pierceResetTime},
		{"playSoundOnSpawn", playSoundOnSpawn},
		{"soundSettings", soundSettings.toJson()}
	};
}

void BulletModel::load(const nlohmann::json& j) {
	j.at("id").get_to(id);
	j.at("name").get_to(name);
	j.at("hitboxRadius").get_to(hitboxRadius);
	j.at("despawnTime").get_to(despawnTime);
	j.at("shadowTrailInterval").get_to(shadowTrailInterval);
	j.at("shadowTrailLifespan").get_to(shadowTrailLifespan);

	if (j.contains("animatable")) {
		animatable.load(j.at("animatable"));
	} else {
		animatable = Animatable();
	}

	j.at("loopAnimation").get_to(loopAnimation);

	if (j.contains("baseSprite")) {
		baseSprite.load(j.at("baseSprite"));
	} else {
		baseSprite = Animatable();
	}

	j.at("damage").get_to(damage);
	j.at("pierceResetTime").get_to(pierceResetTime);
	j.at("playSoundOnSpawn").get_to(playSoundOnSpawn);

	if (j.contains("soundSettings")) {
		soundSettings.load(j.at("soundSettings"));
	} else {
		soundSettings = SoundSettings();
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> BulletModel::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	//TODO: legal
	return std::make_pair(LEGAL_STATUS::ILLEGAL, std::vector<std::string>());
}

void BulletModel::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	// Nothing to compile
}

void BulletModel::setName(std::string name) {
	this->name = name;
	onModelChange();
}

void BulletModel::setPlaysSound(bool playsSound) {
	playSoundOnSpawn = playsSound;
	onModelChange();
}

void BulletModel::setDamage(int damage) {
	this->damage = damage;
	onModelChange();
}

void BulletModel::setPierceResetTime(float pierceResetTime) {
	this->pierceResetTime = pierceResetTime;
	onModelChange();
}

void BulletModel::setAnimatable(Animatable animatable) {
	this->animatable = animatable;
	onModelChange();
}

void BulletModel::setLoopAnimation(bool loopAnimation) {
	this->loopAnimation = loopAnimation;
	onModelChange();
}

void BulletModel::setBaseSprite(Animatable baseSprite) { 
	this->baseSprite = baseSprite;
	onModelChange();
}

void BulletModel::setHitboxRadius(float hitboxRadius) { 
	this->hitboxRadius = hitboxRadius;
	onModelChange();
}

void BulletModel::setDespawnTime(float despawnTime) { 
	this->despawnTime = despawnTime; 
	onModelChange(); 
}

void BulletModel::setShadowTrailInterval(float shadowTrailInterval) { 
	this->shadowTrailInterval = shadowTrailInterval;
	onModelChange();
}

void BulletModel::setShadowTrailLifespan(float shadowTrailLifespan) { 
	this->shadowTrailLifespan = shadowTrailLifespan;
	onModelChange();
}

void BulletModel::onModelChange() {
	// Update all users' fields
	for (std::shared_ptr<EditorMovablePoint> emp : modelUsers) {
		emp->setBulletModel(shared_from_this());
	}
}
