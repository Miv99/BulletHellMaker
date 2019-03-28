#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <vector>
#include "TextMarshallable.h"
#include "TimeFunctionVariable.h"

/*
Actions that EditorEnemyPhases can do.
EPA for short.
*/
class EnemyPhaseAction : public TextMarshallable {
public:
	virtual std::string format() = 0;
	virtual void load(std::string formattedString) = 0;

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
	std::string format() override;
	inline void load(std::string formattedString) override {};

	inline void execute(entt::DefaultRegistry& registry, uint32_t entity) {};
};

/*
EPA for despawning self.
*/
class DespawnEPA : public EnemyPhaseAction {
public:
	std::string format() override;
	void load(std::string formattedString) override;

	void execute(entt::DefaultRegistry& registry, uint32_t entity);
};

/*
EPA for destroying all existing enemy bullets.
*/
class DestroyEnemyBulletsEPA : public EnemyPhaseAction {
public:
	std::string format() override;
	void load(std::string formattedString) override;

	void execute(entt::DefaultRegistry& registry, uint32_t entity);
};

/*
The factory for creating EPAs.
Creates the correct concrete EnemyPhaseAction using the formatted string.
*/
class EPAFactory {
public:
	static std::shared_ptr<EnemyPhaseAction> create(std::string formattedString);
};