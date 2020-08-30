#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>

#include <LevelPack/TextMarshallable.h>
#include <LevelPack/EditorMovablePoint.h>
#include <LevelPack/EditorMovablePointAction.h>
#include <LevelPack/LevelPackObject.h>

/*
An attack pattern consists of a list of attacks and a list of actions executed by the enemy with the attack pattern.
An attack pattern stops when the next one begins.
If an EditorAttackPattern is being used by a player, EMPActions are unused.
*/
class EditorAttackPattern : public LevelPackObject {
public:
	EditorAttackPattern();
	EditorAttackPattern(int id);
	/*
	Copy constructor.
	*/
	EditorAttackPattern(std::shared_ptr<const EditorAttackPattern> copy);
	/*
	Copy constructor.
	*/
	EditorAttackPattern(const EditorAttackPattern* copy);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	void changeEntityPathToAttackPatternActions(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag);

	std::vector<std::tuple<std::string, int, ExprSymbolTable>> getAttacks();
	std::tuple<float, int, exprtk::symbol_table<float>> getAttackData(int index) const;
	std::shared_ptr<EMPAction> getAction(int index) const;
	const std::vector<std::shared_ptr<EMPAction>> getActions() const;
	int getAttacksCount() const;
	int getActionsCount() const;
	float getShadowTrailInterval() const;
	float getShadowTrailLifespan() const;
	float getActionsTotalTime() const;
	bool usesAttack(int attackID) const;

	void setShadowTrailInterval(std::string shadowTrailInterval);
	void setShadowTrailLifespan(std::string shadowTrailLifespan);
	void setActions(std::vector<std::shared_ptr<EMPAction>> actions);
	void setAttacks(std::vector<std::tuple<std::string, int, ExprSymbolTable>> attacks);

	/*
	Add an EditorAttack to this attack pattern.

	time - seconds after the start of the EditorAttackPattern that the EditorAttack is executed
	id - the ID of the EditorAttack
	*/
	void addAttack(std::string time, int id, ExprSymbolTable attackSymbolsDefiner);
	/*
	Removes the EditorAttack at the specified index from the list of EditorAttacks to be executed.
	*/
	void removeAttack(int index);
	/*
	Inserts an EMPAction such that the new action is at the specified index
	*/
	void insertAction(int index, std::shared_ptr<EMPAction> action);
	void removeAction(int index);

private:
	// Tuples of: When the attack pattern occurs (with t=0 being the start of the phase), the attack id (int), 
	// the symbols definer, and the compiled symbols definer.
	// Sorted ascending by time.
	// This should be modified only internally. It will be populated after compileExpressions() is called. Any changes to attackIDs will
	// not be reflected here if compileExpressions() is not called anytime afterwards.
	std::vector<std::tuple<float, int, exprtk::symbol_table<float>>> compiledAttackIDs;
	// The expressions form of compiledAttackIDs. Unordered.
	std::vector<std::tuple<std::string, int, ExprSymbolTable>> attackIDs;
	// EMPActions that will be carried out by the enemy that has this attack pattern as soon as the previous EMPAction ends
	std::vector<std::shared_ptr<EMPAction>> actions;
	// Total time for all actions to finish execution
	float actionsTotalTime = 0;

	// Shadow trails are only for enemies
	// See ShadowTrailComponent
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(shadowTrailInterval, float, 0.15)
	// Set to 0 or a negative number to disable shadow trail
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(shadowTrailLifespan, float, 0)

	// Maps an attack ID to the number of times it appears in attackIDs.
	// Not saved in format(), but reconstructed in load().
	std::map<int, int> attackIDCount;

	/*
	Called any time actions is modified.
	*/
	void onActionsModified();
};
