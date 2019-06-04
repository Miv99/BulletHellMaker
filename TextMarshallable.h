#pragma once
#include <string>
#include <vector>
#include <sstream>

#define tos(T) std::to_string(T)
#define tm_delim std::string(1, DELIMITER)

const static char DELIMITER = '|';

// Splits a string char a delimiter, ignoring all delimiters enclosed in parantheses
std::vector<std::string> split(std::string str, char delimiter);
// Checks if a string contains a char
bool contains(std::string str, char c);

class TextMarshallable {
public:
	// Throws an exception if the implementation contains strings that contain delimiters
	virtual std::string format() = 0;
	virtual void load(std::string formattedString) = 0;
};