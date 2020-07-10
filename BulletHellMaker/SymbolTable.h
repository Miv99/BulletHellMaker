#pragma once
#include <string>
#include <map>
#include "exprtk.hpp"
#include "TextMarshallable.h"

/*
A symbol that can either be redelegated or is assigned some value.
*/
struct ValueSymbolDefinition {
	float value;
	// If this is true, this symbol must be defined by some higher-level LevelPackObject when the LevelPackObject that uses it is used in gameplay
	bool redelegated = false;
};

/*
A symbol whose value comes from an expression that could
require the use of some unrelated symbol table.
*/
struct ExprSymbolDefinition {
	// The string used to create the expression that will be used to evaluate this symbol's value
	std::string expressionStr;
};

class ExprSymbolTable : public TextMarshallable {
public:
	std::string format() const override;
	void load(std::string formattedString) override;

	void setSymbol(std::string symbol, std::string expressionStr);
	void removeSymbol(std::string symbol);

	/*
	Returns a symbol_table that defines constant values for every symbol in this ExprSymbolTable.

	higherLevelSymbolTables - the list of symbol_tables that, combined, defines all symbols needed to evaluate the expression
		in every ExprSymbolDefinition. In the case of the same symbol being defined multiple times, the definition
		in the farthest-back symbol table will be used.
	*/
	exprtk::symbol_table<float> toLowerLevelSymbolTable(std::vector<exprtk::symbol_table<float>> higherLevelSymbolTables);
	/*
	Same thing as toLowerLevelSymbolTable(std::vector<exprtk::symbol_table<float>> higherLevelSymbolTables) but takes
	an expression that has only the higherLevelSymbolTables registered.
	*/
	exprtk::symbol_table<float> toLowerLevelSymbolTable(exprtk::expression<float> expression);

	std::map<std::string, ExprSymbolDefinition>::const_iterator getIteratorBegin();
	std::map<std::string, ExprSymbolDefinition>::const_iterator getIteratorEnd();

	ExprSymbolDefinition getSymbolDefinition(std::string symbol) const;
	bool hasSymbol(std::string symbol) const;

	bool isEmpty() const;

private:
	std::map<std::string, ExprSymbolDefinition> map;
};

class ValueSymbolTable : public TextMarshallable {
public:
	std::string format() const override;
	void load(std::string formattedString) override;

	void setSymbol(std::string symbol, float value, bool redelegated);
	void removeSymbol(std::string symbol);

	/*
	Returns a symbol_table that defines constant values for every unredelegated symbol.
	*/
	exprtk::symbol_table<float> toExprtkSymbolTable() const;
	/*
	Returns a symbol_table that defines constant values for every unredelegated symbol
	and defines every redelegated symbol with value 0.
	*/
	exprtk::symbol_table<float> toZeroFilledSymbolTable() const;

	std::map<std::string, ValueSymbolDefinition>::const_iterator getIteratorBegin();
	std::map<std::string, ValueSymbolDefinition>::const_iterator getIteratorEnd();

	ValueSymbolDefinition getSymbolDefinition(std::string symbol) const;
	bool hasSymbol(std::string symbol) const;

	bool isEmpty() const;

private:
	std::map<std::string, ValueSymbolDefinition> map;
};