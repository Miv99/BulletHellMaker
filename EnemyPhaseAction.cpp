#include "EnemyPhaseAction.h"
#include "Components.h"

std::string NullEPA::format() const {
	return formatString("NullEPA");
}

std::string DespawnEPA::format() const {
	return formatString("DespawnEPA");
}

void DespawnEPA::load(std::string formattedString) {
}

void DespawnEPA::execute(entt::DefaultRegistry & registry, uint32_t entity) {
	if (registry.has<DespawnComponent>(entity)) {
		registry.get<DespawnComponent>(entity).setMaxTime(0.0f);
	} else {
		registry.assign<DespawnComponent>(entity, 0.0f);
	}
}

std::string DestroyEnemyBulletsEPA::format() const {
	return formatString("DestroyEnemyBulletsEPA");
}

void DestroyEnemyBulletsEPA::load(std::string formattedString) {
}

void DestroyEnemyBulletsEPA::execute(entt::DefaultRegistry & registry, uint32_t entity) {
	auto view = registry.view<EnemyBulletComponent>();
	view.each([&](auto entity, auto& bullet) {
		if (registry.has<DespawnComponent>(entity)) {
			registry.get<DespawnComponent>(entity).setMaxTime(0.0f);
		} else {
			registry.assign<DespawnComponent>(entity, 0.0f);
		}
	});
}


std::shared_ptr<EnemyPhaseAction> EPAFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<EnemyPhaseAction> ptr;
	if (name == "DestroyEnemyBulletsEPA") {
		ptr = std::make_shared<DestroyEnemyBulletsEPA>();
	}
	else if (name == "DespawnEPA") {
		ptr = std::make_shared<DespawnEPA>();
	}
	else if (name == "NullEPA") {
		ptr = std::make_shared<NullEPA>();
	}
	ptr->load(formattedString);
	return ptr;
}
