#pragma once
#include <utility>
#include <string>
#include <sstream>
#include <memory>
#include "LevelPack.h"
#include "SpriteLoader.h"

/*
Returns whether the file in the file path exists.
*/
bool fileExists(const std::string& name);
/*
Returns str, but with a tab before each line.
*/
std::string tabEveryLine(const std::string& str);

class LevelPackObject {
public:
	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	/*
	Legality check for the usage of the LevelPackObject.
	Returns a pair indicating whether the object is legal and the message explaining errors, if the object is not legal.
	*/
	virtual std::pair<bool, std::string> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const = 0;

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