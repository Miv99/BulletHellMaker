#include <LevelPack/EnemyPhaseAction.h>

#include <Game/Components/DespawnComponent.h>
#include <Game/Components/EnemyBulletComponent.h>

std::string NullEPA::format() const {
	return formatString("NullEPA");
}

nlohmann::json NullEPA::toJson() {
	return { 
		{"className", "NullEPA"} 
	};
}

void NullEPA::load(const nlohmann::json& j) {
}

std::string DespawnEPA::format() const {
	return formatString("DespawnEPA");
}

void DespawnEPA::load(std::string formattedString) {
}

nlohmann::json DespawnEPA::toJson() {
	return {
		{"className", "DespawnEPA"}
	};
}

void DespawnEPA::load(const nlohmann::json& j) {
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

nlohmann::json DestroyEnemyBulletsEPA::toJson() {
	return {
		{"className", "DestroyEnemyBulletsEPA"}
	};
}

void DestroyEnemyBulletsEPA::load(const nlohmann::json& j) {
}

void DestroyEnemyBulletsEPA::execute(entt::DefaultRegistry & registry, uint32_t entity) {
	auto view = registry.view<EnemyBulletComponent>();
	view.each([this, &registry](auto entity, auto& bullet) {
		if (registry.has<DespawnComponent>(entity)) {
			registry.get<DespawnComponent>(entity).setMaxTime(0.0f);
		} else {
			registry.assign<DespawnComponent>(entity, 0.0f);
		}
	});
}


std::shared_ptr<EnemyPhaseAction> EPAFactory::create(std::string formattedString) {
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
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

std::shared_ptr<EnemyPhaseAction> EPAFactory::create(const nlohmann::json& j) {
	if (j.contains("className")) {
		std::string name;
		j.at("className").get_to(name);

		std::shared_ptr<EnemyPhaseAction> ptr;
		if (name == "DestroyEnemyBulletsEPA") {
			ptr = std::make_shared<DestroyEnemyBulletsEPA>();
		} else if (name == "DespawnEPA") {
			ptr = std::make_shared<DespawnEPA>();
		} else if (name == "NullEPA") {
			ptr = std::make_shared<NullEPA>();
		}
		ptr->load(j);
		return ptr;
	} else {
		return std::make_shared<NullEPA>();
	}
}
