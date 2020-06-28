#include "LevelPack.h"
#include <fstream>
#include "TextFileParser.h"
#include "Attack.h"
#include "AttackPattern.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"
#include "EditorMovablePointSpawnType.h"
#include "Enemy.h"
#include "EnemyPhase.h"
#include "Player.h"
#include "Level.h"
#include "LevelPackObject.h"
#include "MovablePoint.h"
#include "Components.h"
#include "SymbolTable.h"
//TODO delete these
#include "EnemySpawn.h"
#include "LevelEventStartCondition.h"
#include "EntityAnimatableSet.h"
#include "Animatable.h"
#include "DeathAction.h"
#include "RenderSystem.h"
#include "LevelEvent.h"
#include "Constants.h"
#include "EditorMovablePointAction.h"
#include "EntityCreationQueue.h"

LevelPack::LevelPack(AudioPlayer& audioPlayer, std::string name) : audioPlayer(audioPlayer), name(name) {
	onChange = std::make_shared<entt::SigH<void()>>();


	/*
	testing
	*/
	
	auto model1 = createBulletModel();
	model1->setAnimatable(Animatable("Bullet", "sheet1", true, LOCK_ROTATION));
	model1->setDamage(1);
	model1->setName("test model");
	model1->setHitboxRadius(30);
	model1->setPlaysSound(true);
	model1->getSoundSettings().setFileName("test sound.wav");
	model1->getSoundSettings().setVolume(10);
	
	auto attack1 = createAttack();
	attack1->setPlayAttackAnimation(false);
	auto attack1emp0 = attack1->searchEMP(0);
	attack1emp0->setBulletModel(model1);
	attack1emp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(1, 0, 0));
	attack1emp0->setOnCollisionAction(PIERCE_ENTITY);
	attack1emp0->setPierceResetTime(999999);

	auto attack1emp1 = attack1emp0->createChild();
	attack1emp1->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(1, 0, 0));
	attack1emp1->setAnimatable(Animatable("Bullet2", "sheet1", true, LOCK_ROTATION));
	attack1emp1->setHitboxRadius(30);
	attack1emp1->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 200, 10), std::make_shared<ConstantTFV>(4.7f), 10));

	auto dist = std::make_shared<LinearTFV>(30, 40, 60);
	auto angle = std::make_shared<LinearTFV>(0, 3.14f * 8, 30);
	attack1emp0->insertAction(0, std::make_shared<MovePlayerHomingEMPA>(std::make_shared<LinearTFV>(0.005f, 0.025f, 5.0f), std::make_shared<ConstantTFV>(25), 30.0f));

	auto attack2 = createAttack();
	attack2->setPlayAttackAnimation(false);
	auto attack2emp0 = attack2->searchEMP(0);
	attack2emp0->setBulletModel(model1);
	attack2emp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(1, 0, 0));
	auto distanceSegments = std::make_shared<PiecewiseTFV>();
	distanceSegments->insertSegment(std::make_pair(0, std::make_shared<LinearTFV>(0, 100, 1)));
	distanceSegments->insertSegment(std::make_pair(1, std::make_shared<LinearTFV>(100, 200, 2)));
	distanceSegments->insertSegment(std::make_pair(3, std::make_shared<LinearTFV>(200, 300, 3)), 6);
	attack2emp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(distanceSegments, std::make_shared<ConstantTFV>(-PI), 6));
	attack2emp0->setOnCollisionAction(PIERCE_ENTITY);

	auto ap1 = createAttackPattern();
	ap1->setShadowTrailLifespan(3.0f);
	//ap1->insertAction(0, std::make_shared<StayStillAtLastPositionEMPA>(1.0f));
	
	ap1->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 100, 2), std::make_shared<ConstantTFV>(0), 2));
	ap1->insertAction(1, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 100, 2), std::make_shared<ConstantTFV>(PI/2.0), 2));
	ap1->insertAction(2, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 100, 2), std::make_shared<ConstantTFV>(PI), 2));
	ap1->insertAction(3, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 100, 2), std::make_shared<ConstantTFV>(3 * PI/2.0), 2));
	
	//ap1->insertAction(0, std::make_shared<MoveCustomBezierEMPA>(std::vector<sf::Vector2f>{ sf::Vector2f(0, 0), sf::Vector2f(142, 18), sf::Vector2f(189, 340), sf::Vector2f(190, 1) }, 2.5f, std::make_shared<EMPAAngleOffsetToPlayer>(0, 0)));

	bool alt = false;
	for (float time = 0; time < 5; time += 1.0f) {
		//ap1->addAttack(time, attack2->getID());
		if (alt) {
			ap1->addAttack(time, attack1->getID());
		} else {
			ap1->addAttack(time, attack2->getID());
		}
		alt = !alt;
	}
	
	// test: add more attack patterns

	auto ep1 = createEnemyPhase();
	ep1->addAttackPatternID(0, ap1->getID());
	ep1->addAttackPatternID(30, ap1->getID());
	ep1->setAttackPatternLoopDelay(15);
	ep1->setPhaseBeginAction(std::make_shared<NullEPA>());
	ep1->setPhaseEndAction(std::make_shared<NullEPA>());
	ep1->setPlayMusic(false);
	ep1->getMusicSettings().setFileName("test music.wav");
	ep1->getMusicSettings().setVolume(10);

	auto ep2 = createEnemyPhase();
	ep2->addAttackPatternID(0, ap1->getID());
	ep2->addAttackPatternID(30, ap1->getID());
	ep2->setAttackPatternLoopDelay(15);
	ep2->setPhaseBeginAction(std::make_shared<DestroyEnemyBulletsEPA>());
	ep2->setPhaseEndAction(std::make_shared<NullEPA>());
	ep2->setPlayMusic(false);
	ep2->getMusicSettings().setFileName("heaven's fall.wav");
	ep2->getMusicSettings().setVolume(10);
	ep2->getMusicSettings().setTransitionTime(5.5f);

	auto enemy1 = createEnemy();
	auto e1set = EntityAnimatableSet(Animatable("Megaman idle", "sheet1", false, LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT), 
		Animatable("Megaman movement", "sheet1", false, LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT), 
		Animatable("Megaman attack", "sheet1", false, LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT),
		std::make_shared<PlayAnimatableDeathAction>(Animatable("oh my god he's dead", "sheet1", true, LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT), PlayAnimatableDeathAction::NONE, 3.0f));
	enemy1->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>(0), ep1->getID(), e1set);
	enemy1->addPhaseID(1, std::make_shared<TimeBasedEnemyPhaseStartCondition>(10), ep2->getID(), e1set);
	enemy1->setHealth(10);
	enemy1->setHitboxRadius(70);
	enemy1->setName("test enemy 1");
	enemy1->getHurtSound().setFileName("oof.wav");
	enemy1->getHurtSound().setVolume(10);
	enemy1->getDeathSound().setFileName("death.ogg");
	enemy1->setIsBoss(true);

	std::vector<int> e1DeathAttacks;
	for (int i = 0; i < 10; i++) {
		auto atk = createAttack();
		auto emp = atk->searchEMP(0);
		emp->setAnimatable(Animatable("Bullet", "sheet1", true, ROTATE_WITH_MOVEMENT));
		emp->setHitboxRadius(30);
		emp->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(0, 0, 0));
		emp->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 700, 2.0f + (i * 0.3f)), std::make_shared<ConstantTFV>(0), 2.0f, std::make_shared<EMPAAngleOffsetToPlayer>()));
		emp->setOnCollisionAction(DESTROY_THIS_BULLET_ONLY);
		e1DeathAttacks.push_back(atk->getID());
	}
	enemy1->addDeathAction(std::make_shared<ExecuteAttacksDeathAction>(e1DeathAttacks));
	enemy1->addDeathAction(std::make_shared<ParticleExplosionDeathAction>(ParticleExplosionDeathAction::FADE_AWAY, Animatable("Bomb", "sheet1", true, LOCK_ROTATION), false, sf::Color::Yellow));

	auto level = std::make_shared<Level>("test level 1 with a really long name");
	auto playerAP = createAttackPattern();
	auto playerAttack1 = createAttack();
	auto pemp0 = playerAttack1->searchEMP(0);
	pemp0->setAnimatable(Animatable("Bullet", "sheet1", true, LOCK_ROTATION));
	pemp0->setHitboxRadius(30);
	pemp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(1, 0, 0));
	pemp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 700, 2), std::make_shared<ConstantTFV>(PI/2.0f), 2.0f));
	pemp0->setOnCollisionAction(PIERCE_ENTITY);
	playerAP->addAttack(0.1f, playerAttack1->getID());

	auto playerAP2 = createAttackPattern();
	playerAP2->addAttack(0.01f, playerAttack1->getID());

	auto playerFocusedAP = createAttackPattern();
	auto playerAttack2 = createAttack();
	auto p2emp0 = playerAttack2->searchEMP(0);
	auto p2emp1 = p2emp0->createChild();
	p2emp1->setAnimatable(Animatable("Megaman stepping", "sheet1", true, LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT));
	p2emp1->setHitboxRadius(30);
	p2emp1->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(1, 0, 0));
	p2emp1->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 700, 1.1f), std::make_shared<ConstantTFV>(PI / 2.0f), 1.1f));
	playerFocusedAP->addAttack(1.0f, attack1->getID());

	auto bombAP = createAttackPattern();
	for (int i = 0; i < 10; i++) {
		auto bombAttack1 = createAttack();
		auto b1emp0 = bombAttack1->searchEMP(0);
		b1emp0->setAnimatable(Animatable("Bullet", "sheet1", true, LOCK_ROTATION));
		b1emp0->setHitboxRadius(30);
		b1emp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(1, 0, 0));
		b1emp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 1000, 2), std::make_shared<ConstantTFV>(1.0f + i*0.13f), 2.0f));
		b1emp0->setOnCollisionAction(PIERCE_ENTITY);
		bombAP->addAttack(0, bombAttack1->getID());
	}

	auto pset1 = e1set;
	auto pset2 = EntityAnimatableSet(Animatable("Megaman idle", "sheet1", false, ROTATE_WITH_MOVEMENT),
		Animatable("Megaman movement", "sheet1", false, ROTATE_WITH_MOVEMENT),
		Animatable("Megaman attack", "sheet1", false, ROTATE_WITH_MOVEMENT),
		std::make_shared<PlayAnimatableDeathAction>(Animatable("oh my god he's dead", "sheet1", true, ROTATE_WITH_MOVEMENT), PlayAnimatableDeathAction::NONE, 3.0f));
	auto v1 = std::vector<std::shared_ptr<EnemySpawnInfo>>();
	std::vector<std::pair<std::shared_ptr<Item>, std::string>> items;
	items.push_back(std::make_pair(level->getHealthPack(), "3"));
	items.push_back(std::make_pair(level->getPointsPack(), "2"));
	items.push_back(std::make_pair(level->getPowerPack(), "60"));
	items.push_back(std::make_pair(level->getBombItem(), "2"));
	v1.push_back(std::make_shared<EnemySpawnInfo>(enemy1->getID(), "300", "300 + 50", items));
	level->insertEvent(0, std::make_shared<TimeBasedEnemySpawnCondition>("0"), std::make_shared<SpawnEnemiesLevelEvent>(v1));

	level->getHealthPack()->setActivationRadius(150);
	level->getHealthPack()->setAnimatable(Animatable("Health", "sheet1", true, LOCK_ROTATION));
	level->getHealthPack()->setHitboxRadius(40);
	level->getHealthPack()->getOnCollectSound().setFileName("item.wav");

	level->getPointsPack()->setActivationRadius(75);
	level->getPointsPack()->setAnimatable(Animatable("Points", "sheet1", true, LOCK_ROTATION));
	level->getPointsPack()->setHitboxRadius(40);
	level->getPointsPack()->getOnCollectSound().setFileName("item.wav");

	level->getPowerPack()->setActivationRadius(75);
	level->getPowerPack()->setAnimatable(Animatable("Power", "sheet1", true, LOCK_ROTATION));
	level->getPowerPack()->setHitboxRadius(40);
	level->getPowerPack()->getOnCollectSound().setFileName("item.wav");

	level->getBombItem()->setActivationRadius(75);
	level->getBombItem()->setAnimatable(Animatable("Bomb", "sheet1", true, LOCK_ROTATION));
	level->getBombItem()->setHitboxRadius(40);
	level->getBombItem()->getOnCollectSound().setFileName("item.wav");

	level->getMusicSettings().setFileName("seashore.wav");
	level->getMusicSettings().setVolume(10);
	level->getMusicSettings().setLoop(true);
	level->setBackgroundFileName("space1.png");
	level->setBackgroundScrollSpeedX(50);
	level->setBackgroundScrollSpeedY(-100);
	
	/*level->getBloomLayerSettings()[PLAYER_LAYER] = BloomSettings(1.3f, 0.0f);
	level->getBloomLayerSettings()[ENEMY_LAYER] = BloomSettings(1.3f, 0.0f);
	level->getBloomLayerSettings()[ENEMY_BOSS_LAYER] = BloomSettings(1.3f, 0.0f);
	level->getBloomLayerSettings()[PLAYER_BULLET_LAYER] = BloomSettings(1.2f, 0.05f);
	level->getBloomLayerSettings()[ENEMY_BULLET_LAYER] = BloomSettings(1.2f, 0.05f);
	level->getBloomLayerSettings()[ITEM_LAYER] = BloomSettings(1.3f, 0.0f);
	*/
	
	this->insertLevel(0, level);
	auto player = std::make_shared<EditorPlayer>();
	/*
	std::make_shared<EditorPlayer>(10, 11, 300, 100, 5, 0, 0, 2.0f, std::vector<PlayerPowerTier>{ PlayerPowerTier(pset1, playerAP->getID(), 0.1f, playerFocusedAP->getID(), 0.5f, bombAP->getID(), 5.0f),
		PlayerPowerTier(pset2, playerAP2->getID(), 0.01f, playerFocusedAP->getID(), 0.5f, bombAP->getID(), 5.0f) }, SoundSettings("oof.wav", 10), SoundSettings("death.ogg"), Animatable("heart.png", "", true, LOCK_ROTATION),
		3, 10, Animatable("bomb.png", "", true, LOCK_ROTATION), SoundSettings("bomb_ready.wav"), 5.0f)
	*/
	player->setBombInvincibilityTime(5);
	player->setBombSprite(Animatable("GUI\\bomb.png", "", true, LOCK_ROTATION));
	player->setDiscretePlayerHPSprite(Animatable("GUI\\heart.png", "", true, LOCK_ROTATION));
	player->setFocusedSpeed("150");
	player->setHitboxRadius("1");
	player->setInitialHealth("a + 3");
	player->setInvulnerabilityTime("1.0f");
	player->setMaxHealth("a + 3");
	player->setSpeed("300");
	player->insertPowerTier(0, std::make_shared<PlayerPowerTier>(pset1, playerAP->getID(), "0.1", playerFocusedAP->getID(), "0.5", bombAP->getID(), "5", "15"));
	player->insertPowerTier(1, std::make_shared<PlayerPowerTier>(pset2, playerAP2->getID(), "0.01", playerFocusedAP->getID(), "0.5", bombAP->getID(), "5", "20"));
	player->setHurtSound(SoundSettings("oof.wav", 10));
	player->setDeathSound(SoundSettings("death.ogg"));
	player->setBombReadySound(SoundSettings("bomb_ready.wav"));
	player->getSymbolTable().setSymbol("a", 11, false);
	this->setPlayer(player);

	setFontFileName("GUI\\font.ttf");
	
	metadata.addSpriteSheet("sheet1.txt", "sheet1.png");
	metadata.addSpriteSheet("Default.txt", "Default.png");
	
	//save();
	//
	//TODO: uncomment
	//load();
}

