#pragma once
#include <string>
#include <sys/stat.h>

static bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}