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

	std::string format() override;
	void load(std::string formattedString) override;

	bool legal(std::string& message);

	void changeEntityPathToAttackPatternActions(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag);

	inline int getID() { return id; }
	inline std::string getName() { return name; }
	inline std::pair<float, int> getAttackData(int index) { return attackIDs[index]; }
	inline std::shared_ptr<EMPAction> getAction(int index) { return actions[index]; }
	inline int getAttacksCount() { return attackIDs.size(); }
	inline int getActionsCount() { return actions.size(); }
	inline float getShadowTrailInterval() { return shadowTrailInterval; }
	inline float getShadowTrailLifespan() { return shadowTrailLifespan; }

	inline void setShadowTrailInterval(float shadowTrailInterval) { this->shadowTrailInterval = shadowTrailInterval; }
	inline void setShadowTrailLifespan(float shadowTrailLifespan) { this->shadowTrailLifespan = shadowTrailLifespan; }

	void addAttackID(float time, int id);
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

	// Shadow trails are only for enemies
	// See ShadowTrailComponent
	float shadowTrailInterval = 0.15f;
	// Set to 0 or a negative number to disable shadow trail
	float shadowTrailLifespan = 0;
};
