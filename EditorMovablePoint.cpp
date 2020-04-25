#include "EditorMovablePoint.h"
#include "EditorMovablePointSpawnType.h"
#include "LevelPack.h"

EditorMovablePoint::EditorMovablePoint(int & nextID, bool setID, std::map<int, int>& bulletModelsCount) : nextID(nextID), bulletModelsCount(bulletModelsCount) {
	if (setID) {
		id = nextID++;
	}
	spawnType = std::make_shared<EntityRelativeEMPSpawn>(0.0f, 0.0f, 0.0f);

	// Update bulletModelsCount
	if (bulletModelID >= 0) {
		if (bulletModelsCount.count(bulletModelID) == 0) {
			bulletModelsCount[bulletModelID] = 1;
		} else {
			bulletModelsCount[bulletModelID]++;
		}
	}
}

EditorMovablePoint::EditorMovablePoint(int & nextID, std::weak_ptr<EditorMovablePoint> parent, std::map<int, int>& bulletModelsCount) : nextID(nextID), parent(parent), bulletModelsCount(bulletModelsCount) {
	id = nextID++;
	spawnType = std::make_shared<EntityRelativeEMPSpawn>(0.0f, 0.0f, 0.0f);

	// Update bulletModelsCount
	if (bulletModelID >= 0) {
		if (bulletModelsCount.count(bulletModelID) == 0) {
			bulletModelsCount[bulletModelID] = 1;
		} else {
			bulletModelsCount[bulletModelID]++;
		}
	}
}

std::string EditorMovablePoint::format() const {
	std::string res = "";

	res += tos(id) + tos(hitboxRadius) + tos(despawnTime) + tos(children.size());
	for (auto emp : children) {
		res += formatTMObject(*emp);
	}

	res += tos(actions.size());
	for (auto action : actions) {
		res += formatTMObject(*action);
	}

	res += formatTMObject(*spawnType) + tos(shadowTrailInterval) + tos(shadowTrailLifespan) + formatTMObject(animatable)
		+ formatBool(loopAnimation) + formatTMObject(baseSprite) + tos(damage) + tos(static_cast<int>(onCollisionAction))
		+ tos(pierceResetTime) + formatTMObject(soundSettings) + tos(bulletModelID) + formatBool(inheritRadius)
		+ formatBool(inheritDespawnTime) + formatBool(inheritShadowTrailInterval) + formatBool(inheritShadowTrailLifespan)
		+ formatBool(inheritAnimatables) + formatBool(inheritDamage) + formatBool(inheritSoundSettings) + formatBool(isBullet);

	return res;
}

void EditorMovablePoint::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);

	id = std::stoi(items[0]);
	hitboxRadius = std::stof(items[1]);
	despawnTime = std::stof(items[2]);

	int i;
	for (i = 4; i < stoi(items[3]) + 4; i++) {
		std::shared_ptr<EditorMovablePoint> emp = std::make_shared<EditorMovablePoint>(nextID, shared_from_this(), bulletModelsCount);
		emp->load(items[i]);
		children.push_back(emp);
	}

	int last = i;
	int actionsSize = stoi(items[i++]);
	for (i = last + 1; i < actionsSize + last + 1; i++) {
		actions.push_back(EMPActionFactory::create(items[i]));
	}

	spawnType = EMPSpawnTypeFactory::create(items[i++]);

	shadowTrailInterval = std::stof(items[i++]);
	shadowTrailLifespan = std::stof(items[i++]);

	animatable.load(items[i++]);
	if (std::stoi(items[i++]) == 0) {
		loopAnimation = false;
	} else {
		loopAnimation = true;
	}
	baseSprite.load(items[i++]);

	damage = std::stoi(items[i++]);

	onCollisionAction = static_cast<BULLET_ON_COLLISION_ACTION>(std::stoi(items[i++]));
	pierceResetTime = std::stof(items[i++]);

	soundSettings.load(items[i++]);

	bulletModelID = std::stoi(items[i++]);
	inheritRadius = unformatBool(items[i++]);
	inheritDespawnTime = unformatBool(items[i++]);
	inheritShadowTrailInterval = unformatBool(items[i++]);
	inheritShadowTrailLifespan = unformatBool(items[i++]);
	inheritAnimatables = unformatBool(items[i++]);
	inheritDamage = unformatBool(items[i++]);
	inheritSoundSettings = unformatBool(items[i++]);
	isBullet = unformatBool(items[i++]);

	// Update bulletModelsCount
	if (bulletModelID > 0) {
		if (bulletModelsCount.count(bulletModelID) == 0) {
			bulletModelsCount[bulletModelID] = 1;
		} else {
			bulletModelsCount[bulletModelID]++;
		}
	}
}