void LevelPack::load() {
	// First line is always the next ID
	// Every other line is the data for the object

	// Read metafile
	{
		std::ifstream metafile("Level Packs\\" + name + "\\meta.txt");
		std::string line;
		std::getline(metafile, line);
		metadata.load(line);
		std::getline(metafile, line);
		fontFileName = line;
	}

	// Read levels
	std::ifstream levelsFile("Level Packs\\" + name + "\\levels.txt");
	std::string line;
	while (std::getline(levelsFile, line)) {
		std::shared_ptr<Level> level = std::make_shared<Level>();
		level->load(line);
		levels.push_back(level);
	}
	levelsFile.close();

	// Read bullet models
	std::ifstream bulletModelsFile("Level Packs\\" + name + "\\bullet_models.txt");
	std::getline(bulletModelsFile, line);
	while (std::getline(bulletModelsFile, line)) {
		std::shared_ptr<BulletModel> bulletModel = std::make_shared<BulletModel>();
		bulletModel->load(line);
		assert(bulletModels.count(bulletModel->getID()) == 0 && "Bullet model ID conflict");
		bulletModels[bulletModel->getID()] = bulletModel;
	}
	bulletModelsFile.close();

	// Read attacks
	std::ifstream attacksFile("Level Packs\\" + name + "\\attacks.txt");
	std::getline(attacksFile, line);
	while (std::getline(attacksFile, line)) {
		std::shared_ptr<EditorAttack> attack = std::make_shared<EditorAttack>();
		attack->load(line);
		// Update attackIDGen with the newly loaded attack ID
		attackIDGen.markIDAsUsed(attack->getID());
		// Load bullet models for every EMP
		attack->loadEMPBulletModels(*this);
		assert(attacks.count(attack->getID()) == 0 && "Attack ID conflict");
		attacks[attack->getID()] = attack;
	}
	attacksFile.close();

	// Read attack patterns
	std::ifstream attackPatternsFile("Level Packs\\" + name + "\\attack_patterns.txt");
	std::getline(attackPatternsFile, line);
	while (std::getline(attackPatternsFile, line)) {
		std::shared_ptr<EditorAttackPattern> attackPattern = std::make_shared<EditorAttackPattern>();
		attackPattern->load(line);
		assert(attackPatterns.count(attackPattern->getID()) == 0 && "Attack pattern ID conflict");
		attackPatterns[attackPattern->getID()] = attackPattern;
	}
	attackPatternsFile.close();

	// Read enemies
	std::ifstream enemiesFile("Level Packs\\" + name + "\\enemies.txt");
	std::getline(enemiesFile, line);
	while (std::getline(enemiesFile, line)) {
		std::shared_ptr<EditorEnemy> enemy = std::make_shared<EditorEnemy>();
		enemy->load(line);
		assert(enemies.count(enemy->getID()) == 0 && "Enemy ID conflict");
		enemies[enemy->getID()] = enemy;
	}
	enemiesFile.close();

	// Read enemy phases
	std::ifstream enemyPhasesFile("Level Packs\\" + name + "\\enemy_phases.txt");
	std::getline(enemyPhasesFile, line);
	while (std::getline(enemyPhasesFile, line)) {
		std::shared_ptr<EditorEnemyPhase> enemyPhase = std::make_shared<EditorEnemyPhase>();
		enemyPhase->load(line);
		assert(enemyPhases.count(enemyPhase->getID()) == 0 && "Enemy phase ID conflict");
		enemyPhases[enemyPhase->getID()] = enemyPhase;
	}
	enemyPhasesFile.close();
}

