#include "AudioPlayer.h"

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

void AudioPlayer::update(float deltaTime) {
	// Check if any sounds are done playing and remove it from the queue of sounds being played
	while (!currentSounds.empty() && currentSounds.front()->getStatus() == sf::Sound::Status::Stopped) {
		currentSounds.pop();
	}
}

/*
fileName - file name with extension
volume - in range [0, 100], where 100 is full volume
*/
void AudioPlayer::playSound(const SoundSettings& soundSettings) {
	// Check if the sound's SoundBuffer already exists
	if (soundBuffers.count(soundSettings.getFileName()) == 0) {
		sf::SoundBuffer buffer;
		if (!buffer.loadFromFile(soundSettings.getFileName())) {
			//TODO: handle audio not being able to be loaded
			return;
		}
		soundBuffers[soundSettings.getFileName()] = std::move(buffer);
	}
	std::unique_ptr<sf::Sound> sound = std::make_unique<sf::Sound>();
	sound->setBuffer(soundBuffers[soundSettings.getFileName()]);
	sound->setVolume(soundSettings.getVolume());
	sound->setPitch(soundSettings.getPitch());
	sound->play();
	currentSounds.push(std::move(sound));
}


/*
fileName - file name with extension
volume - in range [0, 100], where 100 is full volume
*/
void AudioPlayer::playMusic(const MusicSettings& musicSettings) {
	std::shared_ptr<sf::Music> music = loadMusic(musicSettings);
	music->play();
	currentMusic = music;
}

void AudioPlayer::playMusic(std::shared_ptr<sf::Music> music) {
	music->play();
	currentMusic = music;
}

std::shared_ptr<sf::Music> AudioPlayer::loadMusic(const MusicSettings & musicSettings) {
	std::shared_ptr<sf::Music> music = std::make_shared<sf::Music>();
	if (!music->openFromFile(musicSettings.getFileName())) {
		//TODO: handle audio not being able to be loaded
		return nullptr;
	}
	music->setVolume(musicSettings.getVolume());
	if (musicSettings.getLoop()) {
		music->setLoopPoints(sf::Music::TimeSpan(sf::milliseconds(musicSettings.getLoopStartMilliseconds()), sf::milliseconds(musicSettings.getLoopLengthMilliseconds())));
	}
	music->setPitch(musicSettings.getPitch());
	return music;
}
