#include <Util/TextFileParser.h>

#include <Util/StringUtils.h>

TextFileParser::TextFileParser(std::ifstream& stream)
	: stream(stream) {
}

std::unique_ptr<std::map<std::string, std::unique_ptr<std::map<std::string, std::string>>>> TextFileParser::read(char delimiter) {
	std::unique_ptr<std::map<std::string, std::unique_ptr<std::map<std::string, std::string>>>> map = std::make_unique<std::map<std::string, std::unique_ptr<std::map<std::string, std::string>>>>();

	std::string line;
	while (std::getline(stream, line)) {
		if (line.length() <= 1) {
			continue;
		}

		if (line[0] == '[' && line[line.length() - 1] == ']') {
			std::string headerName = line.substr(1, line.length() - 2);
			std::unique_ptr<std::map<std::string, std::string>> submap = std::make_unique<std::map<std::string, std::string>>();

			// Read in submap
			std::string subline;
			while (std::getline(stream, subline)) {
				if (subline.length() <= 1) {
					break;
				}

				int delimiterPos = subline.find(delimiter);
				std::string key = subline.substr(0, delimiterPos);
				std::string value = subline.substr(delimiterPos + 1);
				submap->insert(std::make_pair(removeTrailingWhitespace(subline.substr(0, delimiterPos)), removeTrailingWhitespace(subline.substr(delimiterPos + 1))));
			}

			map->insert(std::make_pair(headerName, move(submap)));
		}
	}

	return std::move(map);
}
