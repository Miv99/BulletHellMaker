#include <Game/Components/ShadowTrailComponent.h>

ShadowTrailComponent::ShadowTrailComponent(float interval, float lifespan) 
	: interval(interval), lifespan(lifespan) {
}

bool ShadowTrailComponent::update(float deltaTime) {
	if (lifespan <= 0) return false;

	time += deltaTime;
	if (time > interval) {
		time -= interval;
		return true;
	}
	return false;
}

float ShadowTrailComponent::getLifespan() const { 
	return lifespan;
}

void ShadowTrailComponent::setInterval(float interval) { 
	this->interval = interval;
	time = 0;
}

void ShadowTrailComponent::setLifespan(float lifespan) { 
	this->lifespan = lifespan;
	time = 0;
}