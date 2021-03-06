#pragma once
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <utility>

#include <SFML/Audio.hpp>

#include <LevelPack/TextMarshallable.h>

class AudioSettings {
public:
	AudioSettings();

	inline std::string getFileName() const { return fileName; }
	inline float getVolume() const { return volume; }
	inline float getPitch() const { return pitch; }
	inline bool isDisabled() const { return disabled; }

	inline void setFileName(std::string fileName) { this->fileName = fileName; }
	inline void setVolume(float volume) { this->volume = volume; }
	inline void setPitch(float pitch) { this->pitch = pitch; }
	inline void setDisabled(bool disabled) { this->disabled = disabled; }

	virtual bool operator==(const AudioSettings& other) const;

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
	SoundSettings();
	SoundSettings(std::string fileName, float volume = 100, float pitch = 1);
	SoundSettings(std::string fileName, float volume, float pitch, bool disabled);
	SoundSettings(const SoundSettings& copy);

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;
};

class MusicSettings : public TextMarshallable, public AudioSettings {
public:
	MusicSettings();
	MusicSettings(std::string fileName, bool loops = false, int loopStartMilliseconds = 0, int loopLengthMilliseconds = 0, float volume = 100, float pitch = 1);
	MusicSettings(const MusicSettings& copy);

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	inline bool getLoop() const { return loops; }
	inline int getLoopStartMilliseconds() const { return loopStartMilliseconds; }
	inline int getLoopLengthMilliseconds() const { return loopLengthMilliseconds; }
	inline float getTransitionTime() const { return transitionTime; }

	inline void setLoop(bool loops) { this->loops = loops; }
	inline void setLoopStartMilliseconds(int loopStartMilliseconds) { this->loopStartMilliseconds = loopStartMilliseconds; }
	inline void setLoopLengthMilliseconds(int loopStartMilliseconds) { this->loopLengthMilliseconds = loopLengthMilliseconds; }
	inline void setTransitionTime(float transitionTime) { this->transitionTime = transitionTime; }

	bool operator==(const AudioSettings& other) const override;

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

	If a Music was playing at the time of this function call, there will be a crossfade transition to
	the new Music depending on the transition time in musicSettings. The old Music will be paused
	and its reference in this AudioPlayer will be lost at the end of the transition, so a reference to
	the old Music should be kept if it should be resumed later.
	*/
	std::shared_ptr<sf::Music> playMusic(const MusicSettings& musicSettings);
	/*
	Plays music from an existing Music.

	If a Music was playing at the time of this function call, there will be a crossfade transition to
	the new Music depending on the transition time in musicSettings. The old Music will be paused
	and its reference in this AudioPlayer will be lost at the end of the transition, so a reference to
	the old Music should be kept if it should be resumed later.
	*/
	void playMusic(std::shared_ptr<sf::Music> music, const MusicSettings& musicSettings);

private:
	enum class VOLUME_CHANGE_STATUS {
		DECREASING,
		INCREASING
	};

	// Maps file names to SoundBuffers
	std::map<std::string, sf::SoundBuffer> soundBuffers;
	// Queue of sounds currently playing
	std::queue<std::unique_ptr<sf::Sound>> currentSounds;

	std::shared_ptr<sf::Music> currentMusic;

	// Musics being faded, how they're being faded, original volume before global settings, and target volume before global settings
	std::vector<std::tuple<std::shared_ptr<sf::Music>, VOLUME_CHANGE_STATUS, float, float>> currentlyFading;

	// Time it will take to fully transition, in seconds
	float musicTransitionTime = 0;
	// Time since the last transition started, in seconds
	float timeSinceMusicTransitionStart = 0;
};