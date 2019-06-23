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

template<typename T>
std::string formatNum(const T &n) {
	std::ostringstream oss;
	oss << n;
	std::string s = oss.str();
	int dotpos = s.find_first_of('.');
	if (dotpos != std::string::npos) {
		int ipos = s.size() - 1;
		while (s[ipos] == '0' && ipos > dotpos) {
			--ipos;
		}
		s.erase(ipos + 1, std::string::npos);
	}
	return s;
}

class TextMarshallable {
public:
	// Throws an exception if the implementation contains strings that contain delimiters
	virtual std::string format() = 0;
	virtual void load(std::string formattedString) = 0;
};