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

	if (s.size() > 0 && s.find('.') != std::string::npos) {
		// Remove trailing zeros and dot if the string has a dot
		int i = s.size() - 1;
		while (i > 0 && (s[i] == '0' || s[i] == '.')) {
			i--;
		}

		return s.substr(0, i + 1);
	} else {
		return s;
	}
}

class TextMarshallable {
public:
	// Throws an exception if the implementation contains strings that contain delimiters
	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;
};