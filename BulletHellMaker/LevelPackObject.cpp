#include "LevelPackObject.h"
#include <sys/stat.h>

static bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void tabEveryLine(std::vector<std::string>& strings) {
	for (std::string& str : strings) {
		str = "\t" + str;
	}
}

bool expressionStrIsLegal(exprtk::parser<float>& parser, const std::string& expressionStr, ValueSymbolTable symbolTable) {
	exprtk::expression<float> expression;
	try {
		parser.compile(expressionStr, expression);
		// The actual result isn't important; just need to test whether it works, so just get a symbol_table with any values
		expression.register_symbol_table(symbolTable.getZeroFilledSymbolTable());
		expression.value();
	} catch (...) {
		return false;
	}
	return true;
}

void LevelPackObject::compileExpressions(exprtk::symbol_table<float> symbolTable) {
}