void LevelPack::save() {
	// Save metafile
	std::ofstream metafile("Level Packs\\" + name + "\\meta.txt");
	metafile << metadata.format() << std::endl;
	metafile << fontFileName << std::endl;

	// Save levels
	std::ofstream levelsFile("Level Packs\\" + name + "\\levels.txt");
	for (auto level : levels) {
		levelsFile << level->format() << std::endl;
	}
	levelsFile.close();

	// Save bullet models
	std::ofstream bulletModelsFile("Level Packs\\" + name + "\\bullet_models.txt");
	for (auto p : bulletModels) {
		bulletModelsFile << p.second->format() << std::endl;
	}
	bulletModelsFile.close();

	// Save attacks
	std::ofstream attacksFile("Level Packs\\" + name + "\\attacks.txt");
	for (auto p : attacks) {
		// Skip if ID < 0, because that signifies that it's a temporary EditorAttack
		if (p.first < 0) continue;
		attacksFile << p.second->format() << std::endl;
	}
	attacksFile.close();

	// Save attack patterns
	std::ofstream attackPatternsFile("Level Packs\\" + name + "\\attack_patterns.txt");
	for (auto p : attackPatterns) {
		if (p.first < 0) continue;
		attackPatternsFile << p.second->format() << std::endl;
	}
	attackPatternsFile.close();

	// Save enemies
	std::ofstream enemiesFile("Level Packs\\" + name + "\\enemies.txt");
	for (auto p : enemies) {
		if (p.first < 0) continue;
		enemiesFile << p.second->format() << std::endl;
	}
	enemiesFile.close();

	// Save enemy phases
	std::ofstream enemyPhasesFile("Level Packs\\" + name + "\\enemy_phases.txt");
	for (auto p : enemyPhases) {
		if (p.first < 0) continue;
		enemyPhasesFile << p.second->format() << std::endl;
	}
	enemyPhasesFile.close();
}

