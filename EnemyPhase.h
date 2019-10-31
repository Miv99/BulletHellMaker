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

class LevelPack;

/*
An enemy phase consists of a list of attack patterns and at what time each begins.
*/
class EditorEnemyPhase : public TextMarshallable {
public:
	inline EditorEnemyPhase() {}
	inline EditorEnemyPhase(int id) : id(id) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	bool legal(std::string& message)const;

	inline void setAttackPatternLoopDelay(float attackPatternLoopDelay) { this->attackPatternLoopDelay = attackPatternLoopDelay; }
	inline void setPhaseBeginAction(std::shared_ptr<EnemyPhaseAction> phaseBeginAction) { this->phaseBeginAction = phaseBeginAction; }
	inline void setPhaseEndAction(std::shared_ptr<EnemyPhaseAction> phaseEndAction) { this->phaseEndAction = phaseEndAction; }
	inline void setPlayMusic(bool playMusic) { this->playMusic = playMusic; }

	inline int getID() const { return id; }
	/*
	Returns a pair: the amount of time after the start of this phase that the attack pattern at the given index will begin, and the id of that attack pattern.
	Note that there is no upper bound on index, since attack patterns can loop, so this function takes that into account.
	*/
	std::pair<float, int> getAttackPatternData(const LevelPack& levelPack, int index) const;
	inline int getAttackPatternsCount() const { return attackPatternIds.size(); }
	inline std::shared_ptr<EnemyPhaseAction> getPhaseBeginAction() const { return phaseBeginAction; }
	inline std::shared_ptr<EnemyPhaseAction> getPhaseEndAction() const { return phaseEndAction; }
	inline float getAttackPatternLoopDelay() const { return attackPatternLoopDelay; }
	inline bool getPlayMusic() const { return playMusic; }
	inline std::string getName() const { return name; }
	/*
	Returns a reference to the music settings.
	*/
	inline MusicSettings& getMusicSettings() { return musicSettings; }

	void addAttackPatternID(float time, int id);

private:
	// ID of the phase
	int id;
	// User-defined name of the phase
	std::string name;
	// Attack pattern ids (int) and when they occur, with t=0 being the start of the phase
	// Sorted ascending by time
	std::vector<std::pair<float, int>> attackPatternIds;
	// The amount of time to wait after the last attack pattern finishes (all actions have been executed and finished) before restarting the attack pattern loop
	float attackPatternLoopDelay = 0;

	// The EnemyPhaseAction that is executed right when this phase begins
	std::shared_ptr<EnemyPhaseAction> phaseBeginAction;
	// The EnemyPhaseAction that is executed right when this phase ends
	std::shared_ptr<EnemyPhaseAction> phaseEndAction;

	// Whether to play music on phase start
	bool playMusic = false;
	MusicSettings musicSettings;
};