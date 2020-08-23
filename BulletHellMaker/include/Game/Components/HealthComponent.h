#pragma once
#include <entt/entt.hpp>

/*
Component for an entity that has health.
*/
class HealthComponent {
public:
	HealthComponent(int health = 0, int maxHealth = 0);

	/*
	Take damage and returns true if health goes below 0.
	*/
	bool takeDamage(int damage);

	void heal(int amount);

	/*
	Returns the signal that is emitted whenever health changes.
	Parameters: new health and max health
	*/
	std::shared_ptr<entt::SigH<void(int, int)>> getHPChangeSignal();

	int getHealth() const;
	int getMaxHealth() const;
	void setHealth(int health);
	void setMaxHealth(int maxHealth);

private:
	int health;
	int maxHealth;

	// Emitted whenever health changes.
	// Parameters: new health and max health
	std::shared_ptr<entt::SigH<void(int, int)>> onHealthChangeSignal;

	/*
	Called whenever health or max health changes.
	*/
	void onHealthChange();
};