std::unique_ptr<SpriteLoader> LevelPack::createSpriteLoader() {
	std::unique_ptr<SpriteLoader> spriteLoader = std::make_unique<SpriteLoader>("Level Packs\\" + name, metadata.getSpriteSheets());
	return std::move(spriteLoader);
}

void LevelPack::insertLevel(int index, std::shared_ptr<Level> level) {
	levels.insert(levels.begin() + index, level);
	onChange->publish();
}

std::shared_ptr<EditorAttack> LevelPack::createAttack() {
	auto attack = std::make_shared<EditorAttack>(attackIDGen.generateID());
	attacks[attack->getID()] = attack;
	onChange->publish();
	return attack;
}

std::shared_ptr<EditorAttack> LevelPack::createAttack(int id) {
	attackIDGen.markIDAsUsed(id);
	auto attack = std::make_shared<EditorAttack>(id);
	attacks[attack->getID()] = attack;
	onChange->publish();
	return attack;
}

std::shared_ptr<EditorAttackPattern> LevelPack::createAttackPattern() {
	auto attackPattern = std::make_shared<EditorAttackPattern>(attackPatternIDGen.generateID());
	attackPatterns[attackPattern->getID()] = attackPattern;
	onChange->publish();
	return attackPattern;
}

std::shared_ptr<EditorEnemy> LevelPack::createEnemy() {
	auto enemy = std::make_shared<EditorEnemy>(enemyIDGen.generateID());
	enemies[enemy->getID()] = enemy;
	onChange->publish();
	return enemy;
}

