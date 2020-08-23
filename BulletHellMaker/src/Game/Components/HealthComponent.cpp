#include <Game/Components/HealthComponent.h>

HealthComponent::HealthComponent(int health, int maxHealth) 
	: health(health), maxHealth(maxHealth) {
}

bool HealthComponent::takeDamage(int damage) {
	health -= damage;
	onHealthChange();
	return health <= 0;
}

void HealthComponent::heal(int amount) {
	health += amount;
	if (health > maxHealth) {
		health = maxHealth;
	}
	onHealthChange();
}

std::shared_ptr<entt::SigH<void(int, int)>> HealthComponent::getHPChangeSignal() {
	if (onHealthChangeSignal) {
		return onHealthChangeSignal;
	}
	onHealthChangeSignal = std::make_shared<entt::SigH<void(int, int)>>();
	return onHealthChangeSignal;
}

void HealthComponent::onHealthChange() {
	if (onHealthChangeSignal) {
		onHealthChangeSignal->publish(health, maxHealth);
	}
}

int HealthComponent::getHealth() const { 
	return health;
}

int HealthComponent::getMaxHealth() const { 
	return maxHealth;
}

void HealthComponent::setHealth(int health) { 
	this->health = health;
}

void HealthComponent::setMaxHealth(int maxHealth) { 
	this->maxHealth = maxHealth;
}