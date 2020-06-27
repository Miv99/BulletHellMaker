#pragma once
#include <utility>
#include <string>
#include <sstream>
#include <memory>
#include "LevelPack.h"
#include "SpriteLoader.h"
#include "TextMarshallable.h"
#include "SymbolTable.h"

#define DEFINE_PARSER_AND_EXPR_FOR_COMPILE exprtk::parser<float> parser; \
exprtk::expression<float> expr = exprtk::expression<float>(); \
expr.register_symbol_table(symbolTable); \
/*
Macro for defining a variable whose value is defined by an expression, for a LevelPackObject. 
The getter for the variable should return NameExprCompiledValue.
*/
#define DEFINE_EXPRESSION_VARIABLE(Name, Type) std::string Name; \
exprtk::expression<float> Name##Expr; \
Type Name##ExprCompiledValue;\
/*
Same thing as DEFINE_EXPRESSION_VARIABLE but with a default string value.
*/
#define DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(Name, Type, DefaultString) std::string Name = #DefaultString; \
exprtk::expression<float> Name##Expr; \
Type Name##ExprCompiledValue;\
/*
This macro only works if variables following the naming scheme in DEFINE_EXPRESSION_VARIABLE with float for TypeName.

Usage:
void compileExpressions(exprtk::symbol_table<float> symbolTable) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(name1)
	COMPILE_EXPRESSION_FOR_FLOAT(name2)
	...
}
...
*/
#define COMPILE_EXPRESSION_FOR_FLOAT(Name) parser.compile(##Name, expr); \
Name##ExprCompiledValue = expr.value();
/*
This macro only works if variables following the naming scheme in DEFINE_EXPRESSION_VARIABLE with int for TypeName.

Usage:
void compileExpressions(exprtk::symbol_table<float> symbolTable) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_INT(name1)
	COMPILE_EXPRESSION_FOR_INT(name2)
	...
}
*/
#define COMPILE_EXPRESSION_FOR_INT(Name) parser.compile(##Name, expr); \
Name##ExprCompiledValue = (int)std::round(expr.value());

/*
Returns whether the file in the file path exists.
*/
bool fileExists(const std::string& name);
/*
Appends a tab before every string.
*/
void tabEveryLine(std::vector<std::string>& strings);
/*
Returns whether expressionStr is a valid expression string when using some ValueSymbolTable.
The actual values in symbolTable are ignored; the only important part is whether 
every variable used in expressionStr is defined or redelegated in symbolTable.
*/
bool expressionStrIsValid(exprtk::parser<float>& parser, const std::string& expressionStr, ValueSymbolTable symbolTable);


class LevelPackObject {
public:
	/*
	This is structured such that the std::max of a current status and some more important
	status is always the more important one.
	*/
	enum class LEGAL_STATUS {
		LEGAL = 0,
		WARNING = 1,
		ILLEGAL = 2
	};

	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	/*
	Legality check for the usage of the LevelPackObject.
	Returns a pair indicating the legal status of the object and a list of messages explaining errors and/or warnings.

	levelPack - the LevelPack this LevelPackObject belongs to
	spriteLoader - the SpriteLoader to be used with levelPack
	*/
	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const = 0;

	/*
	This shouldn't be used if the LevelPackObject already belongs to a LevelPack.
	*/
	inline void setID(int id) { this->id = id; }
	inline void setName(std::string name) { this->name = name; }

	inline int getID() const { return id; }
	inline std::string getName() const { return name; }


protected:
	// Format for the message in legal() for an invalid expression. The only parameter is the descriptive name of the field as a C string.
	const static std::string INVALID_EXPRESSION_MESSAGE_FORMAT;

	// ID unique to all other LevelPackObjects of the same derived class
	int id;
	// User-defined name of the attack
	std::string name;
};