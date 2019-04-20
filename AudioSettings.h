#pragma once
#include <string>
#include "TextMarshallable.h"

class SoundSettings : public TextMarshallable {
public:
	inline SoundSettings() {}
	inline SoundSettings(std::string fileName, float volume = 100, float pitch = 1) : fileName(fileName), volume(volume), pitch(pitch) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline std::string getFileName() const { return fileName; }
	inline float getVolume() const { return volume; }
	inline float getPitch() const { return pitch; }

	inline void setFileName(std::string fileName) { this->fileName = fileName; }
	inline void setVolume(float volume) { this->volume = volume; }
	inline void setPitch(float pitch) { this->pitch = pitch; }

private:
	std::string fileName;

	// in range [0, 100]
	float volume = 100;
	// in range [1, inf]
	float pitch = 1;
};

class MusicSettings : public TextMarshallable {
public:
	inline MusicSettings() {}
	inline MusicSettings(std::string fileName, bool loops = false, int loopStartMilliseconds = 0, int loopLengthMilliseconds = 0, float volume = 100, float pitch = 1) : fileName(fileName), loops(loops), loopStartMilliseconds(loopStartMilliseconds), loopLengthMilliseconds(loopLengthMilliseconds), volume(volume), pitch(pitch) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline std::string getFileName() const { return fileName; }
	inline bool getLoop() const { return loops; }
	inline int getLoopStartMilliseconds() const { return loopStartMilliseconds; }
	inline int getLoopLengthMilliseconds() const { return loopLengthMilliseconds; }
	inline float getVolume() const { return volume; }
	inline float getPitch() const { return pitch; }

	inline void setFileName(std::string fileName) { this->fileName = fileName; }
	inline void setLoop(bool loops) { this->loops = loops; }
	inline void setLoopStartMilliseconds(int loopStartMilliseconds) { this->loopStartMilliseconds = loopStartMilliseconds; }
	inline void setLoopLengthMilliseconds(int loopStartMilliseconds) { this->loopLengthMilliseconds = loopLengthMilliseconds; }
	inline void setVolume(float volume) { this->volume = volume; }
	inline void setPitch(float pitch) { this->pitch = pitch; }

private:
	std::string fileName;

	bool loops = false;
	// only applicable if loops is true; see sf::Music's setLoopPoints() for explanation of these fields
	int loopStartMilliseconds = 0;
	int loopLengthMilliseconds = 0;

	// in range [0, 100]
	float volume = 100;
	// in range [1, inf]
	float pitch = 1;
};