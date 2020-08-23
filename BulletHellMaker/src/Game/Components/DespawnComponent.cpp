#include <Game/Components/DespawnComponent.h>

DespawnComponent::DespawnComponent() : attachedToEntity(false), useTime(false) {}

DespawnComponent::DespawnComponent(float maxTime) : maxTime(maxTime), useTime(true) {}

DespawnComponent::DespawnComponent(entt::DefaultRegistry& registry, uint32_t entity, uint32_t self)
	: attachedTo(entity), attachedToEntity(true), useTime(false) {
	registry.get<DespawnComponent>(entity).addChild(self);
}

bool DespawnComponent::update(const entt::DefaultRegistry& registry, float deltaTime) {
	if (despawnWhenNoChildren && children.size() == 0) {
		return true;
	}
	if (attachedToEntity && !registry.valid(attachedTo)) {
		return true;
	}
	if (useTime) {
		time += deltaTime;
		if (time >= maxTime) {
			return true;
		}
	}
	return false;
}

void DespawnComponent::removeEntityAttachment(entt::DefaultRegistry& registry, uint32_t self) {
	if (attachedToEntity && registry.valid(attachedTo)) {
		registry.get<DespawnComponent>(attachedTo).removeChild(self);
	}
	attachedToEntity = false;
}

void DespawnComponent::addChild(uint32_t child) {
	children.push_back(child);
}

const std::vector<uint32_t> DespawnComponent::getChildren() const {
	return children;
}

void DespawnComponent::setMaxTime(float maxTime) {
	useTime = true;
	this->maxTime = maxTime;
	if (maxTime <= 0) {
		markedForDespawn = true;
	} else {
		markedForDespawn = false;
	}
}

void DespawnComponent::setDespawnWhenNoChildren() {
	despawnWhenNoChildren = true;
}

bool DespawnComponent::isMarkedForDespawn() const {
	return markedForDespawn;
}

std::shared_ptr<entt::SigH<void(uint32_t)>> DespawnComponent::getDespawnSignal() {
	if (despawnSignal) {
		return despawnSignal;
	}
	despawnSignal = std::make_shared<entt::SigH<void(uint32_t)>>();
	return despawnSignal;
}

void DespawnComponent::onDespawn(uint32_t self) {
	if (despawnSignal) {
		despawnSignal->publish(self);
	}
}

void DespawnComponent::removeChild(uint32_t child) {
	for (int i = 0; i < children.size(); i++) {
		if (children[i] == child) {
			children.erase(children.begin() + i);
			return;
		}
	}
}