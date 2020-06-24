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
	exprtk::expression<float> expression;
};

class ExprSymbolTable : public TextMarshallable {
public:
	std::string format() const override;
	void load(std::string formattedString) override;

	/*
	Returns a symbol_table that defines constant values for every symbol in this ExprSymbolTable.

	higherLevelSymbolTable - the symbol_table that defines all symbols needed to evaluate the expression
		in every ExprSymbolDefinition
	*/
	exprtk::symbol_table<float> getLowerLevelSymbolTable(exprtk::symbol_table<float> higherLevelSymbolTable);

private:
	std::map<std::string, ExprSymbolDefinition> map;
};

class ValueSymbolTable : public TextMarshallable {
public:
	std::string format() const override;
	void load(std::string formattedString) override;

	/*
	Returns a symbol_table that defines constant values for every unredelegated symbol.
	*/
	exprtk::symbol_table<float> getSymbolTable();
	/*
	Returns a symbol_table that defines constant values for every unredelegated symbol
	and defines every redelegated symbol with value 0.
	*/
	exprtk::symbol_table<float> getZeroFilledSymbolTable();

private:
	std::map<std::string, ValueSymbolDefinition> map;
};