std::pair<bool, std::string> EditorMovablePoint::legal(LevelPack & levelPack, SpriteLoader & spriteLoader) const {
	bool good = true;
	std::string message = "";
	if (actions.size() == 0) {
		// Add a tab to show that this EMP is from the parent attack
		message += "\tMovablePoint id " + tos(id) + " must not have an empty list of actions\n";
		good = false;
	}
	if (!spawnType) {
		message += "\tMovablePoint id " + tos(id) + " is missing a spawn type\n";
		good = false;
	}
	if (shadowTrailInterval < 0 && shadowTrailLifespan != 0) {
		message += "\tMovablePoint id " + tos(id) + " has a negative shadow trail interval\n";
		good = false;
	}
	if (!animatable.isSprite() && !loopAnimation && !baseSprite.isSprite()) {
		message += "\tMovablePoint id " + tos(id) + " is missing a base sprite\n";
		good = false;
	} else {
		// Make sure base sprite can be loaded
		if (!animatable.isSprite() && !loopAnimation) {
			try {
				if (baseSprite.isSprite()) {
					spriteLoader.getSprite(baseSprite.getAnimatableName(), baseSprite.getSpriteSheetName());
				} else {
					message += "\tMovablePoint id " + tos(id) + " has an animation as a base sprite\n";
					good = false;
				}
			} catch (const char* str) {
				message += "\t" + std::string(str) + "\n";
				good = false;
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
	} catch (const char* str) {
		message += "\t" + std::string(str) + "\n";
		good = false;
	}
	for (auto child : children) {
		auto childLegal = child->legal(levelPack, spriteLoader);
		if (!childLegal.first) {
			good = false;
			message += tabEveryLine(childLegal.second);
		}
	}

	// Make sure bullet model is valid
	if (usesBulletModel() && !levelPack.hasBulletModel(bulletModelID)) {
		message += "\tMovablePoint id " + tos(id) + " is using a deleted bullet model\n";
		good = false;
	}

	return std::make_pair(good, message);
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

	if (inheritRadius) hitboxRadius = model->getHitboxRadius();
	if (inheritDespawnTime) despawnTime = model->getDespawnTime();
	if (inheritShadowTrailInterval) shadowTrailInterval = model->getShadowTrailInterval();
	if (inheritShadowTrailLifespan) shadowTrailLifespan = model->getShadowTrailLifespan();
	if (inheritAnimatables) {
		animatable = model->getAnimatable();
		loopAnimation = model->getLoopAnimation();
		baseSprite = model->getBaseSprite();
	}
	if (inheritDamage) damage = model->getDamage();
	if (inheritSoundSettings) {
		soundSettings = model->getSoundSettings();
	}
}

void EditorMovablePoint::setBulletModel(std::shared_ptr<BulletModel> model) {
	// Decrement bulletModelsCount of old model, if any
	if (bulletModelID >= 0) {
		bulletModelsCount[bulletModelID]--;
	}

	bulletModelID = model->getID();

	// Update bulletModelsCount
	if (bulletModelsCount.count(bulletModelID) == 0) {
		bulletModelsCount[bulletModelID] = 1;
	} else {
		bulletModelsCount[bulletModelID]++;
	}

	// Add this EMP to the model's set of model users
	model->addModelUser(shared_from_this());

	if (inheritRadius) hitboxRadius = model->getHitboxRadius();
	if (inheritDespawnTime) despawnTime = model->getDespawnTime();
	if (inheritShadowTrailInterval) shadowTrailInterval = model->getShadowTrailInterval();
	if (inheritShadowTrailLifespan) shadowTrailLifespan = model->getShadowTrailLifespan();
	if (inheritAnimatables) {
		animatable = model->getAnimatable();
		loopAnimation = model->getLoopAnimation();
		baseSprite = model->getBaseSprite();
	}
	if (inheritDamage) damage = model->getDamage();
	if (inheritSoundSettings) {
		soundSettings = model->getSoundSettings();
	}
}

void EditorMovablePoint::removeBulletModel() {
	// Decrement bulletModelsCount of old model, if any
	if (bulletModelID >= 0) {
		bulletModelsCount[bulletModelID]--;
	}

	bulletModelID = -1;
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
	children.erase(children.begin() + pos);
}

void EditorMovablePoint::detachFromParent() {
	if (!parent.expired()) {
		parent.lock()->removeChild(id);
	}
}

std::shared_ptr<EditorMovablePoint> EditorMovablePoint::createChild(bool addToChildrenList) {
	std::shared_ptr<EditorMovablePoint> child = std::make_shared<EditorMovablePoint>(nextID, shared_from_this(), bulletModelsCount);
	if (addToChildrenList) {
		addChild(child);
	}
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
}

std::vector<std::vector<sf::String>> EditorMovablePoint::generateTreeViewEmpHierarchy(std::function<sf::String(const EditorMovablePoint&)> nodeText, std::vector<sf::String> pathToThisEmp) {
	pathToThisEmp.push_back(nodeText(*this));
	if (children.size() == 0) {
		return {pathToThisEmp};
	} else {
		std::vector<std::vector<sf::String>> ret;
		for (auto child : children) {
			std::vector<std::vector<sf::String>> childTree = child->generateTreeViewEmpHierarchy(nodeText, pathToThisEmp);
			ret.insert(ret.end(), childTree.begin(), childTree.end());
		}
		return ret;
	}
}

std::vector<sf::String> EditorMovablePoint::generatePathToThisEmp(std::function<sf::String(const EditorMovablePoint&)> nodeText) {
	std::vector<sf::String> ret;
	ret.push_back(nodeText(*this));
	std::shared_ptr<EditorMovablePoint> cur = parent.lock();
	while (cur) {
		ret.insert(ret.begin(), nodeText(*cur));
		cur = cur->parent.lock();
	}

	return ret;
}

std::string BulletModel::format() const {
	return tos(id) + formatString(name) + tos(hitboxRadius) + tos(despawnTime) + tos(shadowTrailInterval) + tos(shadowTrailLifespan)
		+ formatTMObject(animatable) + formatBool(loopAnimation) + formatTMObject(baseSprite) + tos(damage) + formatBool(playSoundOnSpawn)
		+ formatTMObject(soundSettings);
}

void BulletModel::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);

	id = std::stoi(items[0]);
	name = items[1];
	hitboxRadius = std::stof(items[2]);
	despawnTime = std::stof(items[3]);

	shadowTrailInterval = std::stof(items[4]);
	shadowTrailLifespan = std::stof(items[5]);

	animatable.load(items[6]);
	loopAnimation = unformatBool(items[7]);
	baseSprite.load(items[8]);

	damage = std::stoi(items[9]);

	playSoundOnSpawn = unformatBool(items[10]);
	soundSettings.load(items[11]);
}

void BulletModel::onModelChange() {
	// Update all users' fields
	for (std::shared_ptr<EditorMovablePoint> emp : modelUsers) {
		emp->setBulletModel(shared_from_this());
	}
}
