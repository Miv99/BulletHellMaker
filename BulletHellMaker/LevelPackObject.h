#pragma once
#include <utility>
#include <string>
#include <sstream>
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
	/*
	Legality check for the usage of the LevelPackObject.
	Returns a pair indicating whether the object is legal and the message explaining errors, if the object is not legal.
	*/
	virtual std::pair<bool, std::string> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const = 0;
};