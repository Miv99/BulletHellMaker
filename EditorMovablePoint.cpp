#include "EditorMovablePoint.h"
#include "EditorMovablePointSpawnType.h"

std::string EditorMovablePoint::format() {
	if (contains(spriteName, '(') || contains(spriteName, ')')) {
		throw "Sprite names cannot have the character '(' or ')'";
	}

	std::string res = "";

	res += "(" + spriteName + ")" + delim;
	res += "(" + spriteSheetName + ")" + delim;
	res += "(" + tos(hitboxRadius) + ")" + delim;

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

	return res;
}

void EditorMovablePoint::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);

	spriteName = items[0];
	spriteSheetName = items[1];
	hitboxRadius = std::stof(items[2]);

	int i;
	for (i = 4; i < stoi(items[3]) + 4; i++) {
		std::shared_ptr<EditorMovablePoint> emp = std::make_shared<EditorMovablePoint>(nextID, shared_from_this());
		emp->load(items[i]);
		children.push_back(emp);
	}

	int last = i + 1;
	int actionsSize = stoi(items[i]);
	for (i = last; i < actionsSize + last; i++) {
		actions.push_back(EMPActionFactory::create(items[i]));
	}

	spawnType = EMPSpawnTypeFactory::create(items[i++]);

	shadowTrailInterval = stoi(items[i++]);
	shadowTrailLifespan = stoi(items[i++]);
}

void EditorMovablePoint::setSpawnType(std::shared_ptr<EMPSpawnType> spawnType) {
	this->spawnType = spawnType;
}

void EditorMovablePoint::insertAction(int index, std::shared_ptr<EMPAction> action) {
	actions.insert(actions.begin() + index, action);
}

void EditorMovablePoint::removeAction(int index) {
	actions.erase(actions.begin() + index);
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
