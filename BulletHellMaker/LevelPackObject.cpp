#include "LevelPackObject.h"
#include <sys/stat.h>

static bool fileExists(const std::string & name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

std::string tabEveryLine(const std::string & str) {
	std::string result;
	std::istringstream iss(str);
	for (std::string line; std::getline(iss, line); ) {
		result += "\t" + line + "\n";
	}
	return result;
}

void LevelPackObject::compileExpressions(exprtk::symbol_table<float> symbolTable) {
}
