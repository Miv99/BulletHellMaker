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

/*
An enemy phase consists of a list of attack patterns and at what time each begins.
*/
class EditorEnemyPhase : public TextMarshallable {
public:
	inline EditorEnemyPhase() {}
	inline EditorEnemyPhase(int id) : id(id) {}

	std::string format() override;
	void load(std::string formattedString) override;

	bool legal(std::string& message);

	inline void setAttackPatternLoopDelay(float attackPatternLoopDelay) { this->attackPatternLoopDelay = attackPatternLoopDelay; }
	inline void setPhaseBeginAction(std::shared_ptr<EnemyPhaseAction> phaseBeginAction) { this->phaseBeginAction = phaseBeginAction; }
	inline void setPhaseEndAction(std::shared_ptr<EnemyPhaseAction> phaseEndAction) { this->phaseEndAction = phaseEndAction; }
	inline void setPlayMusic(bool playMusic) { this->playMusic = playMusic; }

	inline int getID() { return id; }
	inline std::pair<float, int> getAttackPatternData(int index) { 
		int size = attackPatternIds.size();
		auto item = attackPatternIds[index % size];
		// Increase time of the attack pattern at some index by the loop count multiplied by total time for all attack patterns to execute
		item.first += (attackPatternIds[size - 1].first + attackPatternLoopDelay) * (int)(index/size);
		return item;
	}
	inline int getAttackPatternsCount() { return attackPatternIds.size(); }
	inline std::shared_ptr<EnemyPhaseAction> getPhaseBeginAction() { return phaseBeginAction; }
	inline std::shared_ptr<EnemyPhaseAction> getPhaseEndAction() { return phaseEndAction; }
	inline float getAttackPatternLoopDelay() { return attackPatternLoopDelay; }
	inline bool getPlayMusic() { return playMusic; }
	inline std::string getName() { return name; }
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
	// The amount of time to wait before restarting the attack pattern loop
	float attackPatternLoopDelay = 0;

	// The EnemyPhaseAction that is executed right when this phase begins
	std::shared_ptr<EnemyPhaseAction> phaseBeginAction;
	// The EnemyPhaseAction that is executed right when this phase ends
	std::shared_ptr<EnemyPhaseAction> phaseEndAction;

	// Whether to play music on phase start
	bool playMusic = false;
	MusicSettings musicSettings;
};