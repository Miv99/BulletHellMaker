#include "DeathAction.h"
#include "Components.h"

std::string PlayAnimatableDeathAction::format() {
	return "PlayAnimatableDeathAction" + delim + "(" + animatable.format() + ")" + delim + tos(duration);
}

void PlayAnimatableDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
}

void PlayAnimatableDeathAction::execute(entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	uint32_t newEntity = registry.create();

	if (animatable.isSprite()) {
		registry.assign<DespawnComponent>(newEntity, duration);
		registry.assign<SpriteComponent>(newEntity, spriteLoader.getSprite(animatable.getAnimatableName(), animatable.getSpriteSheetName()));
	} else {
		auto animation = spriteLoader.getAnimation(animatable.getAnimatableName(), animatable.getSpriteSheetName(), false);
		registry.assign<DespawnComponent>(newEntity, animation->getTotalDuration());
		registry.assign<SpriteComponent>(newEntity).setAnimation(std::move(animation));
	}

	auto oldPos = registry.get<PositionComponent>(entity);
	registry.assign<PositionComponent>(newEntity, oldPos.getX(), oldPos.getY());
}

std::shared_ptr<DeathAction> DeathActionFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<DeathAction> ptr;
	if (name == "PlayAnimatableDeathAction") {
		ptr = std::make_shared<PlayAnimatableDeathAction>();
	}
	ptr->load(formattedString);
	return ptr;
}
