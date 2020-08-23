#pragma once
#include <SFML/Graphics.hpp>


enum class ROTATION_TYPE;

/*
Component for an entity that has a hitbox.
A hitbox is a single circle with variable radius and origin.
*/
class HitboxComponent {
public:
	/*
	radius - hitbox radius
	x - local offset of hitbox
	y - local offset of hitbox
	*/
	HitboxComponent(ROTATION_TYPE rotationType, float radius, float x, float y);
	/*
	radius - hitbox radius
	sprite - this component's entity's sprite at the time of this HitboxComponent construction
	*/
	HitboxComponent(float radius, std::shared_ptr<sf::Sprite> sprite);

	void update(float deltaTime);

	/*
	Disable the hitbox for some amount of seconds. If the hitbox is already disabled, the new disabled time is the
	higher of this disable time and the current time left.

	Disabling a hitbox doesn't actually do anything besides make isDisabled() return true. It is up to the system to
	make use of the boolean value.
	*/
	void disable(float time);

	/*
	Rotates this hitbox some amount counter-clockwise.
	angle - radians in range [-pi, pi]
	*/
	void rotate(float angle);
	/*
	Rotates this hitbox to match the sprite's rotation.
	*/
	void rotate(std::shared_ptr<sf::Sprite> sprite);

	bool isDisabled() const;
	float getRadius() const;
	float getX() const;
	float getY() const;

	void setRadius(float radius);

private:
	// If this component's entity has a SpriteComponent, this rotationType must match its SpriteComponent's rotationType
	ROTATION_TYPE rotationType;

	float radius;
	float x = 0, y = 0;
	// Local offset of hitbox when rotated at angle 0
	float unrotatedX, unrotatedY;

	float hitboxDisabledTimeLeft = 0;
};