std::shared_ptr<EditorEnemyPhase> LevelPack::createEnemyPhase() {
	auto enemyPhase = std::make_shared<EditorEnemyPhase>(enemyPhaseIDGen.generateID());
	enemyPhases[enemyPhase->getID()] = enemyPhase;
	onChange->publish();
	return enemyPhase;
}

std::shared_ptr<BulletModel> LevelPack::createBulletModel() {
	auto bulletModel = std::make_shared<BulletModel>(bulletModelIDGen.generateID());
	bulletModels[bulletModel->getID()] = bulletModel;
	onChange->publish();
	return bulletModel;
}

void LevelPack::updateAttack(std::shared_ptr<EditorAttack> attack) {
	attackIDGen.markIDAsUsed(attack->getID());
	attacks[attack->getID()] = attack;
	onChange->publish();
}

void LevelPack::updateAttack(std::shared_ptr<LevelPackObject> attack) {
	assert(std::dynamic_pointer_cast<EditorAttack>(attack));

	attackIDGen.markIDAsUsed(attack->getID());
	attacks[attack->getID()] = std::dynamic_pointer_cast<EditorAttack>(attack);
	onChange->publish();
}

void LevelPack::updateAttackPattern(std::shared_ptr<EditorAttackPattern> attackPattern) {
	attackPatternIDGen.markIDAsUsed(attackPattern->getID());
	attackPatterns[attackPattern->getID()] = attackPattern;
	onChange->publish();
}

