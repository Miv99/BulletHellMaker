#include "EditorMovablePoint.h"
#include "EditorMovablePointSpawnType.h"
#include "LevelPack.h"

EditorMovablePoint::EditorMovablePoint(int & nextID, bool setID) : nextID(nextID) {
	if (setID) {
		id = nextID++;
	}
	spawnType = std::make_shared<EntityRelativeEMPSpawn>(0.0f, 0.0f, 0.0f);
}

EditorMovablePoint::EditorMovablePoint(int & nextID, std::weak_ptr<EditorMovablePoint> parent) : nextID(nextID), parent(parent) {
	id = nextID++;
	spawnType = std::make_shared<EntityRelativeEMPSpawn>(0.0f, 0.0f, 0.0f);
}

std::string EditorMovablePoint::format() {
	std::string res = "";

	res += "(" + tos(id) + ")" + tm_delim;
	res += "(" + tos(hitboxRadius) + ")" + tm_delim;
	res += "(" + tos(despawnTime) + ")" + tm_delim;

	res += "(" + tos(children.size()) + ")";
	for (auto emp : children) {
		res += tm_delim + "(" + emp->format() + ")";
	}

	res += tm_delim + "(" + tos(actions.size()) + ")";
	for (auto action : actions) {
		res += tm_delim + "(" + action->format() + ")";
	}

	res += tm_delim + "(" + spawnType->format() + ")";

	res += tm_delim + "(" + tos(shadowTrailInterval) + ")";
	res += tm_delim + "(" + tos(shadowTrailLifespan) + ")";

	res += tm_delim + "(" + animatable.format() + ")";
	if (loopAnimation) {
		res += tm_delim + "1";
	} else {
		res += tm_delim + "0";
	}
	res += tm_delim + "(" + baseSprite.format() + ")";
	res += tm_delim + tos(damage);

	res += tm_delim + tos(static_cast<int>(onCollisionAction));
	res += tm_delim + tos(pierceResetTime);

	res += tm_delim + "(" + soundSettings.format() + ")";
	
	res += tm_delim + tos(bulletModelID);
	res += tm_delim + (inheritRadius ? "1" : "0");
	res += tm_delim + (inheritDespawnTime ? "1" : "0");
	res += tm_delim + (inheritShadowTrailInterval ? "1" : "0");
	res += tm_delim + (inheritShadowTrailLifespan ? "1" : "0");
	res += tm_delim + (inheritAnimatables ? "1" : "0");
	res += tm_delim + (inheritDamage ? "1" : "0");
	res += tm_delim + (inheritOnCollisionAction ? "1" : "0");
	res += tm_delim + (inheritSoundSettings ? "1" : "0");

	if (isBullet) {
		res += "1" + tm_delim;
	} else {
		res += "0" + tm_delim;
	}

	return res;
}

void EditorMovablePoint::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);

	id = std::stoi(items[0]);
	hitboxRadius = std::stof(items[1]);
	despawnTime = std::stof(items[2]);

	int i;
	for (i = 4; i < stoi(items[3]) + 4; i++) {
		std::shared_ptr<EditorMovablePoint> emp = std::make_shared<EditorMovablePoint>(nextID, shared_from_this());
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
	inheritRadius = (std::stoi(items[i++]) == 1);
	inheritDespawnTime = (std::stoi(items[i++]) == 1);
	inheritShadowTrailInterval = (std::stoi(items[i++]) == 1);
	inheritShadowTrailLifespan = (std::stoi(items[i++]) == 1);
	inheritAnimatables = (std::stoi(items[i++]) == 1);
	inheritDamage = (std::stoi(items[i++]) == 1);
	inheritOnCollisionAction = (std::stoi(items[i++]) == 1);
	inheritSoundSettings = (std::stoi(items[i++]) == 1);

	if (std::stoi(items[i++]) == 1) {
		isBullet = true;
	} else {
		isBullet = false;
	}
}

bool EditorMovablePoint::legal(SpriteLoader& spriteLoader, std::string & message) {
	bool good = true;
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
	if (hitboxRadius > 0) {
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
			if (animatable.isSprite()) {
				spriteLoader.getSprite(animatable.getAnimatableName(), animatable.getSpriteSheetName());
			} else {
				spriteLoader.getAnimation(animatable.getAnimatableName(), animatable.getSpriteSheetName(), false);
			}
		} catch (const char* str) {
			message += "\t" + std::string(str) + "\n";
			good = false;
		}
	}
	for (auto child : children) {
		if (!child->legal(spriteLoader, message)) {
			good = false;
		}
	}
	return good;
}

void EditorMovablePoint::dfsLoadBulletModel(const LevelPack & levelPack) {
	loadBulletModel(levelPack);
	for (auto emp : children) {
		emp->dfsLoadBulletModel(levelPack);
	}
}

void EditorMovablePoint::loadBulletModel(const LevelPack & levelPack) {
	if (bulletModelID < 0) return;

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
	if (inheritOnCollisionAction) onCollisionAction = model->getOnCollisionAction();
	if (inheritSoundSettings) {
		soundSettings = model->getSoundSettings();
	}
}

void EditorMovablePoint::setBulletModel(std::shared_ptr<BulletModel> model) {
	bulletModelID = model->getID();

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
	if (inheritOnCollisionAction) onCollisionAction = model->getOnCollisionAction();
	if (inheritSoundSettings) {
		soundSettings = model->getSoundSettings();
	}
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

std::shared_ptr<EditorMovablePoint> EditorMovablePoint::createChild() {
	std::shared_ptr<EditorMovablePoint> child = std::make_shared<EditorMovablePoint>(nextID, shared_from_this());
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

std::string BulletModel::format() {
	std::string res = "";

	res += "(" + tos(id) + ")" + tm_delim;
	res += "(" + tos(hitboxRadius) + ")" + tm_delim;
	res += "(" + tos(despawnTime) + ")" + tm_delim;

	res += "(" + tos(shadowTrailInterval) + ")" + tm_delim;
	res += "(" + tos(shadowTrailLifespan) + ")" + tm_delim;

	res += "(" + animatable.format() + ")" + tm_delim;
	if (loopAnimation) {
		res += "1" + tm_delim;
	} else {
		res += "0" + tm_delim;
	}
	res += "(" + baseSprite.format() + ")" + tm_delim;
	res += tos(damage) + tm_delim;

	if (playSoundOnSpawn) {
		res += "1" + tm_delim;
	} else {
		res += "0" + tm_delim;
	}
	res += "(" + soundSettings.format() + ")";

	return res;
}

void BulletModel::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);

	id = std::stoi(items[0]);
	hitboxRadius = std::stof(items[1]);
	despawnTime = std::stof(items[2]);

	shadowTrailInterval = std::stof(items[3]);
	shadowTrailLifespan = std::stof(items[4]);

	animatable.load(items[5]);
	if (std::stoi(items[6]) == 0) {
		loopAnimation = false;
	} else {
		loopAnimation = true;
	}
	baseSprite.load(items[7]);

	damage = std::stoi(items[8]);

	if (std::stoi(items[10]) == 1) {
		playSoundOnSpawn = true;
	} else {
		playSoundOnSpawn = false;
	}
	soundSettings.load(items[11]);
}

void BulletModel::onModelChange() {
	// Update all users' fields
	for (std::shared_ptr<EditorMovablePoint> emp : modelUsers) {
		emp->setBulletModel(shared_from_this());
	}
}
