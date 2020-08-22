#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <stdexcept>

/*
Utility to imitate sprintf() but with std::string.
*/
template<typename ... Args>
std::string format(const std::string& format, Args ... args) {
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size <= 0) { throw std::runtime_error("Error during formatting."); }
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

#define tm_delim std::string(1, DELIMITER)

const static char DELIMITER = '|';

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
	return "(" + std::to_string(primitive) + ")" + tm_delim;
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
	// Throws an exception if the implementation contains strings that contain delimiters
	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;
};

/*
Format a TextMarshallable object
*/
std::string formatTMObject(const TextMarshallable& tm);