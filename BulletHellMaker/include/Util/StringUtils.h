#pragma once
#include <memory>
#include <string>
#include <stdexcept>

/*
String format with std::string.
*/
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args) {
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size <= 0) { throw std::runtime_error("Error during formatting."); }
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

/*
Format a number for pretty display.
*/
template<typename T>
std::string formatNum(const T& n) {
	std::string s = std::to_string(n);
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
			if (s[i + 1] == '.') {
				break;
			}
		}

		return s.substr(0, i + 1);
	} else {
		return s;
	}
}

/*
Removes trailing whitespace from a string.
*/
std::string removeTrailingWhitespace(const std::string& str);