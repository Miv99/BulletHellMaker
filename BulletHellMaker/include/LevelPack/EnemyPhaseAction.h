#pragma once
#include <memory>
#include <string>
#include <vector>

#include <entt/entt.hpp>

#include <LevelPack/TextMarshallable.h>
#include <DataStructs/TimeFunctionVariable.h>

/*
Actions that EditorEnemyPhases can do.
EPA for short.
*/
class EnemyPhaseAction : public TextMarshallable {
public:
	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual nlohmann::json toJson() = 0;
	virtual void load(const nlohmann::json& j) = 0;

	/*
	entity - the entity that is executing this action
	timeLag - the time elapsed since this action was supposed to execute
	*/
	virtual void execute(entt::DefaultRegistry& registry, uint32_t entity) = 0;
};

/*
Empty EPA that does nothing.
*/
class NullEPA : public EnemyPhaseAction {
public:
	std::string format() const override;
	inline void load(std::string formattedString) override {};

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	inline void execute(entt::DefaultRegistry& registry, uint32_t entity) {};
};

/*
EPA for despawning self.
*/
class DespawnEPA : public EnemyPhaseAction {
public:
	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	void execute(entt::DefaultRegistry& registry, uint32_t entity);
};

/*
EPA for destroying all existing enemy bullets.
*/
class DestroyEnemyBulletsEPA : public EnemyPhaseAction {
public:
	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	void execute(entt::DefaultRegistry& registry, uint32_t entity);
};

/*
The factory for creating EPAs.
Creates the correct concrete EnemyPhaseAction using the formatted string.
*/
class EPAFactory {
public:
	static std::shared_ptr<EnemyPhaseAction> create(std::string formattedString);
	static std::shared_ptr<EnemyPhaseAction> create(const nlohmann::json& j);
};