void LevelPack::updateAttackPattern(std::shared_ptr<LevelPackObject> attackPattern) {
	assert(std::dynamic_pointer_cast<EditorAttackPattern>(attackPattern));

	attackPatternIDGen.markIDAsUsed(attackPattern->getID());
	attackPatterns[attackPattern->getID()] = std::dynamic_pointer_cast<EditorAttackPattern>(attackPattern);
	onChange->publish();
}

void LevelPack::updateEnemy(std::shared_ptr<EditorEnemy> enemy) {
	enemyIDGen.markIDAsUsed(enemy->getID());
	enemies[enemy->getID()] = enemy;
	onChange->publish();
}

void LevelPack::updateEnemy(std::shared_ptr<LevelPackObject> enemy) {
	assert(std::dynamic_pointer_cast<EditorEnemy>(enemy));

	enemyIDGen.markIDAsUsed(enemy->getID());
	enemies[enemy->getID()] = std::dynamic_pointer_cast<EditorEnemy>(enemy);
	onChange->publish();
}

void LevelPack::updateEnemyPhase(std::shared_ptr<EditorEnemyPhase> enemyPhase) {
	enemyPhaseIDGen.markIDAsUsed(enemyPhase->getID());
	enemyPhases[enemyPhase->getID()] = enemyPhase;
	onChange->publish();
}

void LevelPack::updateEnemyPhase(std::shared_ptr<LevelPackObject> enemyPhase) {
	assert(std::dynamic_pointer_cast<EditorEnemyPhase>(enemyPhase));

	enemyPhaseIDGen.markIDAsUsed(enemyPhase->getID());
	enemyPhases[enemyPhase->getID()] = std::dynamic_pointer_cast<EditorEnemyPhase>(enemyPhase);
	onChange->publish();
}

void LevelPack::updateBulletModel(std::shared_ptr<BulletModel> bulletModel) {
	bulletModelIDGen.markIDAsUsed(bulletModel->getID());
	bulletModels[bulletModel->getID()] = bulletModel;
	onChange->publish();
}

void LevelPack::updateBulletModel(std::shared_ptr<LevelPackObject> bulletModel) {
	assert(std::dynamic_pointer_cast<BulletModel>(bulletModel));

	bulletModelIDGen.markIDAsUsed(bulletModel->getID());
	bulletModels[bulletModel->getID()] = std::dynamic_pointer_cast<BulletModel>(bulletModel);
	onChange->publish();
}

void LevelPack::deleteLevel(int levelIndex) {
	levels.erase(levels.begin() + levelIndex);
	onChange->publish();
}

void LevelPack::deleteAttack(int id) {
	attackIDGen.deleteID(id);
	attacks.erase(id);
	onChange->publish();
}

void LevelPack::deleteAttackPattern(int id) {
	attackPatternIDGen.deleteID(id);
	attackPatterns.erase(id);
	onChange->publish();
}

void LevelPack::deleteEnemy(int id) {
	enemyIDGen.deleteID (id);
	enemies.erase(id);
	onChange->publish();
}

void LevelPack::deleteEnemyPhase(int id) {
	enemyPhaseIDGen.deleteID(id);
	enemyPhases.erase(id);
	onChange->publish();
}

void LevelPack::deleteBulletModel(int id) {
	bulletModelIDGen.deleteID(id);
	bulletModels.erase(id);
	onChange->publish();
}

std::vector<int> LevelPack::getEnemyUsers(int enemyID) {
	std::vector<int> results;
	for (int i = 0; i < levels.size(); i++) {
		if (levels[i]->usesEnemy(enemyID)) {
			results.push_back(i);
		}
	}
	return results;
}

