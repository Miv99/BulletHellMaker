#pragma once

#include <LevelPack/EntityAnimatableSet.h>

class SpriteComponent;
class SpriteLoader;

/*
Component for entities with an EntityAnimatableSet
*/
class AnimatableSetComponent {
public:
	enum class ENTITY_ANIMATION_STATE {
		INITIAL_NULL,
		IDLING,
		MOVING,
		ATTACKING
	};

	AnimatableSetComponent();
	AnimatableSetComponent(EntityAnimatableSet animatableSet);

	/*
	x, y - this component's entity's current global position
	spriteComponent - this component's entity's sprite component
	*/
	void update(SpriteLoader& spriteLoader, float x, float y, SpriteComponent& spriteComponent, float deltaTime);
	/*
	Changes this component's entity's animation statee.
	*/
	void changeState(ENTITY_ANIMATION_STATE newState, SpriteLoader& spriteLoader, SpriteComponent& spriteComponent);

	const EntityAnimatableSet& getAnimatableSet() const;

	void setAnimatableSet(EntityAnimatableSet animatableSet);

private:
	EntityAnimatableSet animatableSet;

	float lastX;
	float lastY;
	ENTITY_ANIMATION_STATE currentState = ENTITY_ANIMATION_STATE::INITIAL_NULL;
	ENTITY_ANIMATION_STATE queuedState = ENTITY_ANIMATION_STATE::INITIAL_NULL;
	bool firstUpdateHasBeenCalled = false;
};