#include "AttackPattern.h"
#include "Components.h"
#include "EditorMovablePointSpawnType.h"

std::string EditorAttackPattern::format() {
	if (contains(name, '(') || contains(name, ')')) {
		throw "Attack names cannot have the character '(' or ')'";
	}

	std::string res = "";
	res += "(" + tos(id) + ")" + delim;
	res += "(" + name + ")" + delim;

	res += "(" + tos(attackIDs.size()) + ")" + delim;
	for (auto p : attackIDs) {
		res += "(" + tos(p.first) + ")" + delim + "(" + tos(p.second) + ")" + delim;
	}

	res += "(" + tos(actions.size()) + ")";
	for (auto p : actions) {
		res += delim + "(" + p->format() + ")";
	}
	return res;
}

void EditorAttackPattern::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = std::stoi(items[0]);
	name = items[1];

	int i = 3;
	for (int a = 0; a < std::stoi(items[2]); a++) {
		attackIDs.push_back(std::make_pair(std::stof(items[i]), std::stoi(items[i + 1])));
		i += 2;
	}

	int actionsSize = std::stoi(items[i]);
	int last = i + 1;
	for (i = last; i < actionsSize + last; i++) {
		actions.push_back(EMPActionFactory::create(items[i]));
	}
}

void EditorAttackPattern::changeEntityPathToAttackPatternActions(entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	auto& pos = registry.get<PositionComponent>(entity);
	registry.replace<MovementPathComponent>(entity, registry, entity, std::make_shared<SpecificGlobalEMPSpawn>(0, pos.getX(), pos.getY()), actions, timeLag);
}

void EditorAttackPattern::addAttackID(float time, int id) {
	auto item = std::make_pair(time, id);
	attackIDs.insert(std::upper_bound(attackIDs.begin(), attackIDs.end(), item), item);
}

void EditorAttackPattern::insertAction(int index, std::shared_ptr<EMPAction> action) {
	actions.insert(actions.begin() + index, action);
}

void EditorAttackPattern::removeAction(int index) {
	actions.erase(actions.begin() + index);
}