std::vector<int> LevelPack::getEditorEnemyUsers(int editorEnemyPhaseID) {
	std::vector<int> results;
	for (auto it = enemies.begin(); it != enemies.end(); it++) {
		if (it->second->usesEnemyPhase(editorEnemyPhaseID)) {
			results.push_back(it->first);
		}
	}
	return results;
}

bool LevelPack::attackPatternIsUsedByPlayer(int attackPatternID) {
	return getPlayer()->usesAttackPattern(attackPatternID);
}

std::vector<int> LevelPack::getAttackPatternEnemyUsers(int attackPatternID) {
	std::vector<int> results;
	for (auto it = enemyPhases.begin(); it != enemyPhases.end(); it++) {
		if (it->second->usesAttackPattern(attackPatternID)) {
			results.push_back(it->first);
		}
	}
	return results;
}

std::vector<int> LevelPack::getAttackUsers(int attackID) {
	std::vector<int> results;
	for (auto it = attackPatterns.begin(); it != attackPatterns.end(); it++) {
		if (it->second->usesAttack(attackID)) {
			results.push_back(it->first);
		}
	}
	return results;
}

std::vector<int> LevelPack::getBulletModelUsers(int bulletModelID) {
	std::vector<int> results;
	for (auto it = attacks.begin(); it != attacks.end(); it++) {
		if (it->second->usesBulletModel(bulletModelID)) {
			results.push_back(it->first);
		}
	}
	return results;
}

bool LevelPack::hasEnemy(int id) {
	return enemies.count(id) != 0;
}

bool LevelPack::hasEnemyPhase(int id) {
	return enemyPhases.count(id) != 0;
}

bool LevelPack::hasAttackPattern(int id) {
	return attackPatterns.count(id) != 0;
}

bool LevelPack::hasAttack(int id) {
	return attacks.count(id) != 0;
}

bool LevelPack::hasBulletModel(int id) {
	return bulletModels.count(id) != 0;
}

bool LevelPack::hasLevel(int levelIndex) {
	return levels.size() > levelIndex;
}

std::string LevelPack::getName() {
	return name;
}

std::shared_ptr<Level> LevelPack::getLevel(int levelIndex) const {
	return levels[levelIndex];
}

std::shared_ptr<Level> LevelPack::getGameplayLevel(int levelIndex) const {
	auto level = levels[levelIndex]->clone();
	// Level is a top-level object so every expression it uses should be in terms of only its own
	// unredelegated, well-defined symbols
	auto derived = std::dynamic_pointer_cast<Level>(level);
	derived->compileExpressions({});
	return derived;
}

std::shared_ptr<EditorAttack> LevelPack::getAttack(int id) const {
	return attacks.at(id);
}

std::shared_ptr<EditorAttackPattern> LevelPack::getAttackPattern(int id) const {
	return attackPatterns.at(id);
}

std::shared_ptr<EditorEnemy> LevelPack::getEnemy(int id) const {
	return enemies.at(id);
}

std::shared_ptr<EditorEnemyPhase> LevelPack::getEnemyPhase(int id) const {
	return enemyPhases.at(id);
}

std::shared_ptr<BulletModel> LevelPack::getBulletModel(int id) const {
	return bulletModels.at(id);
}

std::shared_ptr<EditorPlayer> LevelPack::getPlayer() const {
	return metadata.getPlayer();
}

std::shared_ptr<EditorPlayer> LevelPack::getGameplayPlayer() const {
	auto player = metadata.getPlayer()->clone();
	// EditorPlayer is a top-level object so every expression it uses should be in terms of only its own
	// unredelegated, well-defined symbols
	auto derived = std::dynamic_pointer_cast<EditorPlayer>(player);
	derived->compileExpressions({});
	return derived;
}

std::string LevelPack::getFontFileName() {
	return fontFileName;
}

std::map<int, std::shared_ptr<EditorAttack>>::iterator LevelPack::getAttackIteratorBegin() {
	return attacks.begin();
}

std::map<int, std::shared_ptr<EditorAttack>>::iterator LevelPack::getAttackIteratorEnd() {
	return attacks.end();
}

std::map<int, std::shared_ptr<EditorAttackPattern>>::iterator LevelPack::getAttackPatternIteratorBegin() {
	return attackPatterns.begin();
}

std::map<int, std::shared_ptr<EditorAttackPattern>>::iterator LevelPack::getAttackPatternIteratorEnd() {
	return attackPatterns.end();
}

std::map<int, std::shared_ptr<EditorEnemy>>::iterator LevelPack::getEnemyIteratorBegin() {
	return enemies.begin();
}

std::map<int, std::shared_ptr<EditorEnemy>>::iterator LevelPack::getEnemyIteratorEnd() {
	return enemies.end();
}

