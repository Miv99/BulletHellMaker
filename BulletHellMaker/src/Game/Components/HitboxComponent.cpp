#include <Game/Components/HitboxComponent.h>

#include <Util/MathUtils.h>
#include <LevelPack/Animatable.h>

HitboxComponent::HitboxComponent(ROTATION_TYPE rotationType, float radius, float x, float y) 
	: rotationType(rotationType), radius(radius), x(x), y(y), unrotatedX(x), unrotatedY(y) {
}

HitboxComponent::HitboxComponent(float radius, std::shared_ptr<sf::Sprite> sprite) 
	: radius(radius) {
	rotate(sprite);

	// Rotate sprite to angle 0 to find unrotatedX/Y, then rotate sprite back
	auto oldAngle = sprite->getRotation();
	sprite->setRotation(0);
	auto transformed = sprite->getTransform().transformPoint(sprite->getOrigin());
	unrotatedX = transformed.x;
	unrotatedY = transformed.y;
	sprite->setRotation(oldAngle);
}

void HitboxComponent::update(float deltaTime) {
	hitboxDisabledTimeLeft -= deltaTime;
}

void HitboxComponent::disable(float time) {
	hitboxDisabledTimeLeft = std::max(time, hitboxDisabledTimeLeft);
}

bool HitboxComponent::isDisabled() const {
	return hitboxDisabledTimeLeft > 0 || radius <= 0;
}

void HitboxComponent::setRadius(float radius) {
	this->radius = radius;
}

void HitboxComponent::rotate(float angle) {
	if (unrotatedX == unrotatedY == 0) return;

	if (rotationType == ROTATION_TYPE::ROTATE_WITH_MOVEMENT) {
		float sin = std::sin(angle);
		float cos = std::cos(angle);
		x = unrotatedX * cos - unrotatedY * sin;
		y = unrotatedX * sin + unrotatedY * cos;
	} else if (rotationType == ROTATION_TYPE::LOCK_ROTATION) {
		// Do nothing
	} else if (rotationType == ROTATION_TYPE::LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT) {
		// Flip across y-axis if facing left
		if (angle < -PI / 2.0f || angle > PI / 2.0f) {
			y = -unrotatedY;
		} else if (angle > -PI / 2.0f && angle < PI / 2.0f) {
			y = unrotatedY;
		}
		// Do nothing (maintain last value) if angle is a perfect 90 or -90 degree angle
	}
}

void HitboxComponent::rotate(std::shared_ptr<sf::Sprite> sprite) {
	if (unrotatedX == unrotatedY == 0) return;

	auto oldPos = sprite->getPosition();
	sprite->setPosition(0, 0);
	auto transformed = sprite->getTransform().transformPoint(sprite->getOrigin());
	x = transformed.x;
	y = transformed.y;
	sprite->setPosition(oldPos);
}

float HitboxComponent::getRadius() const { 
	return radius;
}

float HitboxComponent::getX() const { 
	return x;
}

float HitboxComponent::getY() const { 
	return y;
}