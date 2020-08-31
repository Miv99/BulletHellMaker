#pragma once
#include <utility>
#include <string>
#include <sstream>
#include <memory>

#include <exprtk.hpp>
#include <Util/IOUtils.h>
#include <LevelPack/LevelPack.h>
#include <DataStructs/SpriteLoader.h>
#include <LevelPack/TextMarshallable.h>
#include <DataStructs/SymbolTable.h>
#include <LevelPack/ExpressionCompilable.h>
#include <LevelPack/TextMarshallable.h>

#define DEFINE_PARSER_AND_EXPR_FOR_COMPILE exprtk::parser<float> parser; \
exprtk::expression<float> expr = exprtk::expression<float>(); \
if (!this->symbolTable.isEmpty()) { symbolTables.push_back(symbolTable.toExprtkSymbolTable()); } \
for (int i = symbolTables.size() - 1; i >= 0; i--) { expr.register_symbol_table(symbolTables[i]); } \
/*
Macro for defining a variable whose value is defined by an expression, for a LevelPackObject. 
The getter for the variable should return NameExprCompiledValue.
*/
#define DEFINE_EXPRESSION_VARIABLE(Name, Type) std::string Name; \
Type Name##ExprCompiledValue;\
/*
Same thing as DEFINE_EXPRESSION_VARIABLE but with a default string value.
*/
#define DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(Name, Type, DefaultString) std::string Name = #DefaultString; \
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
Name##ExprCompiledValue = expr.value(); \
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
Name##ExprCompiledValue = (int)std::lrint(expr.value()); \

#define DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK exprtk::parser<float> parser; \
parser.enable_unknown_symbol_resolver(); \
if (!this->symbolTable.isEmpty()) { symbolTables.push_back(symbolTable.toZeroFilledSymbolTable()); } \
/*
Macro for doing a legal check on an expression.
DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK should be called anywhere before this macro is called.
*/
#define LEGAL_CHECK_EXPRESSION(Name, DisplayedName) if (!expressionStrIsValid(parser, Name, symbolTables)) { \
status = std::max(status, LEGAL_STATUS::ILLEGAL); \
messages.push_back(std::string("Invalid expression for ").append(#DisplayedName)); \
} \


/*
Appends a tab before every string.
*/
void tabEveryLine(std::vector<std::string>& strings);
/*
Returns whether expressionStr is a valid expression string when using some ValueSymbolTable.
The actual values in symbolTable are ignored; the only important part is whether 
every variable used in expressionStr is defined or redelegated in symbolTable.
*/
bool expressionStrIsValid(exprtk::parser<float>& parser, const std::string& expressionStr, std::vector<exprtk::symbol_table<float>> symbolTables);


class LevelPackObject : public TextMarshallable, public ExpressionCompilable {
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

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	/*
	Legality check for the usage of the LevelPackObject.
	Returns a pair indicating the legal status of the object and a list of messages explaining errors and/or warnings.

	levelPack - the LevelPack this LevelPackObject belongs to
	spriteLoader - the SpriteLoader to be used with levelPack
	symbolTables - a list of symbol_tables; its content combined should define all symbols that will be needed to compile every expression this object
		uses and every expression in this object's unique objects tree. The symbol_tables should be ordered in descending priority such that in the 
		case of the same symbol being defined multiple times, the definition in the farthest-back symbol_table will be used.
	*/
	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const = 0;

	virtual void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) = 0;

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

	// ID unique to all other LevelPackObjects of the same derived class. Only used for non-unique objects (see references for definition of a unique object).
	// Negative IDs should be reserved for object previews in the editor.
	int id;
	// User-defined name. Only used for non-unique objects.
	std::string name;
};