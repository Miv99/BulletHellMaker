#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>

#include <entt/entt.hpp>

#include <LevelPack/TextMarshallable.h>
#include <LevelPack/EnemyPhaseAction.h>
#include <LevelPack/AttackPattern.h>
#include <LevelPack/LevelPackObject.h>
#include <Game/AudioPlayer.h>

class LevelPack;

/*
An enemy phase consists of a list of attack patterns and at what time each begins.
*/
class EditorEnemyPhase : public LevelPackObject {
public:
	EditorEnemyPhase();
	EditorEnemyPhase(int id);
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

	/*
	Add an EditorAttackPattern to this enemy phase.

	time - seconds after the start of the EnemyPhase that the EditorAttackPattern starts
	id - the ID of the EditorAttackPattern
	attackPatternSymbolsDefiner - defines every symbol in the attack pattern
	*/
	void addAttackPatternID(std::string time, int id, ExprSymbolTable attackPatternSymbolsDefiner);
	/*
	Removes the EditorAttackPattern at the specified index from the list of EditorAttackPatterns to be executed.
	Should never be used in gameplay.
	*/
	void removeAttackPattern(int index);

	/*
	Returns a tuple: the amount of time after the start of this phase that the attack pattern at the given index will begin, the ID of that attack pattern,
	and the symbol_table that will be needed to get the gameplay AttackPattern with that ID.
	Note that there is no upper bound on index, since attack patterns can loop, so this function takes that into account.
	*/
	std::tuple<float, int, exprtk::symbol_table<float>> getAttackPatternData(const LevelPack& levelPack, int index);
	inline std::vector<std::tuple<std::string, int, ExprSymbolTable>> getAttackPatterns() const { return attackPatternIDs; }
	inline int getAttackPatternsCount() const { return attackPatternIDs.size(); }
	inline std::shared_ptr<EnemyPhaseAction> getPhaseBeginAction() const { return phaseBeginAction; }
	inline std::shared_ptr<EnemyPhaseAction> getPhaseEndAction() const { return phaseEndAction; }
	inline float getAttackPatternLoopDelay() const { return attackPatternLoopDelayExprCompiledValue; }
	inline bool getPlayMusic() const { return playMusic; }
	inline MusicSettings getMusicSettings() const { return musicSettings; }
	const std::map<int, int>* getAttackPatternsIDCount() const;
	bool usesAttackPattern(int attackPatternID) const;

	inline void setAttackPatternLoopDelay(std::string attackPatternLoopDelay) { this->attackPatternLoopDelay = attackPatternLoopDelay; }
	inline void setPhaseBeginAction(std::shared_ptr<EnemyPhaseAction> phaseBeginAction) { this->phaseBeginAction = phaseBeginAction; }
	inline void setPhaseEndAction(std::shared_ptr<EnemyPhaseAction> phaseEndAction) { this->phaseEndAction = phaseEndAction; }
	inline void setPlayMusic(bool playMusic) { this->playMusic = playMusic; }
	inline void setMusicSettings(MusicSettings musicSettings) { this->musicSettings = musicSettings; }

private:
	// Tuples of: When the attack pattern occurs (with t=0 being the start of the phase), the attack pattern id (int), 
	// the symbols definer, and the compiled symbols definer.
	// Sorted ascending by time.
	// This should be modified only internally. It will be populated after compileExpressions() is called. Any changes to attackPatternIDs will
	// not be reflected here if compileExpressions() is not called anytime afterwards.
	std::vector<std::tuple<float, int, exprtk::symbol_table<float>>> compiledAttackPatternIDs;
	// The expressions form of compiledAttackPatternIDs. Unordered.
	std::vector<std::tuple<std::string, int, ExprSymbolTable>> attackPatternIDs;
	// The amount of time to wait after the last attack pattern finishes (all actions have been executed and finished) before restarting the attack pattern loop
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(attackPatternLoopDelay, float, 1)

	// The EnemyPhaseAction that is executed right when this phase begins
	std::shared_ptr<EnemyPhaseAction> phaseBeginAction;
	// The EnemyPhaseAction that is executed right when this phase ends
	std::shared_ptr<EnemyPhaseAction> phaseEndAction;

	// Whether to play music on phase start
	bool playMusic = false;
	MusicSettings musicSettings;

	// Maps an EditorAttackPattern ID to the number of times it appears in attackPatternIDs.
	// This is not saved in format() but is reconstructed in load().
	std::map<int, int> attackPatternIDCount;
	// Whether lastAttackPatternActionsTotalTime has been calulated. Not saved in format().
	bool lastAttackPatternActionsTotalTimeCalculated = false;
	// Used only in gameplay. This should be updated every time there is any change to attackPatternIDs/compiledAttackPatternIDs. Not saved in format().
	float lastAttackPatternActionsTotalTime;
};