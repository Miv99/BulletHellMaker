#include <Game/Components/SpriteComponent.h>

#include <Util/MathUtils.h>
#include <LevelPack/Animatable.h>
#include <DataStructs/SpriteEffectAnimation.h>
#include <DataStructs/SpriteLoader.h>

SpriteComponent::SpriteComponent(int renderLayer, float subLayer) 
	: renderLayer(renderLayer), subLayer(subLayer) {
}

SpriteComponent::SpriteComponent(SpriteLoader& spriteLoader, Animatable animatable, bool loopAnimatable, int renderLayer, float subLayer) 
	: renderLayer(renderLayer), subLayer(subLayer) {
	setAnimatable(spriteLoader, animatable, loopAnimatable);
	if (animatable.isSprite()) {
		originalSprite = sprite;
	}
}
SpriteComponent::SpriteComponent(ROTATION_TYPE rotationType, std::shared_ptr<sf::Sprite> sprite, int renderLayer, float subLayer) 
	: renderLayer(renderLayer), subLayer(subLayer), rotationType(rotationType), sprite(sprite), originalSprite(sprite) {
}

void SpriteComponent::update(float deltaTime) {
	if (animation != nullptr) {
		auto newSprite = animation->update(deltaTime);
		if (newSprite == nullptr) {
			// Animation is finished, so revert back to original sprite
			if (originalSprite) {
				updateSprite(*originalSprite);
			} else {
				updateSprite(nullptr);
			}
		} else {
			updateSprite(newSprite);
		}
	}
	if (sprite) {
		if (effectAnimation != nullptr) {
			effectAnimation->update(deltaTime);
		}

		// Rotate sprite
		if (rotationType == ROTATION_TYPE::ROTATE_WITH_MOVEMENT) {
			// Negative because SFML uses clockwise rotation
			sprite->setRotation(-rotationAngle * 180.0 / PI);
		} else if (rotationType == ROTATION_TYPE::LOCK_ROTATION) {
			// Do nothing
		} else if (rotationType == ROTATION_TYPE::LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT) {
			// Flip across y-axis if facing left
			auto curScale = sprite->getScale();
			if (rotationAngle < -PI / 2.0f || rotationAngle > PI / 2.0f) {
				lastFacedRight = false;
				if (curScale.x > 0) {
					sprite->setScale(-1.0f * curScale.x, curScale.y);
				}
			} else if (rotationAngle > -PI / 2.0f && rotationAngle < PI / 2.0f) {
				lastFacedRight = true;
				if (curScale.x < 0) {
					sprite->setScale(-1.0f * curScale.x, curScale.y);
				}
			} else if ((lastFacedRight && curScale.x < 0) || (!lastFacedRight && curScale.x > 0)) {
				sprite->setScale(-1.0f * curScale.x, curScale.y);
			}
			// Do nothing (maintain last values) if angle is a perfect 90 or -90 degree angle
		}
	}
}

bool SpriteComponent::animationIsDone() const {
	if (animation) {
		return animation->isDone();
	}
	return true;
}

void SpriteComponent::rotate(float angle) { 
	rotationAngle = angle;
}

int SpriteComponent::getRenderLayer() const { 
	return renderLayer;
}

float SpriteComponent::getSubLayer() const { 
	return subLayer;
}

const std::shared_ptr<sf::Sprite> SpriteComponent::getSprite() const { 
	return sprite;
}

void SpriteComponent::setAnimatable(SpriteLoader& spriteLoader, Animatable animatable, bool loopAnimatable) {
	this->rotationType = animatable.getRotationType();

	if (animatable.isSprite()) {
		// Cancel current animation
		setAnimation(nullptr);
		updateSprite(spriteLoader.getSprite(animatable.getAnimatableName(), animatable.getSpriteSheetName()));
	} else {
		std::unique_ptr<Animation> animation = spriteLoader.getAnimation(animatable.getAnimatableName(), animatable.getSpriteSheetName(), loopAnimatable);
		if (animation) {
			setAnimation(std::move(animation));
		} else {
			// Default to missing sprite
			setAnimation(nullptr);
			updateSprite(spriteLoader.getMissingSprite());
		}
	}
}

void SpriteComponent::setEffectAnimation(std::unique_ptr<SpriteEffectAnimation> effectAnimation) { 
	this->effectAnimation = std::move(effectAnimation);
}

void SpriteComponent::setRotation(float angle) { 
	sprite->setRotation(angle);
}

void SpriteComponent::setScale(float x, float y) { 
	sprite->setScale(x, y);
}

bool SpriteComponent::usesShader() const {
	if (effectAnimation == nullptr) {
		return false;
	}
	return effectAnimation->usesShader();
}

sf::Shader& SpriteComponent::getShader() {
	assert(effectAnimation != nullptr);
	return effectAnimation->getShader();
}

ROTATION_TYPE SpriteComponent::getRotationType() { 
	return rotationType;
}

float SpriteComponent::getInheritedRotationAngle() const {
	if (rotationType == ROTATION_TYPE::LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT) {
		if (lastFacedRight) return 0;
		return -PI;
	}
	return rotationAngle;
}

void SpriteComponent::updateSprite(sf::Sprite newSprite) { 
	*sprite = newSprite;
}

void SpriteComponent::updateSprite(std::shared_ptr<sf::Sprite> newSprite) {
	if (!newSprite) {
		return;
	}
	
	if (!sprite) {
		// SpriteEffectAnimations can change SpriteComponent's Sprite, so create a new Sprite object to avoid 
		// accidentally modifying the parameter Sprite pointer's object
		sprite = std::make_shared<sf::Sprite>(*newSprite);
		if (effectAnimation != nullptr) {
			effectAnimation->setSpritePointer(sprite);
		}
	} else {
		updateSprite(*newSprite);
	}
}

void SpriteComponent::setAnimation(std::unique_ptr<Animation> animation) {
	if (animation) {
		this->animation = std::move(animation);
	} else {
		this->animation = nullptr;
	}
	update(0);
}