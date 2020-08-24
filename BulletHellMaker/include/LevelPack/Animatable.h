#pragma once
#include <string>

#include <LevelPack/TextMarshallable.h>

enum class ROTATION_TYPE {
	ROTATE_WITH_MOVEMENT, // Rotate depending on angle from last position to current position
	LOCK_ROTATION, // Never rotate
	LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT // Never rotate but flip sprite/hitbox across y-axis depending on angle from last position to current position
};

class Animatable : public TextMarshallable {
public:
	Animatable();
	Animatable(std::string animatableName, std::string spriteSheetName, bool animatableIsSprite, ROTATION_TYPE rotationType);

	std::string format() const override;
	void load(std::string formattedString) override;

	inline std::string getAnimatableName() const { return animatableName; }
	inline std::string getSpriteSheetName() const { return spriteSheetName; }
	inline bool isSprite() const { return animatableIsSprite; }
	inline ROTATION_TYPE getRotationType() const { return rotationType; }

	/*
	For testing.
	*/
	bool operator==(const Animatable& other) const;

private:
	// Name of sprite/animation
	std::string animatableName;
	// Name of sprite sheet the animatable is in
	std::string spriteSheetName;
	// True if the associated animatable is a sprite. False if it's an animation
	bool animatableIsSprite = true;

	ROTATION_TYPE rotationType = ROTATION_TYPE::LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT;
};