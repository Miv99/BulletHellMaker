#include "AudioSettings.h"

std::string SoundSettings::format() {
	return "(" + fileName + ")" + delim + tos(volume) + delim + tos(pitch);
}

void SoundSettings::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	fileName = items[0];
	volume = std::stof(items[1]);
	pitch = std::stof(items[2]);
}

std::string MusicSettings::format() {
	std::string ret = "";
	ret += "(" + fileName + ")" + delim;
	if (loops) {
		ret += "1";
	} else {
		ret += "0";
	}
	ret += delim + tos(loopStartMilliseconds);
	ret += delim + tos(loopLengthMilliseconds);
	ret += delim + tos(volume) + delim + tos(pitch);
	return ret;
}

void MusicSettings::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	fileName = items[0];
	if (std::stoi(items[1]) == 1) {
		loops = true;
	} else {
		loops = false;
	}
	loopStartMilliseconds = std::stoi(items[2]);
	loopLengthMilliseconds = std::stoi(items[3]);
	volume = std::stof(items[4]);
	pitch = std::stof(items[5]);
}
