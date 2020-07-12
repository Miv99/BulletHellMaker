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
	for (auto t : attackIDs) {
		res += formatString(std::get<0>(t)) + tos(std::get<1>(t)) + formatTMObject(std::get<2>(t));
	}

	res += tos(actions.size());
	for (auto p : actions) {
		res += formatTMObject(*p);
	}

	res += formatString(shadowTrailInterval) + formatString(shadowTrailLifespan) + formatTMObject(symbolTable);

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
		ExprSymbolTable definer;
		definer.load(items[i + 2]);
		attackIDs.push_back(std::make_tuple(items[i], attackID, definer));

		if (attackIDCount.count(attackID) == 0) {
			attackIDCount[attackID] = 1;
		} else {
			attackIDCount[attackID]++;
		}
		i += 3;
	}

	int actionsSize = std::stoi(items[i]);
	int last = i + 1;
	actions.clear();
	for (i = last; i < actionsSize + last; i++) {
		actions.push_back(EMPActionFactory::create(items[i]));
	}

	shadowTrailInterval = items[i++];
	shadowTrailLifespan = items[i++];
	symbolTable.load(items[i++]);

	actionsTotalTime = 0;
	for (auto action : actions) {
		actionsTotalTime += action->getTime();
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EditorAttackPattern::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(shadowTrailInterval, shadow trail interval)
	LEGAL_CHECK_EXPRESSION(shadowTrailLifespan, shadow trail lifespan)

	int i = 0;
	for (auto t : attackIDs) {
		if (!expressionStrIsValid(parser, std::get<0>(t), symbolTables)) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back(std::string("Invalid expression for attack index " + std::to_string(i) + " start time."));
		}

		if (!levelPack.hasAttack(std::get<1>(t))) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back("Attack index " + std::to_string(i) + " uses a non-existent attack ID " + std::to_string(std::get<1>(t)) + ".");
		}

		i++;
	}

	i = 0;
	for (auto action : actions) {
		auto actionLegal = action->legal(levelPack, spriteLoader, symbolTables);
		if (actionLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, actionLegal.first);
			tabEveryLine(actionLegal.second);
			messages.push_back("Action index " + std::to_string(i) + " start condition:");
			messages.insert(messages.end(), actionLegal.second.begin(), actionLegal.second.end());
		}

		i++;
	}
	
	return std::make_pair(status, messages);
}

void EditorAttackPattern::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(shadowTrailInterval)
	COMPILE_EXPRESSION_FOR_FLOAT(shadowTrailLifespan)

	compiledAttackIDs.clear();
	for (auto t : attackIDs) {
		parser.compile(std::get<0>(t), expr);
		compiledAttackIDs.push_back(std::make_tuple(expr.value(), std::get<1>(t), std::get<2>(t).toLowerLevelSymbolTable(expr)));
	}
	// Keep it sorted ascending by time
	std::sort(compiledAttackIDs.begin(), compiledAttackIDs.end(), [](auto const& t1, auto const& t2) {
		return std::get<0>(t1) < std::get<0>(t2);
	});
}

void EditorAttackPattern::changeEntityPathToAttackPatternActions(EntityCreationQueue& queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	auto& pos = registry.get<PositionComponent>(entity);
	registry.replace<MovementPathComponent>(entity, queue, entity, registry, entity, std::make_shared<SpecificGlobalEMPSpawn>(0, pos.getX(), pos.getY()), actions, timeLag);
}

void EditorAttackPattern::addAttack(std::string time, int id, ExprSymbolTable attackSymbolsDefiner) {
	attackIDs.push_back(std::make_tuple(time, id, attackSymbolsDefiner));

	if (attackIDCount.count(id) == 0) {
		attackIDCount[id] = 1;
	} else {
		attackIDCount[id]++;
	}
}

void EditorAttackPattern::removeAttack(int index) {
	int attackID = std::get<1>(attackIDs[index]);
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