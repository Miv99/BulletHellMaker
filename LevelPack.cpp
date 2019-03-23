#include "LevelPack.h"
#include <fstream>
#include "TextFileParser.h"
//TODO delete these
#include "EnemySpawn.h"
#include "EnemySpawnCondition.h"
#include "EntityAnimatableSet.h"

LevelPack::LevelPack(std::string name) : name(name) {
	// Read meta file
	std::ifstream metafile("Level Packs\\" + name + "\\meta.txt");
	std::unique_ptr<std::map<std::string, std::unique_ptr<std::map<std::string, std::string>>>> metadata = TextFileParser(metafile).read('=');
	std::vector<std::pair<std::string, std::string>> namePairs;
	for (auto it = metadata->at("Sprite Sheets")->begin(); it != metadata->at("Sprite Sheets")->end(); it++) {
		namePairs.push_back(std::make_pair(it->first, it->second));
	}
	spriteLoader = std::make_unique<SpriteLoader>("Level Packs\\" + name, namePairs);

	/*
	testing
	*/
	
	auto attack1 = createAttack();
	auto attack1emp0 = attack1->searchEMP(0);
	attack1emp0->setAnimatable(Animatable("Bullet", "sheet1", true));
	attack1emp0->setHitboxRadius(30);
	attack1emp0->setSpawnType(std::make_shared<EnemyAttachedEMPSpawn>(1, 0, 0));

	auto dist = std::make_shared<LinearTFV>(30, 40, 60);
	auto angle = std::make_shared<LinearTFV>(0, 3.14f * 8, 30);
	// test: add more actions
	attack1emp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(dist, angle, 60.0f));

	auto attack2 = createAttack();
	auto attack2emp0 = attack2->searchEMP(0);
	attack2emp0->setAnimatable(Animatable("Bullet", "sheet1", true));
	attack2emp0->setHitboxRadius(30);
	attack2emp0->setSpawnType(std::make_shared<EnemyAttachedEMPSpawn>(1, 0, 0));
	attack2emp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(dist, angle, 60.0f));

	auto ap1 = createAttackPattern();
	ap1->insertAction(0, std::make_shared<StayStillAtLastPositionEMPA>(5.0f));
	ap1->insertAction(1, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 50, 2), std::make_shared<ConstantTFV>(0), 2));
	/*
	ap1->insertAction(2, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 250, 2), std::make_shared<ConstantTFV>(3.14f/2), 2));
	ap1->insertAction(3, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 300, 2), std::make_shared<ConstantTFV>(3.14f), 2));
	ap1->insertAction(4, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 300, 2), std::make_shared<ConstantTFV>(3.14f * 3.0f/2), 2));
	ap1->insertAction(5, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 5, 2), std::make_shared<ConstantTFV>(0), 2));
	*/
	std::vector<sf::Vector2f> cp = {sf::Vector2f(0, 0), sf::Vector2f(-15.0f, -218.2f), sf::Vector2f(-103.0f, -166.2f), sf::Vector2f(-64.7f, -25.5f)};
	ap1->insertAction(2, std::make_shared<MoveCustomBezierEMPA>(cp, 6.0f));
	ap1->insertAction(3, std::make_shared<StayStillAtLastPositionEMPA>(1.0f));

	/*
	bool alt = false;
	for (float time = 0; time < 5; time += 0.5f) {
		if (alt) {
			ap1->addAttackID(time, attack1->getID());
		} else {
			ap1->addAttackID(time, attack2->getID());
		}
		alt = !alt;
	}
	*/
	// test: add more attack patterns

	auto ep1 = createEnemyPhase();
	ep1->addAttackPatternID(0, ap1->getID());
	ep1->addAttackPatternID(30, ap1->getID());
	ep1->setAttackPatternLoopDelay(7);
	ep1->setPhaseBeginAction(std::make_shared<NullEPA>());
	ep1->setPhaseEndAction(std::make_shared<NullEPA>());

	auto ep2 = createEnemyPhase();
	ep2->addAttackPatternID(0, ap1->getID());
	ep2->addAttackPatternID(30, ap1->getID());
	ep2->setAttackPatternLoopDelay(7);
	ep2->setPhaseBeginAction(std::make_shared<DestroyEnemyBulletsEPA>());
	ep2->setPhaseEndAction(std::make_shared<NullEPA>());

	auto enemy1 = createEnemy();
	auto e1set = EntityAnimatableSet(Animatable("Megaman idle", "sheet1", false), Animatable("Megaman movement", "sheet1", false), Animatable("Megaman attack", "sheet1", false), Animatable("oh my god he's dead", "sheet1", true));
	enemy1->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>(0), ep1->getID(), e1set);
	enemy1->addPhaseID(1, std::make_shared<TimeBasedEnemyPhaseStartCondition>(3), ep2->getID(), e1set);
	enemy1->setHealth(100);
	enemy1->setHitboxRadius(70);
	enemy1->setName("test enemy 1");

	auto level = std::make_shared<Level>("test level 1");
	auto v1 = std::vector<EnemySpawnInfo>();
	v1.push_back(EnemySpawnInfo(enemy1->getID(), 300, 300));
	level->insertEnemySpawns(0, std::make_shared<TimeBasedEnemySpawnCondition>(0), v1);
	this->insertLevel(0, level);
	

	
	// load();
}

