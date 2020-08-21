#include <LevelPack/Animation.h>

std::shared_ptr<sf::Sprite> Animation::update(float deltaTime) {
	if (done) {
		return nullptr;
	}
	
	time += deltaTime;
	while (time >= sprites[currentSpriteIndex].first) {
		time -= sprites[currentSpriteIndex].first;
		currentSpriteIndex++;
		if (currentSpriteIndex == sprites.size()) {
			if (looping) {
				currentSpriteIndex = 0;
			} else {
				done = true;
				return nullptr;
			}
		}
	}

	return sprites[currentSpriteIndex].second;
}
