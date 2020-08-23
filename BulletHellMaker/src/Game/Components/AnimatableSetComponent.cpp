#include <Game/Components/AnimatableSetComponent.h>

#include <DataStructs/SpriteLoader.h>
#include <Game/Components/SpriteComponent.h>

AnimatableSetComponent::AnimatableSetComponent() {
}

AnimatableSetComponent::AnimatableSetComponent(EntityAnimatableSet animatableSet) 
	: animatableSet(animatableSet) {
}

void AnimatableSetComponent::update(SpriteLoader& spriteLoader, float x, float y, SpriteComponent& spriteComponent, float deltaTime) {
	if (deltaTime == 0) {
		changeState(queuedState, spriteLoader, spriteComponent);
	} else {
		if ((x == lastX && y == lastY) || !firstUpdateHasBeenCalled) {
			changeState(ENTITY_ANIMATION_STATE::IDLING, spriteLoader, spriteComponent);
		} else {
			changeState(ENTITY_ANIMATION_STATE::MOVING, spriteLoader, spriteComponent);
		}

		lastX = x;
		lastY = y;
		firstUpdateHasBeenCalled = true;
	}
}

void AnimatableSetComponent::changeState(ENTITY_ANIMATION_STATE newState, SpriteLoader& spriteLoader, SpriteComponent& spriteComponent) {
	Animatable newAnimatable;
	bool changeState = false;
	bool loopNewAnimatable = false;
	if (newState == ENTITY_ANIMATION_STATE::IDLING 
		&& (currentState == ENTITY_ANIMATION_STATE::MOVING || (currentState == ENTITY_ANIMATION_STATE::ATTACKING && spriteComponent.animationIsDone())
			|| currentState == ENTITY_ANIMATION_STATE::INITIAL_NULL)) {
		// Play idle animatable
		newAnimatable = animatableSet.getIdleAnimatable();
		loopNewAnimatable = true;
		changeState = true;
	} else if (newState == ENTITY_ANIMATION_STATE::MOVING 
		&& (currentState == ENTITY_ANIMATION_STATE::IDLING || (currentState == ENTITY_ANIMATION_STATE::ATTACKING && spriteComponent.animationIsDone())
			|| currentState == ENTITY_ANIMATION_STATE::INITIAL_NULL)) {
		// Play movement animatable
		newAnimatable = animatableSet.getMovementAnimatable();
		loopNewAnimatable = true;
		changeState = true;
	} else if (newState == ENTITY_ANIMATION_STATE::ATTACKING) {
		// Play attack animatable
		newAnimatable = animatableSet.getAttackAnimatable();
		loopNewAnimatable = false;
		changeState = true;
	}
	queuedState = newState;

	if (changeState) {
		spriteComponent.setAnimatable(spriteLoader, newAnimatable, loopNewAnimatable);
		currentState = newState;
	}
}

const EntityAnimatableSet& AnimatableSetComponent::getAnimatableSet() const { 
	return animatableSet;
}

void AnimatableSetComponent::setAnimatableSet(EntityAnimatableSet animatableSet) { 
	this->animatableSet = animatableSet;
}