void LevelPack::load() {
	// First line is always the next ID
	// Every other line is the data for the object
	// Read levels
	std::ifstream levelsFile("Level Packs\\" + name + "\\levels.txt");
	std::string line;
	while (std::getline(levelsFile, line)) {
		std::shared_ptr<Level> level = std::make_shared<Level>();
		level->load(line);
		levels.push_back(level);
	}
	levelsFile.close();

	// Read attacks
	std::ifstream attacksFile("Level Packs\\" + name + "\\attacks.txt");
	std::getline(attacksFile, line);
	nextAttackID = std::stoi(line);
	while (std::getline(attacksFile, line)) {
		std::shared_ptr<EditorAttack> attack = std::make_shared<EditorAttack>();
		attack->load(line);
		assert(attacks.find(attack->getID()) != attacks.end() && "Attack ID conflict");
		attacks[attack->getID()] = attack;
	}
	attacksFile.close();

	// Read attack patterns
	std::ifstream attackPatternsFile("Level Packs\\" + name + "\\attack_patterns.txt");
	std::getline(attackPatternsFile, line);
	nextAttackPatternID = std::stoi(line);
	while (std::getline(attackPatternsFile, line)) {
		std::shared_ptr<EditorAttackPattern> attackPattern = std::make_shared<EditorAttackPattern>();
		attackPattern->load(line);
		assert(attackPatterns.find(attackPattern->getID()) != attackPatterns.end() && "Attack pattern ID conflict");
		attackPatterns[attackPattern->getID()] = attackPattern;
	}
	attackPatternsFile.close();

	// Read enemies
	std::ifstream enemiesFile("Level Packs\\" + name + "\\enemies.txt");
	std::getline(enemiesFile, line);
	nextEnemyID = std::stoi(line);
	while (std::getline(enemiesFile, line)) {
		std::shared_ptr<EditorEnemy> enemy = std::make_shared<EditorEnemy>();
		enemy->load(line);
		assert(enemies.find(enemy->getID()) != enemies.end() && "Enemy ID conflict");
		enemies[enemy->getID()] = enemy;
	}
	enemiesFile.close();

	// Read enemy phases
	std::ifstream enemyPhasesFile("Level Packs\\" + name + "\\enemy_phases.txt");
	std::getline(enemyPhasesFile, line);
	nextEnemyPhaseID = std::stoi(line);
	while (std::getline(enemyPhasesFile, line)) {
		std::shared_ptr<EditorEnemyPhase> enemyPhase = std::make_shared<EditorEnemyPhase>();
		enemyPhase->load(line);
		assert(enemyPhases.find(enemyPhase->getID()) != enemyPhases.end() && "Enemy phase ID conflict");
		enemyPhases[enemyPhase->getID()] = enemyPhase;
	}
	enemyPhasesFile.close();
}

void LevelPack::save() {
	// Save levels
	std::ofstream levelsFile("Level Packs\\" + name + "\\levels.txt");
	for (auto level : levels) {
		levelsFile << level->format() << std::endl;
	}
	levelsFile.close();

	// Save attacks
	std::ofstream attacksFile("Level Packs\\" + name + "\\attacks.txt");
	attacksFile << nextAttackID << std::endl;
	for (auto p : attacks) {
		attacksFile << p.second->format() << std::endl;
	}
	attacksFile.close();

	// Save attack patterns
	std::ofstream attackPatternsFile("Level Packs\\" + name + "\\attack_patterns.txt");
	attackPatternsFile << nextAttackPatternID << std::endl;
	for (auto p : attackPatterns) {
		attackPatternsFile << p.second->format() << std::endl;
	}
	attackPatternsFile.close();

	// Save enemies
	std::ofstream enemiesFile("Level Packs\\" + name + "\\enemies.txt");
	enemiesFile << nextEnemyID << std::endl;
	for (auto p : enemies) {
		enemiesFile << p.second->format() << std::endl;
	}
	enemiesFile.close();

	// Save enemy phases
	std::ofstream enemyPhasesFile("Level Packs\\" + name + "\\enemy_phases.txt");
	enemyPhasesFile << nextEnemyPhaseID << std::endl;
	for (auto p : enemyPhases) {
		enemyPhasesFile << p.second->format() << std::endl;
	}
	enemyPhasesFile.close();
}

void LevelPack::preloadTextures() {
	spriteLoader->preloadTextures();
}

float LevelPack::searchLargestHitbox() {
	float max = 0;
	for (auto p : attacks) {
		max = std::max(max, p.second->searchLargestHitbox());
	}
	return max;
}
