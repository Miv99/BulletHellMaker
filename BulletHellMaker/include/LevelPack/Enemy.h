#pragma once
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <tuple>

#include <LevelPack/TextMarshallable.h>
#include <LevelPack/EnemyPhase.h>
#include <LevelPack/EnemyPhaseStartCondition.h>
#include <LevelPack/EntityAnimatableSet.h>
#include <LevelPack/Animatable.h>
#include <LevelPack/DeathAction.h>
#include <LevelPack/LevelPackObject.h>
#include <LevelPack/ExpressionCompilable.h>

/*
An enemy in the editor.
Spawned by EnemySpawn.

The first PhaseStartCondition must be set such that the enemy is always in a phase.
*/
class EditorEnemy : public LevelPackObject {
public:
	EditorEnemy();
	EditorEnemy(int id);
	/*
	Copy constructor.
	*/
	EditorEnemy(std::shared_ptr<const EditorEnemy> copy);
	/*
	Copy constructor.
	*/
	EditorEnemy(const EditorEnemy* copy);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	void setHitboxRadius(std::string hitboxRadius);
	void setHealth(std::string health);
	void setDespawnTime(std::string despawnTime);
	void setIsBoss(bool isBoss);

	std::tuple<std::shared_ptr<EnemyPhaseStartCondition>, int, EntityAnimatableSet, ExprSymbolTable, exprtk::symbol_table<float>> getPhaseData(int index) const;
	int getPhasesCount() const;
	float getHitboxRadius() const;
	int getHealth() const;
	float getDespawnTime() const;
	const std::vector<std::shared_ptr<DeathAction>> getDeathActions() const;
	bool getIsBoss() const;
	SoundSettings getHurtSound();
	SoundSettings getDeathSound();
	bool usesEnemyPhase(int enemyPhaseID) const;

	void setHurtSound(SoundSettings hurtSound);
	void setDeathSound(SoundSettings deathSound);

	void addDeathAction(std::shared_ptr<DeathAction> action);
	void removeDeathAction(int index);

	void addPhaseID(int index, std::shared_ptr<EnemyPhaseStartCondition> startCondition, int phaseID, EntityAnimatableSet animatableSet, ExprSymbolTable phaseSymbolsDefiner);
	void removePhaseID(int index);

private:
	// Radius of the hitbox associated with this enemy
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(hitboxRadius, float, 0)
	// Health and maximum health of this enemy
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(health, int, 1000)
	// Time it takes for this enemy to despawn. Set < 0 if it should not despawn
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(despawnTime, float, -1)
	// Tuple of: the condition to start the phase, the phase ID, the animatable set used by the enemy while in that phase, the symbols definer, and the compiled symbols definer
	// The first phase must have a TimeBasedEnemyPhaseStartCondition with t=0 to ensure that the phase can start as soon as the enemy spawns
	std::vector<std::tuple<std::shared_ptr<EnemyPhaseStartCondition>, int, EntityAnimatableSet, ExprSymbolTable, exprtk::symbol_table<float>>> phaseIDs;
	// Death actions. This should not include the death animation because the death animation is dependent on the current phase.
	std::vector<std::shared_ptr<DeathAction>> deathActions;

	bool isBoss = false;

	SoundSettings hurtSound;
	SoundSettings deathSound;

	// Maps EditorEnemyPhaseID to the number of times it appears in phaseIDs.
	// This map is not saved in format() but is reconstructed in load().
	std::map<int, int> enemyPhaseCount;
};