#include "DeathAction.h"
#include "Components.h"

std::string PlayAnimatableDeathAction::format() {
	return "PlayAnimatableDeathAction" + delim + "(" + animatable.format() + ")" + delim + tos(duration) + delim + "(" + tos((int)(effect)) + ")";
}

void PlayAnimatableDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	duration = std::stof(items[2]);
	effect = static_cast<DEATH_ANIMATION_EFFECT>(std::stoi(items[3]));
}

void PlayAnimatableDeathAction::execute(EntityCreationQueue& queue, entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	uint32_t newEntity = registry.create();
	auto& inheritedSpriteComponent = registry.get<SpriteComponent>(entity);

	auto& spriteComponent = registry.assign<SpriteComponent>(newEntity, spriteLoader, animatable, false);
	if (animatable.isSprite()) {
		registry.assign<DespawnComponent>(newEntity, duration);	
	} else {
		registry.assign<DespawnComponent>(newEntity, spriteLoader.getAnimation(animatable.getAnimatableName(), animatable.getSpriteSheetName(), false)->getTotalDuration());
	}
	spriteComponent.rotate(inheritedSpriteComponent.getInheritedRotationAngle());
	loadEffectAnimation(spriteComponent);

	auto& oldPos = registry.get<PositionComponent>(entity);
	registry.assign<PositionComponent>(newEntity, oldPos.getX(), oldPos.getY());
}

void PlayAnimatableDeathAction::loadEffectAnimation(SpriteComponent & sprite) {
	if (effect == NONE) {
		// Do nothing
	} else if (effect == SHRINK) {
		sprite.setEffectAnimation(std::make_unique<ChangeSizeSEA>(sprite.getSprite(), 0.0f, 1.0f, duration));
	} else if (effect == SHRINK) {
		sprite.setEffectAnimation(std::make_unique<ChangeSizeSEA>(sprite.getSprite(), 0.0f, 1.0f, duration));
	}
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
