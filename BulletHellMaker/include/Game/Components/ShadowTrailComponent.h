#pragma once

/*
Component for an entity with a shadow trail.
*/
class ShadowTrailComponent {
public:
	/*
	interval - time inbetween each shadow's creation
	lifespan - lifespan of each shadow
	*/
	ShadowTrailComponent(float interval, float lifespan);
	/*
	Returns true if a shadow should be created at the moment of the update call.
	*/
	bool update(float deltaTime);

	float getLifespan() const;

	void setInterval(float interval);
	void setLifespan(float lifespan);

private:
	// Time inbetween each shadow's creation
	float interval;
	// Time since the last shadow was created
	float time = 0;
	// Lifespan of each shadow
	float lifespan;
};