#include "EditorMovablePoint.h"
#include "EditorMovablePointSpawnType.h"
#include "LevelPack.h"

std::string EditorMovablePoint::format() {
	std::string res = "";

	res += "(" + tos(id) + ")" + delim;
	res += "(" + tos(hitboxRadius) + ")" + delim;
	res += "(" + tos(despawnTime) + ")" + delim;

	res += "(" + tos(children.size()) + ")";
	for (auto emp : children) {
		res += delim + "(" + emp->format() + ")";
	}

	res += delim + "(" + tos(actions.size()) + ")";
	for (auto action : actions) {
		res += delim + "(" + action->format() + ")";
	}

	res += delim + "(" + spawnType->format() + ")";

	res += delim + "(" + tos(shadowTrailInterval) + ")";
	res += delim + "(" + tos(shadowTrailLifespan) + ")";

	res += delim + "(" + animatable.format() + ")";
	if (loopAnimation) {
		res += delim + "1";
	} else {
		res += delim + "0";
	}
	res += delim + "(" + baseSprite.format() + ")";
	res += delim + tos(damage);

	res += delim + tos(static_cast<int>(onCollisionAction));

	if (playSoundOnSpawn) {
		res += delim + "1";
	} else {
		res += delim + "0";
	}
	res += delim + "(" + soundSettings.format() + ")";
	
	res += delim + tos(bulletModelID);
	res += delim + (inheritRadius ? "1" : "0");
	res += delim + (inheritDespawnTime ? "1" : "0");
	res += delim + (inheritShadowTrailInterval ? "1" : "0");
	res += delim + (inheritShadowTrailLifespan ? "1" : "0");
	res += delim + (inheritAnimatables ? "1" : "0");
	res += delim + (inheritDamage ? "1" : "0");
	res += delim + (inheritOnCollisionAction ? "1" : "0");
	res += delim + (inheritSoundSettings ? "1" : "0");


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

	if (std::stoi(items[i++]) == 1) {
		playSoundOnSpawn = true;
	} else {
		playSoundOnSpawn = false;
	}
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
		playSoundOnSpawn = model->getPlaysSound();
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
		playSoundOnSpawn = model->getPlaysSound();
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

std::shared_ptr<EditorMovablePoint> EditorMovablePoint::createChild(std::shared_ptr<EMPSpawnType> spawnType) {
	std::shared_ptr<EditorMovablePoint> child = std::make_shared<EditorMovablePoint>(nextID, true);
	child->setSpawnType(spawnType);
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

std::string BulletModel::format() {
	std::string res = "";

	res += "(" + tos(id) + ")" + delim;
	res += "(" + tos(hitboxRadius) + ")" + delim;
	res += "(" + tos(despawnTime) + ")" + delim;

	res += "(" + tos(shadowTrailInterval) + ")" + delim;
	res += "(" + tos(shadowTrailLifespan) + ")" + delim;

	res += "(" + animatable.format() + ")" + delim;
	if (loopAnimation) {
		res += "1" + delim;
	} else {
		res += "0" + delim;
	}
	res += "(" + baseSprite.format() + ")" + delim;
	res += tos(damage) + delim;

	res += tos(static_cast<int>(onCollisionAction)) + delim;

	if (playSoundOnSpawn) {
		res += "1" + delim;
	} else {
		res += "0" + delim;
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

	onCollisionAction = static_cast<BULLET_ON_COLLISION_ACTION>(std::stoi(items[9]));

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
