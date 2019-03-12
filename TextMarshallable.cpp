#include "TextMarshallable.h"

std::vector<std::string> split(std::string str, char delimiter) {
	std::stringstream test(str);
	std::string segment;
	std::vector<std::string> seglist;

	int cur = 0;
	int prev = -1;
	for (int i = 0; i < str.length(); i++) {
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
