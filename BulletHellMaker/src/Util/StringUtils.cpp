#include <Util/StringUtils.h>

#include <regex>

std::string removeTrailingWhitespace(const std::string& str) {
	return std::regex_replace(str, std::regex("^ +| +$|( ) +"), "$1");
}