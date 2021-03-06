#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <stdexcept>

#include <Util/StringUtils.h>
#include <Util/json.hpp>

/*
Splits a string char by delimiter, ignoring all delimiters enclosed in parantheses.
All strings formatted with formatString() will automatically be unformatted.
*/
std::vector<std::string> split(std::string str, char delimiter);
// Checks if a string contains a char
bool contains(std::string str, char c);

/*
To string function for all primitive types, not including string and bool.
*/
template<typename T>
std::string tos(const T& primitive) {
	return "(" + std::to_string(primitive) + ")" + std::string(1, TextMarshallable::DELIMITER);
}
/*
To string function for bools.
*/
std::string formatBool(bool b);
/*
Retrieves the original bool passed into tos(bool)
*/
bool unformatBool(std::string str);
/*
Format a string to be compliant with TextMarshallable::format(), TextMarshallable::load(),
and split()
*/
std::string formatString(std::string str);

/*
Due to spaghetti code with split() and encodeString(), the user's implementation of format() can
never have the character '@' or '|', unless it is part of another TextMarshallable object or in a string.
*/
class TextMarshallable {
public:
	const static char DELIMITER;

	// Throws an exception if the implementation contains strings that contain delimiters
	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual nlohmann::json toJson() { return nlohmann::json(); };
	virtual void load(const nlohmann::json& j) {};
};

/*
Format a TextMarshallable object
*/
std::string formatTMObject(const TextMarshallable& tm);