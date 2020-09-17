#include <Game/AudioPlayer.h>

#include <algorithm>

//TODO: move these into some settings class
float masterVolume = 0.8f;
float soundVolume = 0.2f;
float musicVolume = 0.2f;

AudioSettings::AudioSettings() {
}

SoundSettings::SoundSettings() {
}

SoundSettings::SoundSettings(std::string fileName, float volume, float pitch) {
	this->fileName = fileName;
	this->volume = volume;
	this->pitch = pitch;
}

SoundSettings::SoundSettings(std::string fileName, float volume, float pitch, bool disabled) {
	this->fileName = fileName;
	this->volume = volume;
	this->pitch = pitch;
	this->disabled = disabled;
}

SoundSettings::SoundSettings(const SoundSettings& copy) {
	fileName = copy.fileName;
	volume = copy.volume;
	pitch = copy.pitch;
	disabled = copy.disabled;
}

std::string SoundSettings::format() const {
	return formatString(fileName) + tos(volume) + tos(pitch) + formatBool(disabled);
}

void SoundSettings::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	fileName = items[0];
	volume = std::stof(items[1]);
	pitch = std::stof(items[2]);
	disabled = unformatBool(items[3]);
}

MusicSettings::MusicSettings() {
}

MusicSettings::MusicSettings(std::string fileName, bool loops, int loopStartMilliseconds, int loopLengthMilliseconds, float volume, float pitch) {
	this->fileName = fileName;
	this->loops = loops;
	this->loopStartMilliseconds = loopStartMilliseconds;
	this->loopLengthMilliseconds = loopLengthMilliseconds;
	this->volume = volume;
	this->pitch = pitch;
}

MusicSettings::MusicSettings(const MusicSettings& copy) {
	fileName = copy.fileName;
	loops = copy.loops;
	loopStartMilliseconds = copy.loopStartMilliseconds;
	loopLengthMilliseconds = copy.loopLengthMilliseconds;
	volume = copy.volume;
	pitch = copy.pitch;
	transitionTime = copy.transitionTime;
	disabled = copy.disabled;
}

std::string MusicSettings::format() const {
	return formatString(fileName) + formatBool(loops) + tos(loopStartMilliseconds) + tos(loopLengthMilliseconds) + tos(volume) + tos(pitch) + formatBool(disabled) + tos(transitionTime);
}

void MusicSettings::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
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
	if (timeSinceMusicTransitionStart <= musicTransitionTime) {
		if (musicTransitionTime != 0) {
			for (auto t : currentlyFading) {
				float newVolume = std::get<2>(t) + (timeSinceMusicTransitionStart / musicTransitionTime) * (std::get<3>(t) - std::get<2>(t));
				std::get<0>(t)->setVolume(newVolume * masterVolume * musicVolume);
			}
		}
		
		timeSinceMusicTransitionStart += deltaTime;
		if (timeSinceMusicTransitionStart >= musicTransitionTime) {
			// Prevent volume over/undershoot
			for (auto t : currentlyFading) {
				std::get<0>(t)->setVolume(std::get<3>(t) * masterVolume * musicVolume);

				if (std::get<1>(t) == VOLUME_CHANGE_STATUS::DECREASING) {
					std::get<0>(t)->pause();
				}
			}
			currentlyFading.clear();
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

	// Fade-out anything currently being played
	if (currentMusic && currentMusic->getStatus() == sf::SoundSource::Status::Playing) {
		currentlyFading.push_back(std::make_tuple(currentMusic, VOLUME_CHANGE_STATUS::DECREASING, currentMusic->getVolume()/(masterVolume * musicVolume), 0));
	}
	for (auto t : currentlyFading) {
		std::get<1>(t) = VOLUME_CHANGE_STATUS::DECREASING;
		std::get<2>(t) = std::get<0>(t)->getVolume() / (masterVolume * musicVolume);
		std::get<3>(t) = 0;
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

	// Fade-in new music
	currentlyFading.push_back(std::make_tuple(music, VOLUME_CHANGE_STATUS::INCREASING, 0, musicSettings.getVolume()));
	musicTransitionTime = musicSettings.getTransitionTime();
	timeSinceMusicTransitionStart = 0;
	update(0);

	return music;
}

void AudioPlayer::playMusic(std::shared_ptr<sf::Music> music, const MusicSettings& musicSettings) {
	if (!music || musicSettings.isDisabled() || musicSettings.getFileName() == "") return;

	// Fade-out anything currently being played
	if (currentMusic && currentMusic->getStatus() == sf::SoundSource::Status::Playing) {
		currentlyFading.push_back(std::make_tuple(currentMusic, VOLUME_CHANGE_STATUS::DECREASING, currentMusic->getVolume() / (masterVolume * musicVolume), 0));
	}
	for (auto t : currentlyFading) {
		std::get<1>(t) = VOLUME_CHANGE_STATUS::DECREASING;
		std::get<2>(t) = std::get<0>(t)->getVolume() / (masterVolume * musicVolume);
		std::get<3>(t) = 0;
	}

	music->setVolume(musicSettings.getVolume() * masterVolume * musicVolume);
	music->play();
	currentMusic = music;

	// Fade-in new music
	currentlyFading.push_back(std::make_tuple(music, VOLUME_CHANGE_STATUS::INCREASING, 0, musicSettings.getVolume()));
	musicTransitionTime = musicSettings.getTransitionTime();
	timeSinceMusicTransitionStart = 0;
	update(0);
}

bool AudioSettings::operator==(const AudioSettings& other) const {
	return fileName == other.fileName && volume == other.volume && pitch == other.pitch && disabled == other.disabled;
}
