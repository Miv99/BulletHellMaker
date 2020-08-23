#pragma once
#include <queue>

#include <entt/entt.hpp>

class EditorMovablePoint;
class SpriteLoader;
class EntityCreationQueue;

/*
Component for an entity that will spawn enemy or player bullets after some time.
*/
class EMPSpawnerComponent {
public:
	/*
	Constructor for an enemy bullets spawner.

	emps - the EMPs that will be spawned by this component's entity and, if applicable, will be spawned with respect to parent; must be sorted ascending by time of spawn
	parent - the entity that the spawned EMPs will be spawned in reference to; may be unused depending on the EMP's spawn type
	attackID - the ID of the attack each EMP originated from
	attackPattern - same but attack pattern
	enemyID - same but enemy
	enemyPhaseID - same but enemy phase
	playAttackAnimation - whether or not to play this component's entity's attack animation
	*/
	EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, int enemyID, int enemyPhaseID, bool playAttackAnimation);
	/*
	Constructor for an player bullets spawner.

	emps - the EMPs that will be spawned by this component's entity and, if applicable, will be spawned with respect to parent; must be sorted ascending by time of spawn
	parent - the entity that the spawned EMPs will be spawned in reference to; may be unused depending on the EMP's spawn type
	attackID - the ID of the attack each EMP originated from
	attackPattern - same but attack pattern
	playAttackAnimation - whether or not to play this component's entity's attack animation
	*/
	EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, bool playAttackAnimation);

	void update(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, EntityCreationQueue& queue, float deltaTime);

private:
	enum class BULLET_TYPE {
		ENEMY,
		PLAYER
	};

	bool isEnemyBulletSpawner;

	std::queue<std::shared_ptr<EditorMovablePoint>> emps;
	uint32_t parent;
	int attackID;
	int attackPatternID;
	int enemyID;
	int enemyPhaseID;
	bool playAttackAnimation;

	// Time since this component's entity was spawned
	float time = 0;

	BULLET_TYPE bulletType;
};