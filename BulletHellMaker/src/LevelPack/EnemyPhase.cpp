#include <LevelPack/EnemyPhase.h>

#include <LevelPack/LevelPack.h>

EditorEnemyPhase::EditorEnemyPhase() {
}

EditorEnemyPhase::EditorEnemyPhase(int id) {
	this->id = id;
}

EditorEnemyPhase::EditorEnemyPhase(std::shared_ptr<const EditorEnemyPhase> copy) {
	load(copy->format());
}

EditorEnemyPhase::EditorEnemyPhase(const EditorEnemyPhase* copy) {
	load(copy->format());
}

std::shared_ptr<LevelPackObject> EditorEnemyPhase::clone() const {
	return std::static_pointer_cast<LevelPackObject>(std::make_shared<EditorEnemyPhase>(this));
}

std::string EditorEnemyPhase::format() const {
	std::string res = tos(id) + formatString(name) + formatTMObject(*phaseBeginAction) + formatTMObject(*phaseEndAction) + formatString(attackPatternLoopDelay) + tos(attackPatternIDs.size());
	for (auto t : attackPatternIDs) {
		res += formatString(std::get<0>(t)) + tos(std::get<1>(t)) + formatTMObject(std::get<2>(t));
	}
	res += formatBool(playMusic) + formatTMObject(musicSettings) + formatTMObject(symbolTable);
	return res;
}

void EditorEnemyPhase::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	id = stoi(items.at(0));
	name = items.at(1);
	phaseBeginAction = EPAFactory::create(items.at(2));
	phaseEndAction = EPAFactory::create(items.at(3));
	attackPatternLoopDelay = items.at(4);

	attackPatternIDs.clear();
	attackPatternIDCount.clear();
	int i = 6;
	for (int a = 0; a < std::stoi(items.at(5)); a++) {
		int attackPatternID = std::stoi(items.at(i + 1));
		ExprSymbolTable definer;
		definer.load(items.at(i + 2));
		attackPatternIDs.push_back(std::make_tuple(items.at(i), attackPatternID, definer));

		if (attackPatternIDCount.find(attackPatternID) == attackPatternIDCount.end()) {
			attackPatternIDCount[attackPatternID] = 1;
		} else {
			attackPatternIDCount[attackPatternID]++;
		}
		i += 3;
	}
	playMusic = unformatBool(items.at(i++));
	musicSettings.load(items.at(i++));
	symbolTable.load(items.at(i++));
}

nlohmann::json EditorEnemyPhase::toJson() {
	nlohmann::json j = {
		{"id", id},
		{"name", name},
		{"valueSymbolTable", symbolTable.toJson()},
		{"attackPatternLoopDelay", attackPatternLoopDelay},
		{"phaseBeginAction", phaseBeginAction->toJson()},
		{"phaseEndAction", phaseEndAction->toJson()},
		{"playMusic", playMusic},
		{"musicSettings", musicSettings.toJson()}
	};

	nlohmann::json attackPatternIDsJson;
	for (auto t : attackPatternIDs) {
		attackPatternIDsJson.push_back(nlohmann::json{ {"beginTime", std::get<0>(t)},
			{"attackPatternID", std::get<1>(t)}, {"exprSymbolTable", std::get<2>(t).toJson()} });
	}
	j["attackPatternIDs"] = attackPatternIDsJson;

	return j;
}

