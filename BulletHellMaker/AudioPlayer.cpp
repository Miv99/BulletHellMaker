#include "AudioPlayer.h"
#include <algorithm>

//TODO: move these into some settings class
float masterVolume = 0.8f;
float soundVolume = 0.2f;
float musicVolume = 0.2f;

std::string SoundSettings::format() const {
	return formatString(fileName) + tos(volume) + tos(pitch) + formatBool(disabled);
}

void SoundSettings::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	fileName = items[0];
	volume = std::stof(items[1]);
	pitch = std::stof(items[2]);
	disabled = unformatBool(items[3]);
}

std::string MusicSettings::format() const {
	return formatString(fileName) + formatBool(loops) + tos(loopStartMilliseconds) + tos(loopLengthMilliseconds) + tos(volume) + tos(pitch) + formatBool(disabled) + tos(transitionTime);
}

void MusicSettings::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	fileName = items[0];
	loops = unformatBool(items[1]);
	loopStartMilliseconds = std::stoi(items[2]);
	loopLengthMilliseconds = std::stoi(items[3]);
	volume = std::stof(items[4]);
	pitch = std::stof(items[5]);
	disabled = unformatBool(items[6]);
	transitionTime = std::stof(items[7]);
}

bool MusicSettings::operator==(const AudioSettings& other) const {
	const MusicSettings& derived = dynamic_cast<const MusicSettings&>(other);
	return AudioSettings::operator==(other) && loops == derived.loops && loopStartMilliseconds == derived.loopStartMilliseconds
		&& loopLengthMilliseconds == derived.loopLengthMilliseconds && transitionTime == derived.transitionTime;
}

void AudioPlayer::update(float deltaTime) {
	// Check if any sounds are done playing and remove it from the queue of sounds being played
	while (!currentSounds.empty() && currentSounds.front()->getStatus() == sf::Sound::Status::Stopped) {
		currentSounds.pop();
	}

	// Update music transitioning
	if (currentMusic && timeSinceMusicTransitionStart < musicTransitionTime) {
		if (fadingMusic) {
			fadingMusic->setVolume((musicTransitionFromVolume * masterVolume * musicVolume) * (1.0f - timeSinceMusicTransitionStart / musicTransitionTime));
		}
		currentMusic->setVolume((timeSinceMusicTransitionStart/musicTransitionTime) * musicTransitionFinalVolume * masterVolume * musicVolume);

		timeSinceMusicTransitionStart += deltaTime;
		if (timeSinceMusicTransitionStart >= musicTransitionTime) {
			// Prevent volume over/undershoot
			currentMusic->setVolume(musicTransitionFinalVolume * masterVolume * musicVolume);
			if (fadingMusic) {
				fadingMusic->setVolume(0);
				fadingMusic = nullptr;
			}
		}
	}
}

void AudioPlayer::playSound(const SoundSettings& soundSettings) {
	if (soundSettings.isDisabled() || soundSettings.getFileName() == "") return;

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
	sound->setVolume(soundSettings.getVolume() * masterVolume * soundVolume);
	sound->setPitch(soundSettings.getPitch());
	sound->play();
	currentSounds.push(std::move(sound));
}

std::shared_ptr<sf::Music> AudioPlayer::playMusic(const MusicSettings& musicSettings) {
	if (musicSettings.isDisabled() || musicSettings.getFileName() == "") return nullptr;

	if (currentMusic && currentMusic->getStatus() == sf::SoundSource::Status::Playing && musicSettings.getTransitionTime() > 0) {
		fadingMusic = currentMusic;
		musicTransitionFromVolume = musicTransitionFinalVolume;
	} else {
		fadingMusic = nullptr;
	}

	std::shared_ptr<sf::Music> music = std::make_shared<sf::Music>();
	if (!music->openFromFile(musicSettings.getFileName())) {
		//TODO: handle audio not being able to be loaded
		return nullptr;
	}
	music->setVolume(musicSettings.getVolume() * masterVolume * musicVolume);
	if (musicSettings.getLoop()) {
		music->setLoopPoints(sf::Music::TimeSpan(sf::milliseconds(musicSettings.getLoopStartMilliseconds()), sf::milliseconds(musicSettings.getLoopLengthMilliseconds())));
	}
	music->setPitch(musicSettings.getPitch());
	music->play();
	currentMusic = music;
	update(0);

	musicTransitionFinalVolume = musicSettings.getVolume();
	musicTransitionTime = musicSettings.getTransitionTime();
	timeSinceMusicTransitionStart = 0;

	return music;
}

bool AudioSettings::operator==(const AudioSettings& other) const {
	return fileName == other.fileName && volume == other.volume && pitch == other.pitch && disabled == other.disabled;
}
