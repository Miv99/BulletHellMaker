#include <LevelPack/EnemyPhase.h>

#include <LevelPack/LevelPack.h>

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
	auto items = split(formattedString, DELIMITER);
	id = stoi(items[0]);
	name = items[1];
	phaseBeginAction = EPAFactory::create(items[2]);
	phaseEndAction = EPAFactory::create(items[3]);
	attackPatternLoopDelay = items[4];

	attackPatternIDCount.clear();
	int i = 6;
	for (int a = 0; a < std::stoi(items[5]); a++) {
		int attackPatternID = std::stoi(items[i + 1]);
		ExprSymbolTable definer;
		definer.load(items[i + 2]);
		attackPatternIDs.push_back(std::make_tuple(items[i], attackPatternID, definer));

		if (attackPatternIDCount.count(attackPatternID) == 0) {
			attackPatternIDCount[attackPatternID] = 1;
		} else {
			attackPatternIDCount[attackPatternID]++;

		}
		i += 3;
	}
	playMusic = unformatBool(items[i++]);
	musicSettings.load(items[i++]);
	symbolTable.load(items[i++]);
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

void EditorEnemyPhase::addAttackPatternID(std::string time, int id, ExprSymbolTable attackPatternSymbolsDefiner) {
	auto item = std::make_tuple(time, id, attackPatternSymbolsDefiner);
	attackPatternIDs.push_back(item);

	if (attackPatternIDCount.count(id) == 0) {
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
