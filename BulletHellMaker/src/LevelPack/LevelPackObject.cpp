#include <LevelPack/LevelPackObject.h>

#include <sys/stat.h>
#include <set>

const std::string LevelPackObject::INVALID_EXPRESSION_MESSAGE_FORMAT = "Invalid expression for %s";

void tabEveryLine(std::vector<std::string>& strings) {
	for (std::string& str : strings) {
		str = "\t" + str;
	}
}

bool expressionStrIsValid(exprtk::parser<float>& parser, const std::string& expressionStr, std::vector<exprtk::symbol_table<float>> symbolTables) {
	exprtk::expression<float> expression;

	// Will contain every unknown symbol used in expressionStr
	exprtk::symbol_table<float> unknownVarSymbolTable;
	expression.register_symbol_table(unknownVarSymbolTable);

	for (int i = symbolTables.size() - 1; i >= 0; i--) { 
		// Register every symbol table thought to be needed
		expression.register_symbol_table(symbolTables[i]); 

		std::vector<std::string> variable_list;
		symbolTables[i].get_variable_list(variable_list);
	}

	try {
		parser.compile(expressionStr, expression);
		expression.value();

		std::vector<std::string> variableList;
		unknownVarSymbolTable.get_variable_list(variableList);
		// The expression is legal only if there are no unknown symbols
		return variableList.size() == 0;
	} catch (...) {
		return false;
	}
	return true;
}