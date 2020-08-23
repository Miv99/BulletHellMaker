#pragma once
#include <fstream>
#include <memory>
#include <map>
#include <string>

class TextFileParser {
public:
	TextFileParser(std::ifstream& stream);
	/*
	Example:
	[Header text]
	key : value
	key2 : value2
	...

	will return a map that maps "Header text" to a map that has items ("key", "value"), ("key2", "value2")
	*/
	std::unique_ptr<std::map<std::string, std::unique_ptr<std::map<std::string, std::string>>>> read(char delimiter);

private:
	std::ifstream& stream;
};