std::map<int, std::shared_ptr<EditorEnemyPhase>>::iterator LevelPack::getEnemyPhaseIteratorBegin() {
	return enemyPhases.begin();
}

std::map<int, std::shared_ptr<EditorEnemyPhase>>::iterator LevelPack::getEnemyPhaseIteratorEnd() {
	return enemyPhases.end();
}

std::map<int, std::shared_ptr<BulletModel>>::iterator LevelPack::getBulletModelIteratorBegin() {
	return bulletModels.begin();
}

std::map<int, std::shared_ptr<BulletModel>>::iterator LevelPack::getBulletModelIteratorEnd() {
	return bulletModels.end();
}

int LevelPack::getNextAttackID() const {
	return attackIDGen.getNextID();
}

int LevelPack::getNextAttackPatternID() const {
	return attackPatternIDGen.getNextID();
}

int LevelPack::getNextEnemyID() const {
	return enemyIDGen.getNextID();
}

int LevelPack::getNextEnemyPhaseID() const {
	return enemyPhaseIDGen.getNextID();
}

int LevelPack::getNextBulletModelID() const {
	return bulletModelIDGen.getNextID();
}

std::set<int> LevelPack::getNextAttackIDs(int count) const {
	return attackIDGen.getNextIDs(count);
}

std::set<int> LevelPack::getNextAttackPatternIDs(int count) const {
	return attackPatternIDGen.getNextIDs(count);
}

std::set<int> LevelPack::getNextEnemyIDs(int count) const {
	return enemyIDGen.getNextIDs(count);
}

std::set<int> LevelPack::getNextEnemyPhaseIDs(int count) const {
	return enemyPhaseIDGen.getNextIDs(count);
}

std::set<int> LevelPack::getNextBulletModelIDs(int count) const {
	return bulletModelIDGen.getNextIDs(count);
}

std::shared_ptr<entt::SigH<void()>> LevelPack::getOnChange() {
	if (!onChange) {
		onChange = std::make_shared<entt::SigH<void()>>();
	}
	return onChange;
}

bool LevelPack::hasBulletModel(int id) const {
	return bulletModels.count(id) > 0;
}

void LevelPack::setPlayer(std::shared_ptr<EditorPlayer> player) {
	metadata.setPlayer(player);
	onChange->publish();
}

float LevelPack::searchLargestBulletHitbox() const {
	float max = 0;
	for (auto p : attacks) {
		max = std::max(max, p.second->searchLargestHitbox());
	}
	return max;
}

float LevelPack::searchLargestItemActivationHitbox() const {
	float max = 0;
	for (auto level : levels) {
		max = std::max(max, level->getHealthPack()->getActivationRadius());
		max = std::max(max, level->getPointsPack()->getActivationRadius());
		max = std::max(max, level->getPowerPack()->getActivationRadius());
		max = std::max(max, level->getBombItem()->getActivationRadius());
	}
	return max;
}

float LevelPack::searchLargestItemCollectionHitbox() const {
	float max = 0;
	for (auto level : levels) {
		max = std::max(max, level->getHealthPack()->getHitboxRadius());
		max = std::max(max, level->getPointsPack()->getHitboxRadius());
		max = std::max(max, level->getPowerPack()->getHitboxRadius());
		max = std::max(max, level->getBombItem()->getHitboxRadius());
	}
	return max;
}

void LevelPack::playSound(const SoundSettings & soundSettings) const {
	if (soundSettings.getFileName() == "") return;
	SoundSettings alteredPath = SoundSettings(soundSettings);
	alteredPath.setFileName("Level Packs/" + name + "/Sounds/" + alteredPath.getFileName());
	audioPlayer.playSound(alteredPath);
}

void LevelPack::playMusic(const MusicSettings & musicSettings) const {
	if (musicSettings.getFileName() == "") return;
	MusicSettings alteredPath = MusicSettings(musicSettings);
	alteredPath.setFileName("Level Packs/" + name + "/Music/" + alteredPath.getFileName());
	audioPlayer.playMusic(alteredPath);
}

std::string LevelPackMetadata::format() const {
	std::string ret = formatTMObject(*player) + tos(spriteSheets.size());
	for (auto p : spriteSheets) {
		ret += formatString(p.first) + formatString(p.second);
	}
	return ret;
}

void LevelPackMetadata::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	player = std::make_shared<EditorPlayer>();
	player->load(items[0]);
	int i;
	for (i = 2; i < std::stoi(items[1]) + 2; i += 2) {
		addSpriteSheet(items[i], items[i + 1]);
	}
}

std::shared_ptr<EditorPlayer> LevelPackMetadata::getPlayer() const {
	return player;
}

void LevelPackMetadata::setPlayer(std::shared_ptr<EditorPlayer> player) {
	this->player = player;
}
