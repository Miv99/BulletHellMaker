#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include "TextMarshallable.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"

/*
An attack pattern consists of a list of attacks and a list of actions executed by the enemy with the attack pattern.
An attack pattern stops when the next one begins.
If an EditorAttackPattern is being used by a player, EMPActions are unused.
*/
class EditorAttackPattern : public TextMarshallable {
public:
	inline EditorAttackPattern() {}
	inline EditorAttackPattern(int id) : id(id) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	bool legal(std::string& message) const;

	void changeEntityPathToAttackPatternActions(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag);

	inline int getID() const { return id; }
	inline std::string getName() const { return name; }
	inline const std::vector<std::pair<float, int>>& getAttacks() { return attackIDs; }
	inline std::pair<float, int> getAttackData(int index) const { return attackIDs[index]; }
	inline std::shared_ptr<EMPAction> getAction(int index) const { return actions[index]; }
	inline const std::vector<std::shared_ptr<EMPAction>> getActions() const { return actions; }
	inline int getAttacksCount() const { return attackIDs.size(); }
	inline int getActionsCount() const { return actions.size(); }
	inline float getShadowTrailInterval() const { return shadowTrailInterval; }
	inline float getShadowTrailLifespan() const { return shadowTrailLifespan; }
	inline float getActionsTotalTime() const { return actionsTotalTime; }
	inline bool usesAttack(int attackID) { return attackIDCount.count(attackID) > 0 && attackIDCount[attackID] > 0; }

	inline void setShadowTrailInterval(float shadowTrailInterval) { this->shadowTrailInterval = shadowTrailInterval; }
	inline void setShadowTrailLifespan(float shadowTrailLifespan) { this->shadowTrailLifespan = shadowTrailLifespan; }

	/*
	Add an EditorAttack to this attack pattern.

	time - seconds after the start of the EditorAttackPattern that the EditorAttack is executed
	id - the ID of the EditorAttack
	*/
	void addAttack(float time, int id);
	/*
	Removes the EditorAttack at the specified index from the list of EditorAttacks to be executed.
	*/
	void removeAttack(int index);
	// Inserts an EMPAction such that the new action is at the specified index
	void insertAction(int index, std::shared_ptr<EMPAction> action);
	void removeAction(int index);

private:
	// ID of the attack pattern
	int id;
	// User-defined name of the attack pattern
	std::string name;
	// List of attack ids (int) and when they will occur, with t=0 being the start of the attack pattern
	// Sorted ascending by time
	std::vector<std::pair<float, int>> attackIDs;
	// EMPActions that will be carried out by the enemy that has this attack pattern as soon as the previous EMPAction ends
	std::vector<std::shared_ptr<EMPAction>> actions;
	// Total time for all actions to finish execution
	float actionsTotalTime = 0;

	// Shadow trails are only for enemies
	// See ShadowTrailComponent
	float shadowTrailInterval = 0.15f;
	// Set to 0 or a negative number to disable shadow trail
	float shadowTrailLifespan = 0;

	// Maps an attack ID to the number of times it appears in attackIDs.
	// Not saved in format(), but reconstructed in load().
	std::map<int, int> attackIDCount;
};
