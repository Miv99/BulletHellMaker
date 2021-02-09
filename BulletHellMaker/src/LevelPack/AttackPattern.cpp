#include <LevelPack/AttackPattern.h>

#include <Game/Components/PositionComponent.h>
#include <Game/Components/MovementPathComponent.h>
#include <LevelPack/EditorMovablePointSpawnType.h>

EditorAttackPattern::EditorAttackPattern() {
}

EditorAttackPattern::EditorAttackPattern(int id) {
	this->id = id;
}

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
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	id = std::stoi(items.at(0));
	name = items.at(1);

	int i = 3;
	attackIDs.clear();
	attackIDCount.clear();
	for (int a = 0; a < std::stoi(items.at(2)); a++) {
		int attackID = std::stoi(items.at(i + 1));
		ExprSymbolTable definer;
		definer.load(items.at(i + 2));
		attackIDs.push_back(std::make_tuple(items.at(i), attackID, definer));

		if (attackIDCount.find(attackID) == attackIDCount.end()) {
			attackIDCount[attackID] = 1;
		} else {
			attackIDCount[attackID]++;
		}
		i += 3;
	}

	int actionsSize = std::stoi(items.at(i));
	int last = i + 1;
	actions.clear();
	for (i = last; i < actionsSize + last; i++) {
		actions.push_back(EMPActionFactory::create(items.at(i)));
	}

	shadowTrailInterval = items.at(i++);
	shadowTrailLifespan = items.at(i++);
	symbolTable.load(items.at(i++));

	onActionsModified();
}

nlohmann::json EditorAttackPattern::toJson() {
	nlohmann::json j = {
		{"id", id},
		{"name", name},
		{"shadowTrailInterval", shadowTrailInterval},
		{"shadowTrailLifespan", shadowTrailLifespan},
		{"valueSymbolTable", symbolTable.toJson()}
	};

	nlohmann::json attackIDsJson;
	for (std::tuple<std::string, int, ExprSymbolTable> t : attackIDs) {
		attackIDsJson.push_back(nlohmann::json{ {"beginTime", std::get<0>(t)}, 
			{"attackID", std::get<1>(t)}, {"exprSymbolTable", std::get<2>(t).toJson()} });
	}
	j["attackIDs"] = attackIDsJson;

	nlohmann::json actionsJson;
	for (std::shared_ptr<EMPAction> action : actions) {
		actionsJson.push_back(action->toJson());
	}
	j["actions"] = actionsJson;

	j["shadowTrailInterval"] = shadowTrailInterval;

	return j;
}

void EditorAttackPattern::load(const nlohmann::json& j) {
	j.at("id").get_to(id);
	j.at("name").get_to(name);
	j.at("shadowTrailInterval").get_to(shadowTrailInterval);
	j.at("shadowTrailLifespan").get_to(shadowTrailLifespan);

	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	attackIDs.clear();
	attackIDCount.clear();
	if (j.contains("attackIDs")) {
		for (const nlohmann::json& item : j.at("attackIDs")) {
			std::string beginTime;
			int attackID;
			ExprSymbolTable exprSymbolTable;

			item.at("beginTime").get_to(beginTime);
			item.at("attackID").get_to(attackID);
			if (item.contains("exprSymbolTable")) {
				exprSymbolTable.load(item.at("exprSymbolTable"));
			}

			if (attackIDCount.find(attackID) == attackIDCount.end()) {
				attackIDCount[attackID] = 1;
			} else {
				attackIDCount[attackID]++;
			}

			attackIDs.push_back(std::make_tuple(beginTime, attackID, exprSymbolTable));
		}
	}

	actions.clear();
	if (j.contains("actions")) {
		for (nlohmann::json actionJson : j.at("actions")) {
			actions.push_back(EMPActionFactory::create(actionJson));
		}
	}

	onActionsModified();
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

std::vector<std::tuple<std::string, int, ExprSymbolTable>> EditorAttackPattern::getAttacks() {
	return attackIDs;
}

std::tuple<float, int, exprtk::symbol_table<float>> EditorAttackPattern::getAttackData(int index) const {
	return compiledAttackIDs[index];
}

std::shared_ptr<EMPAction> EditorAttackPattern::getAction(int index) const {
	return actions[index];
}

const std::vector<std::shared_ptr<EMPAction>> EditorAttackPattern::getActions() const {
	return actions;
}

int EditorAttackPattern::getAttacksCount() const {
	return attackIDs.size();
}

int EditorAttackPattern::getActionsCount() const {
	return actions.size();
}

float EditorAttackPattern::getShadowTrailInterval() const {
	return shadowTrailIntervalExprCompiledValue;
}

float EditorAttackPattern::getShadowTrailLifespan() const {
	return shadowTrailLifespanExprCompiledValue;
}

float EditorAttackPattern::getActionsTotalTime() const {
	return actionsTotalTime;
}

bool EditorAttackPattern::usesAttack(int attackID) const {
	return attackIDCount.find(attackID) != attackIDCount.end() && attackIDCount.at(attackID) > 0;
}

const std::map<int, int>* EditorAttackPattern::getAttackIDsCount() const {
	return &attackIDCount;
}

void EditorAttackPattern::setShadowTrailInterval(std::string shadowTrailInterval) {
	this->shadowTrailInterval = shadowTrailInterval;
}

void EditorAttackPattern::setShadowTrailLifespan(std::string shadowTrailLifespan) {
	this->shadowTrailLifespan = shadowTrailLifespan;
}

void EditorAttackPattern::setActions(std::vector<std::shared_ptr<EMPAction>> actions) {
	this->actions = actions;
	onActionsModified();
}

void EditorAttackPattern::setAttacks(std::vector<std::tuple<std::string, int, ExprSymbolTable>> attacks) {
	// Can clear attackIDCount here because it's affected by only attackIDs
	this->attackIDCount.clear();
	this->attackIDs.clear();

	for (auto t : attacks) {
		this->attackIDs.push_back(t);

		int id = std::get<1>(t);
		if (this->attackIDCount.find(id) == this->attackIDCount.end()) {
			this->attackIDCount[id] = 1;
		} else {
			this->attackIDCount[id]++;
		}
	}
}

void EditorAttackPattern::addAttack(std::string time, int id, ExprSymbolTable attackSymbolsDefiner) {
	attackIDs.push_back(std::make_tuple(time, id, attackSymbolsDefiner));

	if (attackIDCount.find(id) == attackIDCount.end()) {
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
	onActionsModified();
}

void EditorAttackPattern::removeAction(int index) {
	actions.erase(actions.begin() + index);
	onActionsModified();
}

void EditorAttackPattern::onActionsModified() {
	actionsTotalTime = 0;
	for (auto action : actions) {
		actionsTotalTime += action->getTime();
	}
}
