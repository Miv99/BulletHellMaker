#include "AttackPattern.h"
#include "Components.h"
#include "EditorMovablePointSpawnType.h"

EditorAttackPattern::EditorAttackPattern(std::shared_ptr<const EditorAttackPattern> copy) {
	load(copy->format());
}

EditorAttackPattern::EditorAttackPattern(const EditorAttackPattern* copy) {
	load(copy->format());
}

std::shared_ptr<LevelPackObject> EditorAttackPattern::clone() const {
	return std::static_pointer_cast<LevelPackObject>(std::make_shared<EditorAttackPattern>(this));
}

std::string EditorAttackPattern::format() const {
	std::string res = tos(id) + formatString(name) + tos(attackIDs.size());
	for (auto p : attackIDs) {
		res += tos(p.first) + tos(p.second);
	}

	res += tos(actions.size());
	for (auto p : actions) {
		res += formatTMObject(*p);
	}

	res += tos(shadowTrailInterval);
	res += tos(shadowTrailLifespan);

	return res;
}

void EditorAttackPattern::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = std::stoi(items[0]);
	name = items[1];

	int i = 3;
	attackIDs.clear();
	attackIDCount.clear();
	for (int a = 0; a < std::stoi(items[2]); a++) {
		int attackID = std::stoi(items[i + 1]);
		attackIDs.push_back(std::make_pair(std::stof(items[i]), attackID));

		if (attackIDCount.count(attackID) == 0) {
			attackIDCount[attackID] = 1;
		} else {
			attackIDCount[attackID]++;
		}
		i += 2;
	}

	int actionsSize = std::stoi(items[i]);
	int last = i + 1;
	actions.clear();
	for (i = last; i < actionsSize + last; i++) {
		actions.push_back(EMPActionFactory::create(items[i]));
	}

	shadowTrailInterval = std::stof(items[i++]);
	shadowTrailLifespan = std::stof(items[i++]);

	actionsTotalTime = 0;
	for (auto action : actions) {
		actionsTotalTime += action->getTime();
	}
}

std::pair<bool, std::string> EditorAttackPattern::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	//TODO: implement this
	return std::pair<bool, std::string>();
}

bool EditorAttackPattern::legal(std::string & message) const {
	bool good = true;
	if (contains(name, '(') || contains(name, ')')) {
		message += "Attack pattern \"" + name + "\" cannot have the character '(' or ')' in its name\n";
		good = false;
	}
	if (actions.size() == 0) {
		message += "Attack pattern \"" + name + "\" cannot have an empty list of actions\n";
		good = false;
	}
	return good;
}

void EditorAttackPattern::changeEntityPathToAttackPatternActions(EntityCreationQueue& queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	auto& pos = registry.get<PositionComponent>(entity);
	//registry.get<MovementPathComponent>(entity).setActions(queue, registry, entity, actions);
	registry.replace<MovementPathComponent>(entity, queue, entity, registry, entity, std::make_shared<SpecificGlobalEMPSpawn>(0, pos.getX(), pos.getY()), actions, timeLag);
}

void EditorAttackPattern::addAttack(float time, int id) {
	auto item = std::make_pair(time, id);
	attackIDs.insert(std::upper_bound(attackIDs.begin(), attackIDs.end(), item), item);

	if (attackIDCount.count(id) == 0) {
		attackIDCount[id] = 1;
	} else {
		attackIDCount[id]++;
	}
}

void EditorAttackPattern::removeAttack(int index) {
	int attackID = attackIDs[index].second;
	attackIDs.erase(attackIDs.begin() + index);
	attackIDCount[attackID]--;
}

void EditorAttackPattern::insertAction(int index, std::shared_ptr<EMPAction> action) {
	actions.insert(actions.begin() + index, action);

	// Recalculate completely just in case of floating point imprecision
	actionsTotalTime = 0;
	for (auto action : actions) {
		actionsTotalTime += action->getTime();
	}
}

void EditorAttackPattern::removeAction(int index) {
	actions.erase(actions.begin() + index);

	// Recalculate completely just in case of floating point imprecision
	actionsTotalTime = 0;
	for (auto action : actions) {
		actionsTotalTime += action->getTime();
	}
}