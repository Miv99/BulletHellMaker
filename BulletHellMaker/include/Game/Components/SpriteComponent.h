#pragma once
#include <SFML/Graphics.hpp>

#include <DataStructs/SpriteEffectAnimation.h>

enum class ROTATION_TYPE;
class SpriteLoader;
class Animatable;
class Animation;

class SpriteComponent {
public:
	/*
	renderLayer - the layer to render this sprite at. Higher means being drawn on top.
	sublayer - determines order of sprite drawing for sprites in the same layer. Higher means being drawn on top.
	*/
	SpriteComponent(int renderLayer, float subLayer);
	/*
	loopAnimatable - only applicable if animatable is an animation
	*/
	SpriteComponent(SpriteLoader& spriteLoader, Animatable animatable, bool loopAnimatable, int renderLayer, float subLayer);
	SpriteComponent(ROTATION_TYPE rotationType, std::shared_ptr<sf::Sprite> sprite, int renderLayer, float subLayer);

	void update(float deltaTime);

	/*
	angle - radians in range [-pi, pi]
	*/
	void rotate(float angle);

	int getRenderLayer() const;
	float getSubLayer() const;
	bool animationIsDone() const;
	const std::shared_ptr<sf::Sprite> getSprite() const;

	/*
	loopAnimatable - only applicable if animatable is an animation
	*/
	void setAnimatable(SpriteLoader& spriteLoader, Animatable animatable, bool loopAnimatable);
	void setEffectAnimation(std::unique_ptr<SpriteEffectAnimation> effectAnimation);
	/*
	angle - in degrees
	*/
	void setRotation(float angle);
	void setScale(float x, float y);
	bool usesShader() const;
	sf::Shader& getShader();
	ROTATION_TYPE getRotationType();
	/*
	Returns the angle of rotation of the sprite for the purpose of
	creating sprites with the same rotation angle and type.
	*/
	float getInheritedRotationAngle() const;

private:
	int renderLayer;
	float subLayer;

	// If entity has a HitboxComponent, this rotationType must match its HitboxComponent's rotationType
	ROTATION_TYPE rotationType;
	// Last faced direction used only for rotation type LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT
	bool lastFacedRight = true;
	// In radians
	float rotationAngle = 0;

	std::shared_ptr<sf::Sprite> sprite;
	// The original sprite. Used for returning to original appearance after an animation ends.
	std::shared_ptr<sf::Sprite> originalSprite;
	// Effect animation that the sprite is currently undergoing, if any
	std::unique_ptr<SpriteEffectAnimation> effectAnimation;
	// Animation that the sprite is currently undergoing, if any
	std::unique_ptr<Animation> animation;

	void updateSprite(sf::Sprite newSprite);
	void updateSprite(std::shared_ptr<sf::Sprite> newSprite);
	void setAnimation(std::unique_ptr<Animation> animation);
};