#include <Game/Components/EMPSpawnerComponent.h>

#include <DataStructs/SpriteLoader.h>
#include <Game/EntityCreationQueue.h>
#include <LevelPack/EditorMovablePoint.h>

EMPSpawnerComponent::EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, 
	int attackPatternID, int enemyID, int enemyPhaseID, bool playAttackAnimation) 
	: parent(parent), attackID(attackID), attackPatternID(attackPatternID), enemyID(enemyID), enemyPhaseID(enemyPhaseID), 
	playAttackAnimation(playAttackAnimation), isEnemyBulletSpawner(true) {
	bulletType = BULLET_TYPE::ENEMY;
	for (auto emp : emps) {
		this->emps.push(emp);
	}
}

EMPSpawnerComponent::EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, 
	int attackPatternID, bool playAttackAnimation) 
	: parent(parent), attackID(attackID), attackPatternID(attackPatternID), playAttackAnimation(playAttackAnimation), isEnemyBulletSpawner(false) {
	bulletType = BULLET_TYPE::PLAYER;
	for (auto emp : emps) {
		this->emps.push(emp);
	}
}

void EMPSpawnerComponent::update(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, EntityCreationQueue& queue, float deltaTime) {
	time += deltaTime;

	switch (bulletType) {
	case BULLET_TYPE::ENEMY:
		while (!emps.empty()) {
			float t = emps.front()->getSpawnType()->getTime();
			if (time >= t) {
				queue.pushBack(std::make_unique<EMPSpawnFromEnemyCommand>(registry, spriteLoader, emps.front(), false, parent, 
					time - t, attackID, attackPatternID, enemyID, enemyPhaseID, playAttackAnimation));
				emps.pop();
			} else {
				break;
			}
		}
		break;
	case BULLET_TYPE::PLAYER:
		while (!emps.empty()) {
			float t = emps.front()->getSpawnType()->getTime();
			if (time >= t) {
				queue.pushBack(std::make_unique<EMPSpawnFromPlayerCommand>(registry, spriteLoader, emps.front(), false, parent, 
					time - t, attackID, attackPatternID, playAttackAnimation));
				emps.pop();
			} else {
				break;
			}
		}
		break;
	}
}