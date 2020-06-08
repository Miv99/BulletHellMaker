#pragma once
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <SFML/Audio.hpp>
#include "TextMarshallable.h"

class AudioSettings {
public:
	inline AudioSettings() {}

	inline std::string getFileName() const { return fileName; }
	inline float getVolume() const { return volume; }
	inline float getPitch() const { return pitch; }
	inline bool isDisabled() const { return disabled; }

	inline void setFileName(std::string fileName) { this->fileName = fileName; }
	inline void setVolume(float volume) { this->volume = volume; }
	inline void setPitch(float pitch) { this->pitch = pitch; }
	inline void setDisabled(bool disabled) { this->disabled = disabled; }

protected:
	std::string fileName = "";

	// in range [0, 100]
	float volume = 100;
	// in range [1, inf]
	float pitch = 1;

	// If true, sound will not play
	bool disabled = false;
};

class SoundSettings : public TextMarshallable, public AudioSettings {
public:
	inline SoundSettings() {}
	inline SoundSettings(std::string fileName, float volume = 100, float pitch = 1) {
		this->fileName = fileName;
		this->volume = volume;
		this->pitch = pitch;
	}
	inline SoundSettings(std::string fileName, float volume, float pitch, bool disabled) {
		this->fileName = fileName;
		this->volume = volume;
		this->pitch = pitch;
		this->disabled = disabled;
	}
	inline SoundSettings(const SoundSettings& copy) {
		fileName = copy.fileName;
		volume = copy.volume;
		pitch = copy.pitch;
		disabled = copy.disabled;
	}

	std::string format() const override;
	void load(std::string formattedString) override;
};

class MusicSettings : public TextMarshallable, public AudioSettings {
public:
	inline MusicSettings() {}
	inline MusicSettings(std::string fileName, bool loops = false, int loopStartMilliseconds = 0, int loopLengthMilliseconds = 0, float volume = 100, float pitch = 1) {
		this->fileName = fileName;
		this->loops = loops;
		this->loopStartMilliseconds = loopStartMilliseconds;
		this->loopLengthMilliseconds = loopLengthMilliseconds;
		this->volume = volume;
		this->pitch = pitch;
	}
	inline MusicSettings(const MusicSettings& copy) {
		fileName = copy.fileName;
		loops = copy.loops;
		loopStartMilliseconds = copy.loopStartMilliseconds;
		loopLengthMilliseconds = copy.loopLengthMilliseconds;
		volume = copy.volume;
		pitch = copy.pitch;
		transitionTime = copy.transitionTime;
		disabled = copy.disabled;
	}

	std::string format() const override;
	void load(std::string formattedString) override;

	inline bool getLoop() const { return loops; }
	inline int getLoopStartMilliseconds() const { return loopStartMilliseconds; }
	inline int getLoopLengthMilliseconds() const { return loopLengthMilliseconds; }
	inline float getTransitionTime() const { return transitionTime; }

	inline void setLoop(bool loops) { this->loops = loops; }
	inline void setLoopStartMilliseconds(int loopStartMilliseconds) { this->loopStartMilliseconds = loopStartMilliseconds; }
	inline void setLoopLengthMilliseconds(int loopStartMilliseconds) { this->loopLengthMilliseconds = loopLengthMilliseconds; }
	inline void setTransitionTime(float transitionTime) { this->transitionTime = transitionTime; }

private:
	bool loops = false;
	// only applicable if loops is true; see sf::Music's setLoopPoints() for explanation of these fields
	int loopStartMilliseconds = 0;
	int loopLengthMilliseconds = 0;

	// Time in seconds for music's volume to go from 0 to the above volume when the music starts playing
	float transitionTime = 0;
};

class AudioPlayer {
public:
	void update(float deltaTime);

	void playSound(const SoundSettings& soundSettings);
	/*
	Plays music.
	Returns a pointer to the Music object.
	*/
	std::shared_ptr<sf::Music> playMusic(const MusicSettings& musicSettings);

private:
	// Maps file names to SoundBuffers
	std::map<std::string, sf::SoundBuffer> soundBuffers;
	// Queue of sounds currently playing
	std::queue<std::unique_ptr<sf::Sound>> currentSounds;

	std::shared_ptr<sf::Music> currentMusic;
	// The volume the music is transitioning to, in seconds. Volume settings do not affect this value.
	float musicTransitionFinalVolume;
	// Time it will take to fully transition, in seconds
	float musicTransitionTime = 0;
	// in seconds
	float timeSinceMusicTransitionStart = 0;
};