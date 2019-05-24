#include "DeathAction.h"
#include "Components.h"
#include "LevelPack.h"

std::string PlayAnimatableDeathAction::format() {
	return "PlayAnimatableDeathAction" + delim + "(" + animatable.format() + ")" + delim + tos(duration) + delim + "(" + tos((int)(effect)) + ")";
}

void PlayAnimatableDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	duration = std::stof(items[2]);
	effect = static_cast<DEATH_ANIMATION_EFFECT>(std::stoi(items[3]));
}

void PlayAnimatableDeathAction::execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	uint32_t newEntity = registry.create();
	auto& inheritedSpriteComponent = registry.get<SpriteComponent>(entity);

	auto& dyingSprite = registry.get<SpriteComponent>(entity);
	auto& spriteComponent = registry.assign<SpriteComponent>(newEntity, spriteLoader, animatable, false, dyingSprite.getRenderLayer(), dyingSprite.getSubLayer());
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


std::string PlaySoundDeathAction::format() {
	return "PlaySoundDeathAction" + delim + "(" + soundSettings.format() + ")";
}

void PlaySoundDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	soundSettings.load(items[1]);
}

void PlaySoundDeathAction::execute(LevelPack& levelPack, EntityCreationQueue & queue, entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	levelPack.playSound(soundSettings);
}

std::string ExecuteAttacksDeathAction::format() {
	std::string ret = "ExecuteAttacksDeathAction" + delim;
	ret += tos(attackIDs.size());
	for (auto id : attackIDs) {
		ret += delim + tos(id);
	}
	return ret;
}

void ExecuteAttacksDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	for (int i = 2; i < std::stoi(items[1]) + 2; i++) {
		attackIDs.push_back(std::stoi(items[i]));
	}
}

void ExecuteAttacksDeathAction::execute(LevelPack & levelPack, EntityCreationQueue & queue, entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	// Bullets from death actions have an attack pattern origin id of -1.
	// Enemy bullets also have an enemy phase origin id of -1.

	if (registry.has<PlayerTag>(entity)) {
		// Execute attacks as player
		for (auto attackID : attackIDs) {
			levelPack.getAttack(attackID)->executeAsPlayer(queue, spriteLoader, registry, entity, 0, -1);
		}
	} else {
		// Execute attacks as enemy
		int enemyID = -1;
		if (registry.has<EnemyComponent>(entity)) {
			enemyID = registry.get<EnemyComponent>(entity).getEnemyData()->getID();
		}
		for (auto attackID : attackIDs) {
			levelPack.getAttack(attackID)->executeAsEnemy(queue, spriteLoader, registry, entity, 0, -1, enemyID, -1);
		}
	}
}

std::string ParticleExplosionDeathAction::format() {
	std::string ret = "ParticleExplosionDeathAction" + delim;
	ret += tos(static_cast<int>(effect)) + delim;
	ret += tos(color.r) + delim + tos(color.g) + delim + tos(color.b) + delim + tos(color.a);
	return ret;
}

void ParticleExplosionDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	effect = static_cast<PARTICLE_EFFECT>(std::stoi(items[1]));
	color.r = std::stoi(items[2]);
	color.g = std::stoi(items[3]);
	color.b = std::stoi(items[4]);
	color.a = std::stoi(items[5]);
}

void ParticleExplosionDeathAction::execute(LevelPack & levelPack, EntityCreationQueue & queue, entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	auto& pos = registry.get<PositionComponent>(entity);
	queue.pushBack(std::make_unique<ParticleExplosionCommand>(registry, spriteLoader, pos.getX(), pos.getY(), animatable, loopAnimatable, effect, color, minParticles, maxParticles, minDistance, maxDistance, minLifespan, maxLifespan));
}

std::shared_ptr<DeathAction> DeathActionFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<DeathAction> ptr;
	if (name == "PlayAnimatableDeathAction") {
		ptr = std::make_shared<PlayAnimatableDeathAction>();
	} else if (name == "PlaySoundDeathAction") {
		ptr = std::make_shared<PlaySoundDeathAction>();
	} else if (name == "ExecuteAttacksDeathAction") {
		ptr = std::make_shared<ExecuteAttacksDeathAction>();
	} else if (name == "ParticleExplosionDeathAction") {
		ptr = std::make_shared<ParticleExplosionDeathAction>();
	}
	ptr->load(formattedString);
	return ptr;
}