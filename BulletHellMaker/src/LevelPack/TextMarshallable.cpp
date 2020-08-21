#include <LevelPack/TextMarshallable.h>

std::vector<std::string> split(std::string str, char delimiter) {
	std::string segment;
	std::vector<std::string> seglist;

	int cur = 0; // how many layers of parentheses the current index is in
	int prev = -1;
	// When the beginning of a string is found, skip forward the the length of the string
	// Since recursive TextMarshallable objects will be formatted into a string, this also skips TM objects
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == '@') {
			prev = str.find(':', i);
			int charsUntilColon = prev - i;
			int numChars = std::stoi(str.substr(i + 1, charsUntilColon));
			i += numChars + charsUntilColon + 1;
			cur++;
			continue;
		}

		if (str[i] == DELIMITER && cur == 0) {
			seglist.push_back(str.substr(prev + 1, i - prev - 1));
			prev = i;
		}

		if (str[i] == '(') {
			cur++;
		} else if (str[i] == ')') {
			cur--;
		}
	}
	if (prev != str.length() - 1) {
		seglist.push_back(str.substr(prev + 1, str.length() - prev - 1));
	}

	// Remove leading/trailing parantheses
	for (int i = 0; i < seglist.size(); i++) {
		if (seglist[i][0] == '(') {
			seglist[i] = seglist[i].substr(1, seglist[i].length() - 1);
		}

		if (seglist[i][seglist[i].length() - 1] == ')') {
			seglist[i] = seglist[i].substr(0, seglist[i].length() - 1);
		}
	}

	return seglist;
}

bool contains(std::string str, char c)
{
	return str.find(c) != std::string::npos;
}

std::string formatBool(bool b) {
	if (b) {
		return "(1)" + tm_delim;
	} else {
		return "(0)" + tm_delim;
	}
}

bool unformatBool(std::string str) {
	return str == "1" ? true : false;
}

std::string formatString(std::string str) {
	return "@" + std::to_string(str.length()) + ":(" + str + ")" + tm_delim;
}

std::string formatTMObject(const TextMarshallable & tm) {
	return formatString(tm.format());
}
