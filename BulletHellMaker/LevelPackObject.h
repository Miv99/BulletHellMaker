#pragma once
#include <utility>
#include <string>
#include <sstream>
#include <memory>
#include "LevelPack.h"
#include "SpriteLoader.h"
#include "TextMarshallable.h"
#include "SymbolTable.h"

/*
Returns whether the file in the file path exists.
*/
bool fileExists(const std::string& name);
/*
Returns str, but with a tab before each line.
*/
std::string tabEveryLine(const std::string& str);
/*
Returns whether expressionStr is a valid expression string when using some ValueSymbolTable.
The actual values in symbolTable are ignored; the only important part is whether 
every variable used in expressionStr is defined or redelegated in symbolTable.
*/
bool expressionStrIsLegal(exprtk::parser<float>& parser, const std::string& expressionStr, ValueSymbolTable symbolTable);

class LevelPackObject {
public:
	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	/*
	Legality check for the usage of the LevelPackObject.
	Returns a pair indicating whether the object is legal and the message explaining errors, if the object is not legal.
	*/
	virtual std::pair<bool, std::string> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const = 0;

	/*
	Compiles every expression used in this LevelPackObject as well as every expression used in this LevelPackObject's 
	unique objects.

	symbolTable - a symbol_table that defines all symbols that will be needed
	*/
	virtual void compileExpressions(exprtk::symbol_table<float> symbolTable);

	/*
	This shouldn't be used if the LevelPackObject already belongs to a LevelPack.
	*/
	inline void setID(int id) { this->id = id; }
	inline void setName(std::string name) { this->name = name; }

	inline int getID() const { return id; }
	inline std::string getName() const { return name; }


protected:
	// ID unique to all other LevelPackObjects of the same derived class
	int id;
	// User-defined name of the attack
	std::string name;
};