void EditorEnemyPhase::load(const nlohmann::json& j) {
	j.at("id").get_to(id);
	j.at("name").get_to(name);

	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	j.at("attackPatternLoopDelay").get_to(attackPatternLoopDelay);
	phaseBeginAction = EPAFactory::create(j.at("phaseBeginAction"));
	phaseEndAction = EPAFactory::create(j.at("phaseEndAction"));
	j.at("playMusic").get_to(playMusic);

	if (j.contains("musicSettings")) {
		musicSettings.load(j.at("musicSettings"));
	} else {
		musicSettings = MusicSettings();
	}

	attackPatternIDs.clear();
	attackPatternIDCount.clear();
	if (j.contains("attackPatternIDs")) {
		for (const nlohmann::json& item : j.at("attackPatternIDs")) {
			std::string beginTime;
			int attackPatternID;
			ExprSymbolTable exprSymbolTable;

			item.at("beginTime").get_to(beginTime);
			item.at("attackPatternID").get_to(attackPatternID);
			if (item.contains("exprSymbolTable")) {
				exprSymbolTable.load(item.at("exprSymbolTable"));
			}

			if (attackPatternIDCount.find(attackPatternID) == attackPatternIDCount.end()) {
				attackPatternIDCount[attackPatternID] = 1;
			} else {
				attackPatternIDCount[attackPatternID]++;
			}

			attackPatternIDs.push_back(std::make_tuple(beginTime, attackPatternID, exprSymbolTable));
		}
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EditorEnemyPhase::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(attackPatternLoopDelay, attack pattern loop delay)

	int i = 0;
	for (auto t : attackPatternIDs) {
		if (!expressionStrIsValid(parser, std::get<0>(t), symbolTables)) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back(std::string("Invalid expression for attack pattern index " + std::to_string(i) + " start time."));
		}

		if (!levelPack.hasAttackPattern(std::get<1>(t))) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back("Attack pattern index " + std::to_string(i) + " uses a non-existent attack pattern ID " + std::to_string(std::get<1>(t)) + ".");
		}
		i++;
	}

	// phaseBeginAction and phaseEndAction cannot be illegal, so no need to do legal check

	// TODO: legal check musicSettings

	return std::make_pair(status, messages);
}

void EditorEnemyPhase::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(attackPatternLoopDelay)

	compiledAttackPatternIDs.clear();
	for (auto t : attackPatternIDs) {
		parser.compile(std::get<0>(t), expr);
		compiledAttackPatternIDs.push_back(std::make_tuple(expr.value(), std::get<1>(t), std::get<2>(t).toLowerLevelSymbolTable(expr)));
	}
	// Keep it sorted ascending by time
	std::sort(compiledAttackPatternIDs.begin(), compiledAttackPatternIDs.end(), [](auto const& t1, auto const& t2) {
		return std::get<0>(t1) < std::get<0>(t2);
	});
}

std::tuple<float, int, exprtk::symbol_table<float>> EditorEnemyPhase::getAttackPatternData(const LevelPack & levelPack, int index) {
	int size = compiledAttackPatternIDs.size();
	auto item = compiledAttackPatternIDs[index % size];
	if (!lastAttackPatternActionsTotalTimeCalculated) {
		lastAttackPatternActionsTotalTime = levelPack.getGameplayAttackPattern(std::get<1>(compiledAttackPatternIDs[size - 1]), std::get<2>(compiledAttackPatternIDs[size - 1]))->getActionsTotalTime();
		lastAttackPatternActionsTotalTimeCalculated = true;
	}
	// Increase time of the attack pattern at some index by the loop count multiplied by total time for all attack patterns to finish
	std::get<0>(item) += (std::get<0>(compiledAttackPatternIDs[size - 1]) + lastAttackPatternActionsTotalTime + attackPatternLoopDelayExprCompiledValue) * (int)(index / size);
	return item;
}

const std::map<int, int>* EditorEnemyPhase::getAttackPatternsIDCount() const {
	return &attackPatternIDCount;
}

bool EditorEnemyPhase::usesAttackPattern(int attackPatternID) const {
	return attackPatternIDCount.find(attackPatternID) != attackPatternIDCount.end() && attackPatternIDCount.at(attackPatternID) > 0;
}

void EditorEnemyPhase::addAttackPatternID(std::string time, int id, ExprSymbolTable attackPatternSymbolsDefiner) {
	auto item = std::make_tuple(time, id, attackPatternSymbolsDefiner);
	attackPatternIDs.push_back(item);

	if (attackPatternIDCount.find(id) == attackPatternIDCount.end()) {
		attackPatternIDCount[id] = 1;
	} else {
		attackPatternIDCount[id]++;
	}

	lastAttackPatternActionsTotalTimeCalculated = false;
}

void EditorEnemyPhase::removeAttackPattern(int index) {
	int attackPatternID = std::get<1>(attackPatternIDs[index]);
	attackPatternIDs.erase(attackPatternIDs.begin() + index);
	attackPatternIDCount[attackPatternID]--;

	lastAttackPatternActionsTotalTimeCalculated = false;
}
