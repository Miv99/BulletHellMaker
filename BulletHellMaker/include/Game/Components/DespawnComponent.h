#pragma once
#include <entt/entt.hpp>

/*
Component for an entity that can despawn with time.
*/
class DespawnComponent {
public:
	/*
	Empty DespawnComponent that does nothing on updates
	*/
	DespawnComponent();
	/*
	maxTime - time before entity with this component despawns
	*/
	DespawnComponent(float maxTime);
	/*
	entity - the entity that, when it despawns, this component's entity will despawn. 
		This parameter entity must have a DespawnComponent.
	self - this component's entity
	*/
	DespawnComponent(entt::DefaultRegistry& registry, uint32_t entity, uint32_t self);

	/*
	Returns true if entity with this component should be despawned
	*/
	bool update(const entt::DefaultRegistry& registry, float deltaTime);

	void removeEntityAttachment(entt::DefaultRegistry& registry, uint32_t self);

	void addChild(uint32_t child);

	const std::vector<uint32_t> getChildren() const;

	void setMaxTime(float maxTime);
	void setDespawnWhenNoChildren();

	/*
	Returns true if this component's entity will be despawned in the next update call.
	This is used to make sure entities do not trigger death events multiple times
	in the same frame.
	*/
	bool isMarkedForDespawn() const;
	/*
	Returns the signal that emits the entity being despawned right before
	the actual despawn occurs.
	Parameter: the entity being despawned
	*/
	std::shared_ptr<entt::SigH<void(uint32_t)>> getDespawnSignal();

	/*
	Should be called by the system managing this component right before the 
	entity with this component is removed from the registry.
	*/
	void onDespawn(uint32_t self);

private:
	// Time since this component's entity spawned
	float time = 0;
	float maxTime;
	bool useTime = false;

	bool attachedToEntity = false;
	uint32_t attachedTo;

	// If this is true, when list of children is empty, this component's entity will despawn
	bool despawnWhenNoChildren = false;

	// Entities that are attached to this component's entity.
	// When this component's entity is deleted, so are all its recursive children.
	std::vector<uint32_t> children;

	bool markedForDespawn = false;

	// Emitted right before the actual despawn occurs.
	// Parameter: the entity being despawned
	std::shared_ptr<entt::SigH<void(uint32_t)>> despawnSignal;

	void removeChild(uint32_t child);
};