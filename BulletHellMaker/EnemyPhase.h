#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "EnemyPhaseAction.h"
#include "AudioPlayer.h"
#include "AttackPattern.h"
#include "LevelPackObject.h"

class LevelPack;

/*
An enemy phase consists of a list of attack patterns and at what time each begins.
*/
class EditorEnemyPhase : public LevelPackObject {
public:
	inline EditorEnemyPhase() {}
	inline EditorEnemyPhase(int id) {
		this->id = id;
	}
	/*
	Copy constructor.
	*/
	EditorEnemyPhase(std::shared_ptr<const EditorEnemyPhase> copy);
	/*
	Copy constructor.
	*/
	EditorEnemyPhase(const EditorEnemyPhase* copy);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	inline void setAttackPatternLoopDelay(float attackPatternLoopDelay) { this->attackPatternLoopDelay = attackPatternLoopDelay; }
	inline void setPhaseBeginAction(std::shared_ptr<EnemyPhaseAction> phaseBeginAction) { this->phaseBeginAction = phaseBeginAction; }
	inline void setPhaseEndAction(std::shared_ptr<EnemyPhaseAction> phaseEndAction) { this->phaseEndAction = phaseEndAction; }
	inline void setPlayMusic(bool playMusic) { this->playMusic = playMusic; }

	/*
	Returns a pair: the amount of time after the start of this phase that the attack pattern at the given index will begin, and the id of that attack pattern.
	Note that there is no upper bound on index, since attack patterns can loop, so this function takes that into account.
	*/
	std::pair<float, int> getAttackPatternData(const LevelPack& levelPack, int index) const;
	const std::vector<std::pair<float, int>>& getAttackPatterns() { return attackPatternIDs; }
	inline int getAttackPatternsCount() const { return attackPatternIDs.size(); }
	inline std::shared_ptr<EnemyPhaseAction> getPhaseBeginAction() const { return phaseBeginAction; }
	inline std::shared_ptr<EnemyPhaseAction> getPhaseEndAction() const { return phaseEndAction; }
	inline float getAttackPatternLoopDelay() const { return attackPatternLoopDelay; }
	inline bool getPlayMusic() const { return playMusic; }
	/*
	Returns a reference to the music settings.
	*/
	inline MusicSettings& getMusicSettings() { return musicSettings; }
	inline bool usesAttackPattern(int attackPatternID) { return attackPatternIDCount.count(attackPatternID) > 0 && attackPatternIDCount.at(attackPatternID) > 0; }

	/*
	Add an EditorAttackPattern to this enemy phase.

	time - seconds after the start of the EnemyPhase that the EditorAttackPattern starts
	id - the ID of the EditorAttackPattern
	*/
	void addAttackPatternID(float time, int id);
	/*
	Removes the EditorAttackPattern at the specified index from the list of EditorAttackPatterns to be executed.
	*/
	void removeAttackPattern(int index);

private:
	// Attack pattern ids (int) and when they occur, with t=0 being the start of the phase
	// Sorted ascending by time
	std::vector<std::pair<float, int>> attackPatternIDs;
	// The amount of time to wait after the last attack pattern finishes (all actions have been executed and finished) before restarting the attack pattern loop
	float attackPatternLoopDelay = 0;

	// The EnemyPhaseAction that is executed right when this phase begins
	std::shared_ptr<EnemyPhaseAction> phaseBeginAction;
	// The EnemyPhaseAction that is executed right when this phase ends
	std::shared_ptr<EnemyPhaseAction> phaseEndAction;

	// Whether to play music on phase start
	bool playMusic = false;
	MusicSettings musicSettings;

	// Maps an EditorAttackPattern ID to the number of times it appears in attackPatternIDs.
	// This is not saved on format() but is reconstructed in load().
	std::map<int, int> attackPatternIDCount;
};