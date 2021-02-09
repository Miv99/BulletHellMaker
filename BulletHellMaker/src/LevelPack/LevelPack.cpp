#include <LevelPack/LevelPack.h>

#include <fstream>

#include <Config.h>
#include <Util/Logger.h>
#include <Util/TextFileParser.h>
#include <LevelPack/Attack.h>
#include <LevelPack/AttackPattern.h>
#include <LevelPack/EditorMovablePoint.h>
#include <LevelPack/EditorMovablePointAction.h>
#include <LevelPack/EditorMovablePointSpawnType.h>
#include <LevelPack/Enemy.h>
#include <LevelPack/EnemyPhase.h>
#include <LevelPack/Player.h>
#include <LevelPack/Level.h>
#include <LevelPack/LayerRootLevelPackObject.h>
#include <DataStructs/MovablePoint.h>
#include <DataStructs/SymbolTable.h>

//TODO delete these
#include <LevelPack/EnemySpawn.h>
#include <LevelPack/LevelEventStartCondition.h>
#include <LevelPack/EntityAnimatableSet.h>
#include <LevelPack/Animatable.h>
#include <LevelPack/DeathAction.h>
#include <Game/Systems/RenderSystem/RenderSystem.h>
#include <LevelPack/LevelEvent.h>
#include <Constants.h>
#include <LevelPack/EditorMovablePointAction.h>
#include <Game/EntityCreationQueue.h>

const std::string LevelPack::LEVELS_ORDER_FILE_NAME = "levels_order" + LEVEL_PACK_SERIALIZED_DATA_FORMAT;
const std::string LevelPack::PLAYER_FILE_NAME = "player" + LEVEL_PACK_SERIALIZED_DATA_FORMAT;

const std::string LevelPack::LEVEL_FILE_PREFIX = "level";
const std::string LevelPack::ATTACK_FILE_PREFIX = "attack";
const std::string LevelPack::ATTACK_PATTERN_FILE_PREFIX = "attack_pattern";
const std::string LevelPack::BULLET_MODEL_FILE_PREFIX = "bullet_model";
const std::string LevelPack::ENEMY_FILE_PREFIX = "enemy";
const std::string LevelPack::ENEMY_PHASE_FILE_PREFIX = "enemy_phase";

LevelPack::LevelPack(AudioPlayer& audioPlayer, std::string name, std::shared_ptr<SpriteLoader> spriteLoader) 
	: audioPlayer(audioPlayer), name(name) {

	onChange = std::make_shared<entt::SigH<void(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE, int)>>();
	if (spriteLoader) {
		this->spriteLoader = spriteLoader;
	} else {
		this->spriteLoader = std::make_shared<SpriteLoader>(name);
		this->spriteLoader->loadFromSpriteSheetsFolder();
	}

	/*
	testing
	*/

	attemptedLoad = true;
	successfulLoad = true;
	
	//auto model1 = createBulletModel();
	//model1->setAnimatable(Animatable("Bullet", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION));
	//model1->setDamage(1);
	//model1->setName("test model");
	//model1->setHitboxRadius(30);
	//model1->setPlaysSound(true);
	//model1->getSoundSettings().setFileName("test sound.wav");
	//model1->getSoundSettings().setVolume(10);
	//
	//auto attack1 = createAttack();
	//attack1->setPlayAttackAnimation(false);
	//auto attack1emp0 = attack1->searchEMP(0);
	//attack1emp0->setBulletModel(model1);
	//attack1emp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>("1", "0", "0"));
	//attack1emp0->setOnCollisionAction(BULLET_ON_COLLISION_ACTION::PIERCE_ENTITY);
	//attack1emp0->setPierceResetTime("999999");

	//auto attack1emp1 = attack1emp0->createChild();
	//attack1emp1->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>("1", "0", "0"));
	//attack1emp1->setAnimatable(Animatable("Bullet2", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION));
	//attack1emp1->setHitboxRadius("30");
	//attack1emp1->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 200, 10), std::make_shared<ConstantTFV>(4.7f), 10));

	//auto dist = std::make_shared<LinearTFV>(30, 40, 60);
	//auto angle = std::make_shared<LinearTFV>(0, 3.14f * 8, 30);
	//attack1emp0->insertAction(0, std::make_shared<MovePlayerHomingEMPA>(std::make_shared<LinearTFV>(0.005f, 0.025f, 5.0f), std::make_shared<ConstantTFV>(25), 30.0f));

	//auto attack2 = createAttack();
	//attack2->setPlayAttackAnimation(false);
	//auto attack2emp0 = attack2->searchEMP(0);
	//attack2emp0->setBulletModel(model1);
	//attack2emp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>("1", "0", "0"));
	//auto distanceSegments = std::make_shared<PiecewiseTFV>();
	//distanceSegments->insertSegment(std::make_pair(0, std::make_shared<LinearTFV>(0, 100, 1)));
	//distanceSegments->insertSegment(std::make_pair(1, std::make_shared<LinearTFV>(100, 200, 2)));
	//distanceSegments->insertSegment(std::make_pair(3, std::make_shared<LinearTFV>(200, 300, 3)), 6);
	//attack2emp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(distanceSegments, std::make_shared<ConstantTFV>(-PI), 6));
	//attack2emp0->setOnCollisionAction(BULLET_ON_COLLISION_ACTION::PIERCE_ENTITY);

	//auto ap1 = createAttackPattern();
	//ap1->setShadowTrailLifespan("3.0");
	////ap1->insertAction(0, std::make_shared<StayStillAtLastPositionEMPA>(1.0f));
	//
	//ap1->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 100, 2), std::make_shared<ConstantTFV>(0), 2));
	//ap1->insertAction(1, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 100, 2), std::make_shared<ConstantTFV>(PI/2.0), 2));
	//ap1->insertAction(2, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 100, 2), std::make_shared<ConstantTFV>(PI), 2));
	//ap1->insertAction(3, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 100, 2), std::make_shared<ConstantTFV>(3 * PI/2.0), 2));
	//
	////ap1->insertAction(0, std::make_shared<MoveCustomBezierEMPA>(std::vector<sf::Vector2f>{ sf::Vector2f(0, 0), sf::Vector2f(142, 18), sf::Vector2f(189, 340), sf::Vector2f(190, 1) }, 2.5f, std::make_shared<EMPAAngleOffsetToPlayer>(0, 0)));

	//bool alt = false;
	//for (float time = 0; time < 5; time += 1.0f) {
	//	//ap1->addAttack(time, attack2->getID());
	//	if (alt) {
	//		ap1->addAttack(std::to_string(time), attack1->getID(), ExprSymbolTable());
	//	} else {
	//		ap1->addAttack(std::to_string(time), attack2->getID(), ExprSymbolTable());
	//	}
	//	alt = !alt;
	//}
	//
	//// test: add more attack patterns

	//auto ep1 = createEnemyPhase();
	//ep1->addAttackPatternID("0", ap1->getID(), ExprSymbolTable());
	//ep1->addAttackPatternID("30", ap1->getID(), ExprSymbolTable());
	//ep1->setAttackPatternLoopDelay("15");
	//ep1->setPhaseBeginAction(std::make_shared<NullEPA>());
	//ep1->setPhaseEndAction(std::make_shared<NullEPA>());
	//ep1->setPlayMusic(true);
	//auto ep1ms = MusicSettings();
	//ep1ms.setFileName("test music.wav");
	//ep1ms.setVolume(10);
	//ep1->setMusicSettings(ep1ms);

	//auto ep2 = createEnemyPhase();
	//ep2->addAttackPatternID("0", ap1->getID(), ExprSymbolTable());
	//ep2->addAttackPatternID("30", ap1->getID(), ExprSymbolTable());
	//ep2->setAttackPatternLoopDelay("15");
	//ep2->setPhaseBeginAction(std::make_shared<DestroyEnemyBulletsEPA>());
	//ep2->setPhaseEndAction(std::make_shared<NullEPA>());
	//ep2->setPlayMusic(true);
	//auto ep2ms = MusicSettings();
	//ep2ms.setFileName("heaven's fall.wav");
	//ep2ms.setVolume(10);
	//ep2ms.setTransitionTime(5.5f);
	//ep2->setMusicSettings(ep2ms);
	//

	//auto enemy1 = createEnemy();
	//auto e1set = EntityAnimatableSet(Animatable("Megaman idle", "sheet1.png", false, ROTATION_TYPE::LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT),
	//	Animatable("Megaman movement", "sheet1.png", false, ROTATION_TYPE::LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT),
	//	Animatable("Megaman attack", "sheet1.png", false, ROTATION_TYPE::LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT),
	//	std::make_shared<PlayAnimatableDeathAction>(Animatable("oh my god he's dead", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT), PlayAnimatableDeathAction::DEATH_ANIMATION_EFFECT::NONE, "3.0"));
	//enemy1->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>("0"), ep1->getID(), e1set, ExprSymbolTable());
	//enemy1->addPhaseID(1, std::make_shared<TimeBasedEnemyPhaseStartCondition>("10"), ep2->getID(), e1set, ExprSymbolTable());
	//enemy1->setHealth("health + 5");
	//enemy1->setHitboxRadius("70");
	//enemy1->setName("test enemy 1");
	//SoundSettings e1hs;
	//e1hs.setFileName("oof.wav");
	//e1hs.setVolume(10);
	//enemy1->setHurtSound(e1hs);
	//SoundSettings e1ds;
	//e1ds.setFileName("death.ogg");
	//enemy1->setDeathSound(e1ds);
	//enemy1->setIsBoss(true);
	//ValueSymbolTable est;
	//est.setSymbol("health", -1, true);
	//enemy1->setSymbolTable(est);

	//std::vector<std::pair<int, ExprSymbolTable>> e1DeathAttacks;
	//for (int i = 0; i < 10; i++) {
	//	auto atk = createAttack();
	//	auto emp = atk->searchEMP(0);
	//	emp->setAnimatable(Animatable("Bullet", "sheet1.png", true, ROTATION_TYPE::ROTATE_WITH_MOVEMENT));
	//	emp->setHitboxRadius("30");
	//	emp->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>("0", "0", "0"));
	//	emp->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 700, 2.0f + (i * 0.3f)), std::make_shared<ConstantTFV>(0), 2.0f, std::make_shared<EMPAAngleOffsetToPlayer>()));
	//	emp->setOnCollisionAction(BULLET_ON_COLLISION_ACTION::DESTROY_THIS_BULLET_ONLY);
	//	e1DeathAttacks.push_back(std::make_pair(atk->getID(), ExprSymbolTable()));
	//}
	//enemy1->addDeathAction(std::make_shared<ExecuteAttacksDeathAction>(e1DeathAttacks));
	//enemy1->addDeathAction(std::make_shared<ParticleExplosionDeathAction>(ParticleExplosionDeathAction::PARTICLE_EFFECT::FADE_AWAY, Animatable("Bomb", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION), false, sf::Color::Yellow));

	//auto level = createLevel();
	//level->setName("test level name that is really long");
	//auto playerAP = createAttackPattern();
	//auto playerAttack1 = createAttack();
	//auto pemp0 = playerAttack1->searchEMP(0);
	//pemp0->setAnimatable(Animatable("Bullet", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION));
	//pemp0->setHitboxRadius("30");
	//pemp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>("1", "0", "0"));
	//pemp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 700, 2), std::make_shared<ConstantTFV>(PI/2.0f), 2.0f));
	//pemp0->setOnCollisionAction(BULLET_ON_COLLISION_ACTION::PIERCE_ENTITY);
	//playerAP->addAttack("0.1", playerAttack1->getID(), ExprSymbolTable());

	//auto playerAP2 = createAttackPattern();
	//playerAP2->addAttack("0.01", playerAttack1->getID(), ExprSymbolTable());

	//auto playerFocusedAP = createAttackPattern();
	//auto playerAttack2 = createAttack();
	//auto p2emp0 = playerAttack2->searchEMP(0);
	//auto p2emp1 = p2emp0->createChild();
	//p2emp1->setAnimatable(Animatable("Megaman stepping", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT));
	//p2emp1->setHitboxRadius("30");
	//p2emp1->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>("1", "0", "0"));
	//p2emp1->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 700, 1.1f), std::make_shared<ConstantTFV>(PI / 2.0f), 1.1f));
	//playerFocusedAP->addAttack("1", attack1->getID(), ExprSymbolTable());

	//auto bombAP = createAttackPattern();
	//for (int i = 0; i < 10; i++) {
	//	auto bombAttack1 = createAttack();
	//	auto b1emp0 = bombAttack1->searchEMP(0);
	//	b1emp0->setAnimatable(Animatable("Bullet", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION));
	//	b1emp0->setHitboxRadius("30");
	//	b1emp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>("1", "0", "0"));
	//	b1emp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 1000, 2), std::make_shared<ConstantTFV>(1.0f + i*0.13f), 2.0f));
	//	b1emp0->setOnCollisionAction(BULLET_ON_COLLISION_ACTION::PIERCE_ENTITY);
	//	bombAP->addAttack("0", bombAttack1->getID(), ExprSymbolTable());
	//}

	//auto pset1 = e1set;
	//auto pset2 = EntityAnimatableSet(Animatable("Megaman idle", "sheet1.png", false, ROTATION_TYPE::ROTATE_WITH_MOVEMENT),
	//	Animatable("Megaman movement", "sheet1.png", false, ROTATION_TYPE::ROTATE_WITH_MOVEMENT),
	//	Animatable("Megaman attack", "sheet1.png", false, ROTATION_TYPE::ROTATE_WITH_MOVEMENT),
	//	std::make_shared<PlayAnimatableDeathAction>(Animatable("oh my god he's dead", "sheet1.png", true, ROTATION_TYPE::ROTATE_WITH_MOVEMENT), PlayAnimatableDeathAction::DEATH_ANIMATION_EFFECT::NONE, "3.0"));
	//auto v1 = std::vector<std::shared_ptr<EnemySpawnInfo>>();
	//std::vector<std::pair<std::shared_ptr<Item>, std::string>> items;
	//items.push_back(std::make_pair(level->getHealthPack(), "3"));
	//items.push_back(std::make_pair(level->getPointsPack(), "2"));
	//items.push_back(std::make_pair(level->getPowerPack(), "60"));
	//items.push_back(std::make_pair(level->getBombItem(), "2"));
	//auto e1si = std::make_shared<EnemySpawnInfo>(enemy1->getID(), "300", "300 + 50", items);
	//ExprSymbolTable e1siDefiner;
	//e1siDefiner.setSymbol("health", "10");
	//e1si->setEnemySymbolsDefiner(e1siDefiner);
	//v1.push_back(e1si);
	//level->insertEvent(0, std::make_shared<TimeBasedEnemySpawnCondition>("0"), std::make_shared<SpawnEnemiesLevelEvent>(v1));

	//level->getHealthPack()->setActivationRadius(150);
	//level->getHealthPack()->setAnimatable(Animatable("Health", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION));
	//level->getHealthPack()->setHitboxRadius(40);
	//level->getHealthPack()->getOnCollectSound().setFileName("item.wav");

	//level->getPointsPack()->setActivationRadius(75);
	//level->getPointsPack()->setAnimatable(Animatable("Points", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION));
	//level->getPointsPack()->setHitboxRadius(40);
	//level->getPointsPack()->getOnCollectSound().setFileName("item.wav");

	//level->getPowerPack()->setActivationRadius(75);
	//level->getPowerPack()->setAnimatable(Animatable("Power", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION));
	//level->getPowerPack()->setHitboxRadius(40);
	//level->getPowerPack()->getOnCollectSound().setFileName("item.wav");

	//level->getBombItem()->setActivationRadius(75);
	//level->getBombItem()->setAnimatable(Animatable("Bomb", "sheet1.png", true, ROTATION_TYPE::LOCK_ROTATION));
	//level->getBombItem()->setHitboxRadius(40);
	//level->getBombItem()->getOnCollectSound().setFileName("item.wav");

	//MusicSettings levelMusic;
	//levelMusic.setFileName("seashore.wav");
	//levelMusic.setVolume(10);
	//levelMusic.setLoop(true);
	//levelMusic.setTransitionTime(3.0f);
	//level->setMusicSettings(levelMusic);
	//level->setBackgroundFileName("space1.png");
	//level->setBackgroundScrollSpeedX(50);
	//level->setBackgroundScrollSpeedY(-100);
	//
	//this->insertLevel(0, level->getID());
	//auto player = std::make_shared<EditorPlayer>();
	///*
	//std::make_shared<EditorPlayer>(10, 11, 300, 100, 5, 0, 0, 2.0f, std::vector<PlayerPowerTier>{ PlayerPowerTier(pset1, playerAP->getID(), 0.1f, playerFocusedAP->getID(), 0.5f, bombAP->getID(), 5.0f),
	//	PlayerPowerTier(pset2, playerAP2->getID(), 0.01f, playerFocusedAP->getID(), 0.5f, bombAP->getID(), 5.0f) }, SoundSettings("oof.wav", 10), SoundSettings("death.ogg"), Animatable("heart.png", "", true, LOCK_ROTATION),
	//	3, 10, Animatable("bomb.png", "", true, LOCK_ROTATION), SoundSettings("bomb_ready.wav"), 5.0f)
	//*/
	//player->setBombInvincibilityTime("5");
	//player->setBombGuiElementFileName("bomb.png");
	//player->setDiscretePlayerHPGuiElementFileName("heart.png");
	//player->setFocusedSpeed("150");
	//player->setHitboxRadius("1");

	//player->setInitialHealth("a + 3");
	//player->setInvulnerabilityTime("1.0");
	//player->setMaxHealth("a + 3");
	//player->setSpeed("300");
	//auto asd = std::make_shared<PlayerPowerTier>(pset1, playerAP->getID(), "0.1", playerFocusedAP->getID(), "0.5", bombAP->getID(), "5", "a");
	//auto asdTable = asd->getSymbolTable();
	//asdTable.setSymbol("a", 50, false);
	//asd->setSymbolTable(asdTable);
	//player->insertPowerTier(0, asd);
	//player->insertPowerTier(1, std::make_shared<PlayerPowerTier>(pset2, playerAP2->getID(), "0.01", playerFocusedAP->getID(), "0.5", bombAP->getID(), "5", "20"));
	//player->setHurtSound(SoundSettings("oof.wav", 10));
	//player->setDeathSound(SoundSettings("death.ogg"));
	//player->setBombReadySound(SoundSettings("bomb_ready.wav"));
	//auto playerTable = player->getSymbolTable();
	//playerTable.setSymbol("a", 11, false);
	//player->setSymbolTable(playerTable);
	//this->setPlayer(player);

	setFontFileName("font.ttf");
	
	//TODO: uncomment
	load();
	//save();
}

LevelPack::LoadMetrics LevelPack::load() {
	attemptedLoad = true;

	LoadMetrics loadMetrics;

	L_(linfo) << "Loading level pack from \"" << format(RELATIVE_LEVEL_PACK_ENEMY_PHASES_FOLDER_NAME, name.c_str()) << "\"...";

	// Read player
	player = std::make_shared<EditorPlayer>();
	std::string playerFilePath = format(RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\%s", name.c_str(), PLAYER_FILE_NAME.c_str());
	std::ifstream playerFile(playerFilePath);
	try {
		nlohmann::json j;
		playerFile >> j;
		player->load(j);
		L_(linfo) << "Successfully loaded " << playerFilePath;
		loadMetrics.playerSuccess = true;
	} catch (const std::exception& e) {
		L_(lerror) << "Failed to load " << playerFilePath << ". Exception: " << e.what();
		loadMetrics.playerSuccess = false;
	} catch (...) {
		L_(lerror) << "Failed to load " << playerFilePath << ". Unknown exception.";
		loadMetrics.playerSuccess = false;
	}
	playerFile.close();

	// Read levels
	levelsMap.clear();
	std::set<int> existingLevelFilesIDs = getAllExistingLevelPackObjectFilesIDs(format(RELATIVE_LEVEL_PACK_LEVELS_FOLDER_NAME.c_str(), name.c_str()),
		LEVEL_FILE_PREFIX, LEVEL_PACK_SERIALIZED_DATA_FORMAT);
	for (int id : existingLevelFilesIDs) {
		std::string fileName = format(RELATIVE_LEVEL_PACK_LEVELS_FOLDER_NAME + "\\%s%d%s", name.c_str(),
			LEVEL_FILE_PREFIX.c_str(), id, LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str());
		std::ifstream file(fileName);
		try {
			std::shared_ptr<Level> level = std::make_shared<Level>();

			nlohmann::json j;
			file >> j;
			level->load(j);

			if (level->getID() != id) {
				L_(lwarning) << "ID of the object in " << fileName << " does not match the file name";
			}

			levelIDGen.markIDAsUsed(level->getID());

			levelsMap[level->getID()] = level;
			L_(linfo) << "Successfully loaded " << fileName;
		} catch (const std::exception& e) {
			L_(lerror) << "Failed to load " << fileName << ". Exception: " << e.what();
			loadMetrics.levelsFailed++;
		} catch (...) {
			L_(lerror) << "Failed to load " << fileName << ". Unknown exception.";
			loadMetrics.levelsFailed++;
		}
		loadMetrics.levelsTotal++;
		file.close();
	}

	// Read levels ordering
	levels.clear();
	std::string levelsOrderingFilePath = format(RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\%s", name.c_str(), LEVELS_ORDER_FILE_NAME.c_str());
	L_(linfo) << "Loading levels order from \"" << levelsOrderingFilePath << "\"...";
	std::ifstream levelsOrderingFile(levelsOrderingFilePath);
	if (levelsOrderingFile) {
		nlohmann::json j;
		levelsOrderingFile >> j;

		if (j.contains("idOrder")) {
			j.at("idOrder").get_to(levels);
		}
	} else {
		L_(lerror) << "Unable to load " << levelsOrderingFilePath;
	}
	levelsOrderingFile.close();

	// Read bullet models
	bulletModels.clear();
	std::set<int> existingBulletModelFilesIDs = getAllExistingLevelPackObjectFilesIDs(format(RELATIVE_LEVEL_PACK_BULLET_MODELS_FOLDER_NAME.c_str(), name.c_str()),
		BULLET_MODEL_FILE_PREFIX, LEVEL_PACK_SERIALIZED_DATA_FORMAT);
	for (int id : existingBulletModelFilesIDs) {
		std::string fileName = format(RELATIVE_LEVEL_PACK_BULLET_MODELS_FOLDER_NAME + "\\%s%d%s", name.c_str(),
			BULLET_MODEL_FILE_PREFIX.c_str(), id, LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str());
		std::ifstream file(fileName);
		try {
			std::shared_ptr<BulletModel> bulletModel = std::make_shared<BulletModel>();

			nlohmann::json j;
			file >> j;
			bulletModel->load(j);

			if (bulletModel->getID() != id) {
				L_(lwarning) << "ID of the object in " << fileName << " does not match the file name";
			}

			bulletModelIDGen.markIDAsUsed(bulletModel->getID());

			bulletModels[bulletModel->getID()] = bulletModel;
			L_(linfo) << "Successfully loaded " << fileName;
		} catch (const std::exception& e) {
			L_(lerror) << "Failed to load " << fileName << ". Exception: " << e.what();
			loadMetrics.bulletModelsFailed++;
		} catch (...) {
			L_(lerror) << "Failed to load " << fileName << ". Unknown exception.";
			loadMetrics.bulletModelsFailed++;
		}
		loadMetrics.bulletModelsTotal++;
		file.close();
	}

	// Read attacks
	attacks.clear();
	std::set<int> existingAttackFilesIDs = getAllExistingLevelPackObjectFilesIDs(format(RELATIVE_LEVEL_PACK_ATTACKS_FOLDER_NAME.c_str(), name.c_str()),
		ATTACK_FILE_PREFIX, LEVEL_PACK_SERIALIZED_DATA_FORMAT);
	for (int id : existingAttackFilesIDs) {
		std::string fileName = format(RELATIVE_LEVEL_PACK_ATTACKS_FOLDER_NAME + "\\%s%d%s", name.c_str(),
			ATTACK_FILE_PREFIX.c_str(), id, LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str());
		std::ifstream file(fileName);
		try {
			std::shared_ptr<EditorAttack> attack = std::make_shared<EditorAttack>();

			nlohmann::json j;
			file >> j;
			attack->load(j);

			if (attack->getID() != id) {
				L_(lwarning) << "ID of the object in " << fileName << " does not match the file name";
			}

			attackIDGen.markIDAsUsed(attack->getID());

			// Load bullet models for every EMP
			attack->loadEMPBulletModels(*this);
			attacks[attack->getID()] = attack;
			L_(linfo) << "Successfully loaded " << fileName;
		} catch (const std::exception& e) {
			L_(lerror) << "Failed to load " << fileName << ". Exception: " << e.what();
			loadMetrics.attacksFailed++;
		} catch (...) {
			L_(lerror) << "Failed to load " << fileName << ". Unknown exception.";
			loadMetrics.attacksFailed++;
		}
		loadMetrics.attacksTotal++;
		file.close();
	}

	// Read attack patterns
	attackPatterns.clear();
	std::set<int> existingAttackPatternFilesIDs = getAllExistingLevelPackObjectFilesIDs(format(RELATIVE_LEVEL_PACK_ATTACK_PATTERNS_FOLDER_NAME.c_str(), name.c_str()),
		ATTACK_PATTERN_FILE_PREFIX, LEVEL_PACK_SERIALIZED_DATA_FORMAT);
	for (int id : existingAttackPatternFilesIDs) {
		std::string fileName = format(RELATIVE_LEVEL_PACK_ATTACK_PATTERNS_FOLDER_NAME + "\\%s%d%s", name.c_str(),
			ATTACK_PATTERN_FILE_PREFIX.c_str(), id, LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str());
		std::ifstream file(fileName);
		try {
			std::shared_ptr<EditorAttackPattern> attackPattern = std::make_shared<EditorAttackPattern>();

			nlohmann::json j;
			file >> j;
			attackPattern->load(j);

			if (attackPattern->getID() != id) {
				L_(lwarning) << "ID of the object in " << fileName << " does not match the file name";
			}

			attackPatternIDGen.markIDAsUsed(attackPattern->getID());

			attackPatterns[attackPattern->getID()] = attackPattern;
			L_(linfo) << "Successfully loaded " << fileName;
		} catch (const std::exception& e) {
			L_(lerror) << "Failed to load " << fileName << ". Exception: " << e.what();
			loadMetrics.attackPatternsFailed++;
		} catch (...) {
			L_(lerror) << "Failed to load " << fileName << ". Unknown exception.";
			loadMetrics.attackPatternsFailed++;
		}
		loadMetrics.attackPatternsTotal++;
		file.close();
	}

	// Read enemies
	enemies.clear();
	std::set<int> existingEnemyFilesIDs = getAllExistingLevelPackObjectFilesIDs(format(RELATIVE_LEVEL_PACK_ENEMIES_FOLDER_NAME.c_str(), name.c_str()),
		ENEMY_FILE_PREFIX, LEVEL_PACK_SERIALIZED_DATA_FORMAT);
	for (int id : existingEnemyFilesIDs) {
		std::string fileName = format(RELATIVE_LEVEL_PACK_ENEMIES_FOLDER_NAME + "\\%s%d%s", name.c_str(),
			ENEMY_FILE_PREFIX.c_str(), id, LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str());
		std::ifstream file(fileName);
		try {
			std::shared_ptr<EditorEnemy> enemy = std::make_shared<EditorEnemy>();

			nlohmann::json j;
			file >> j;
			enemy->load(j);

			if (enemy->getID() != id) {
				L_(lwarning) << "ID of the object in " << fileName << " does not match the file name";
			}

			enemyIDGen.markIDAsUsed(enemy->getID());

			enemies[enemy->getID()] = enemy;
			L_(linfo) << "Successfully loaded " << fileName;
		} catch (const std::exception& e) {
			L_(lerror) << "Failed to load " << fileName << ". Exception: " << e.what();
			loadMetrics.enemiesFailed++;
		} catch (...) {
			L_(lerror) << "Failed to load " << fileName << ". Unknown exception.";
			loadMetrics.enemiesFailed++;
		}
		loadMetrics.enemiesTotal++;
		file.close();
	}

	// Read enemy phases
	enemyPhases.clear();
	std::set<int> existingEnemyPhaseFilesIDs = getAllExistingLevelPackObjectFilesIDs(format(RELATIVE_LEVEL_PACK_ENEMY_PHASES_FOLDER_NAME.c_str(), name.c_str()),
		ENEMY_PHASE_FILE_PREFIX, LEVEL_PACK_SERIALIZED_DATA_FORMAT);
	for (int id : existingEnemyPhaseFilesIDs) {
		std::string fileName = format(RELATIVE_LEVEL_PACK_ENEMY_PHASES_FOLDER_NAME + "\\%s%d%s", name.c_str(),
			ENEMY_PHASE_FILE_PREFIX.c_str(), id, LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str());
		std::ifstream file(fileName);
		try {
			std::shared_ptr<EditorEnemyPhase> enemyPhase = std::make_shared<EditorEnemyPhase>();

			nlohmann::json j;
			file >> j;
			enemyPhase->load(j);

			if (enemyPhase->getID() != id) {
				L_(lwarning) << "ID of the object in " << fileName << " does not match the file name";
			}

			enemyPhaseIDGen.markIDAsUsed(enemyPhase->getID());

			enemyPhases[enemyPhase->getID()] = enemyPhase;
			L_(linfo) << "Successfully loaded " << fileName;
		} catch (const std::exception& e) {
			L_(lerror) << "Failed to load " << fileName << ". Exception: " << e.what();
			loadMetrics.enemyPhasesFailed++;
		} catch (...) {
			L_(lerror) << "Failed to load " << fileName << ". Unknown exception.";
			loadMetrics.enemyPhasesFailed++;
		}
		loadMetrics.enemyPhasesTotal++;
		file.close();
	}

	successfulLoad = loadMetrics.playerSuccess && (loadMetrics.attacksFailed == 0)
		&& (loadMetrics.attackPatternsFailed == 0) && (loadMetrics.bulletModelsFailed == 0)
		&& (loadMetrics.enemiesFailed == 0) && (loadMetrics.enemyPhasesFailed == 0)
		&& (loadMetrics.levelsFailed == 0);

	if (successfulLoad) {
		L_(linfo) << "Successfully loaded level pack from \"" << format(RELATIVE_LEVEL_PACK_ENEMY_PHASES_FOLDER_NAME, name.c_str()) << "\"";
	} else {
		L_(lerror) << "Failed to load level pack from \"" << format(RELATIVE_LEVEL_PACK_ENEMY_PHASES_FOLDER_NAME, name.c_str()) << "\"";
	}

	return loadMetrics;
}

void LevelPack::save() {
	// Create folders if they don't exist already
	if (!std::filesystem::exists(RELATIVE_LOGS_FOLDER_PATH)) {
		std::filesystem::create_directory(RELATIVE_LOGS_FOLDER_PATH);
	}
	for (std::string folder : { format(RELATIVE_LEVEL_PACK_SOUND_FOLDER_PATH, name.c_str()),
		format(RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH, name.c_str()), format(RELATIVE_LEVEL_PACK_BACKGROUNDS_FOLDER_PATH, name.c_str()),
		format(RELATIVE_LEVEL_PACK_GUI_FOLDER_PATH, name.c_str()), format(RELATIVE_LEVEL_PACK_MUSIC_FOLDER_NAME, name.c_str()),
		format(RELATIVE_LEVEL_PACK_ATTACKS_FOLDER_NAME, name.c_str()), format(RELATIVE_LEVEL_PACK_ATTACK_PATTERNS_FOLDER_NAME, name.c_str()),
		format(RELATIVE_LEVEL_PACK_BULLET_MODELS_FOLDER_NAME, name.c_str()), format(RELATIVE_LEVEL_PACK_ENEMIES_FOLDER_NAME, name.c_str()),
		format(RELATIVE_LEVEL_PACK_ENEMY_PHASES_FOLDER_NAME, name.c_str()), format(RELATIVE_LEVEL_PACK_LEVELS_FOLDER_NAME, name.c_str())}) {

		if (!std::filesystem::exists(folder)) {
			std::filesystem::create_directory(folder);
		}
	}

	// Save sprite sheets' metadata
	spriteLoader->saveMetadataFiles();

	// Save player
	std::ofstream playerFile(format(RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\%s", name.c_str(), PLAYER_FILE_NAME.c_str()));
	playerFile << std::setw(4) << player->toJson() << std::endl;
	playerFile.close();

	// Save levels, attacks, attack patterns, bullet models, enemies, and enemy phases
	for (std::tuple<std::string, std::string, std::map<int, std::shared_ptr<LayerRootLevelPackObject>>> dataTypeAttributes
		: { std::make_tuple(RELATIVE_LEVEL_PACK_LEVELS_FOLDER_NAME, LEVEL_FILE_PREFIX, std::map<int, std::shared_ptr<LayerRootLevelPackObject>>{levelsMap.begin(), levelsMap.end()}),
		std::make_tuple(RELATIVE_LEVEL_PACK_ATTACKS_FOLDER_NAME, ATTACK_FILE_PREFIX, std::map<int, std::shared_ptr<LayerRootLevelPackObject>>{attacks.begin(), attacks.end()}),
		std::make_tuple(RELATIVE_LEVEL_PACK_ATTACK_PATTERNS_FOLDER_NAME, ATTACK_PATTERN_FILE_PREFIX, std::map<int, std::shared_ptr<LayerRootLevelPackObject>>{attackPatterns.begin(), attackPatterns.end()}),
		std::make_tuple(RELATIVE_LEVEL_PACK_BULLET_MODELS_FOLDER_NAME, BULLET_MODEL_FILE_PREFIX, std::map<int, std::shared_ptr<LayerRootLevelPackObject>>{bulletModels.begin(), bulletModels.end()}),
		std::make_tuple(RELATIVE_LEVEL_PACK_ENEMIES_FOLDER_NAME, ENEMY_FILE_PREFIX, std::map<int, std::shared_ptr<LayerRootLevelPackObject>>{enemies.begin(), enemies.end()}),
		std::make_tuple(RELATIVE_LEVEL_PACK_ENEMY_PHASES_FOLDER_NAME, ENEMY_PHASE_FILE_PREFIX, std::map<int, std::shared_ptr<LayerRootLevelPackObject>>{enemyPhases.begin(), enemyPhases.end()}) }) {

		std::set<int> existingFilesIDs = getAllExistingLevelPackObjectFilesIDs(format(std::get<0>(dataTypeAttributes), name.c_str()),
			std::get<1>(dataTypeAttributes), LEVEL_PACK_SERIALIZED_DATA_FORMAT);
		std::set<int> newFilesIDs;
		for (std::pair<int, std::shared_ptr<LayerRootLevelPackObject>> p : std::get<2>(dataTypeAttributes)) {
			// Skip if ID < 0, because that signifies that it's a temporary object
			if (p.first < 0) continue;

			newFilesIDs.insert(p.first);

			std::ofstream file(format(std::get<0>(dataTypeAttributes) + "\\%s%d%s", name.c_str(), std::get<1>(dataTypeAttributes).c_str(), 
				p.first, LEVEL_PACK_SERIALIZED_DATA_FORMAT.c_str()));
			file << std::setw(4) << p.second->toJson() << std::endl;
			file.close();
		}
		// Delete all files that weren't just saved
		std::set<int> fileIDsToBeDeleted;
		std::set_difference(existingFilesIDs.begin(), existingFilesIDs.end(),
			newFilesIDs.begin(), newFilesIDs.end(),
			std::inserter(fileIDsToBeDeleted, fileIDsToBeDeleted.begin()));
		deleteLevelPackObjectFiles(format(std::get<0>(dataTypeAttributes), name.c_str()),
			std::get<1>(dataTypeAttributes), LEVEL_PACK_SERIALIZED_DATA_FORMAT, fileIDsToBeDeleted);
	}

	// Save levels ordering
	std::ofstream levelsOrderingFile(format(RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\%s", name.c_str(), LEVELS_ORDER_FILE_NAME.c_str()));
	for (int id : levels) {
		levelsOrderingFile << std::setw(4) << nlohmann::json{ {"idOrder", levels} };
	}
	levelsOrderingFile.close();
}

std::shared_ptr<SpriteLoader> LevelPack::getSpriteLoader() {
	return spriteLoader;
}

void LevelPack::insertLevel(int index, int levelID) {
	levels.insert(levels.begin() + index, levelID);
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::LEVEL, levelID);
}

std::shared_ptr<Level> LevelPack::createLevel() {
	return createLevel(levelIDGen.generateID());
}

std::shared_ptr<Level> LevelPack::createLevel(int id) {
	auto level = std::make_shared<Level>(id);
	levelsMap[level->getID()] = level;
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::LEVEL, level->getID());
	return level;
}

std::shared_ptr<EditorAttack> LevelPack::createAttack() {
	return createAttack(attackIDGen.generateID());
}

std::shared_ptr<EditorAttack> LevelPack::createAttack(int id) {
	attackIDGen.markIDAsUsed(id);
	auto attack = std::make_shared<EditorAttack>(id);
	attacks[attack->getID()] = attack;
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK, attack->getID());
	return attack;
}

std::shared_ptr<EditorAttackPattern> LevelPack::createAttackPattern() {
	return createAttackPattern(attackPatternIDGen.generateID());

}

std::shared_ptr<EditorAttackPattern> LevelPack::createAttackPattern(int id) {
	attackPatternIDGen.markIDAsUsed(id);
	auto attackPattern = std::make_shared<EditorAttackPattern>(id);
	attackPatterns[attackPattern->getID()] = attackPattern;
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK_PATTERN, attackPattern->getID());
	return attackPattern;
}

std::shared_ptr<EditorEnemy> LevelPack::createEnemy() {
	return createEnemy(enemyIDGen.generateID());

}

std::shared_ptr<EditorEnemy> LevelPack::createEnemy(int id) {
	auto enemy = std::make_shared<EditorEnemy>(id);
	enemies[enemy->getID()] = enemy;
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY, enemy->getID());
	return enemy;
}

std::shared_ptr<EditorEnemyPhase> LevelPack::createEnemyPhase() {
	return createEnemyPhase(enemyPhaseIDGen.generateID());
}

std::shared_ptr<EditorEnemyPhase> LevelPack::createEnemyPhase(int id) {
	auto enemyPhase = std::make_shared<EditorEnemyPhase>(id);
	enemyPhases[enemyPhase->getID()] = enemyPhase;
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY_PHASE, enemyPhase->getID());
	return enemyPhase;
}

std::shared_ptr<BulletModel> LevelPack::createBulletModel() {
	auto bulletModel = std::make_shared<BulletModel>(bulletModelIDGen.generateID());
	bulletModels[bulletModel->getID()] = bulletModel;
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::BULLET_MODEL, bulletModel->getID());
	return bulletModel;
}

void LevelPack::updateSpriteSheet(std::shared_ptr<SpriteSheet> spriteSheet, bool emitOnChange) {
	spriteLoader->updateSpriteSheet(spriteSheet);
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::SPRITE_SHEET, -1);
	}
}

void LevelPack::updateAttack(std::shared_ptr<EditorAttack> attack, bool emitOnChange) {
	attackIDGen.markIDAsUsed(attack->getID());
	attacks[attack->getID()] = attack;
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK, attack->getID());
	}
}

void LevelPack::updateAttack(std::shared_ptr<LayerRootLevelPackObject> attack, bool emitOnChange) {
	attackIDGen.markIDAsUsed(attack->getID());
	attacks[attack->getID()] = std::dynamic_pointer_cast<EditorAttack>(attack);
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK, attack->getID());
	}
}

void LevelPack::updateAttackPattern(std::shared_ptr<EditorAttackPattern> attackPattern, bool emitOnChange) {
	attackPatternIDGen.markIDAsUsed(attackPattern->getID());
	attackPatterns[attackPattern->getID()] = attackPattern;
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK_PATTERN, attackPattern->getID());
	}
}

void LevelPack::updateAttackPattern(std::shared_ptr<LayerRootLevelPackObject> attackPattern, bool emitOnChange) {
	attackPatternIDGen.markIDAsUsed(attackPattern->getID());
	attackPatterns[attackPattern->getID()] = std::dynamic_pointer_cast<EditorAttackPattern>(attackPattern);
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK_PATTERN, attackPattern->getID());
	}
}

void LevelPack::updateEnemy(std::shared_ptr<EditorEnemy> enemy, bool emitOnChange) {
	enemyIDGen.markIDAsUsed(enemy->getID());
	enemies[enemy->getID()] = enemy;
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY, enemy->getID());
	}
}

void LevelPack::updateEnemy(std::shared_ptr<LayerRootLevelPackObject> enemy, bool emitOnChange) {
	enemyIDGen.markIDAsUsed(enemy->getID());
	enemies[enemy->getID()] = std::dynamic_pointer_cast<EditorEnemy>(enemy);
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY, enemy->getID());
	}
}

void LevelPack::updateEnemyPhase(std::shared_ptr<EditorEnemyPhase> enemyPhase, bool emitOnChange) {
	enemyPhaseIDGen.markIDAsUsed(enemyPhase->getID());
	enemyPhases[enemyPhase->getID()] = enemyPhase;
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY_PHASE, enemyPhase->getID());
	}
}

void LevelPack::updateEnemyPhase(std::shared_ptr<LayerRootLevelPackObject> enemyPhase, bool emitOnChange) {
	enemyPhaseIDGen.markIDAsUsed(enemyPhase->getID());
	enemyPhases[enemyPhase->getID()] = std::dynamic_pointer_cast<EditorEnemyPhase>(enemyPhase);
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY_PHASE, enemyPhase->getID());
	}
}

void LevelPack::updateBulletModel(std::shared_ptr<BulletModel> bulletModel, bool emitOnChange) {
	bulletModelIDGen.markIDAsUsed(bulletModel->getID());
	bulletModels[bulletModel->getID()] = bulletModel;
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::BULLET_MODEL, bulletModel->getID());
	}
}

void LevelPack::updateBulletModel(std::shared_ptr<LayerRootLevelPackObject> bulletModel, bool emitOnChange) {
	bulletModelIDGen.markIDAsUsed(bulletModel->getID());
	bulletModels[bulletModel->getID()] = std::dynamic_pointer_cast<BulletModel>(bulletModel);
	if (emitOnChange) {
		onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::BULLET_MODEL, bulletModel->getID());
	}
}

void LevelPack::removeLevelFromPlayableLevelsList(int levelIndex) {
	levels.erase(levels.begin() + levelIndex);
	// Don't emit onChange here because the actual Level isn't modified from this operation
}

void LevelPack::deleteLevel(int id) {
	levelIDGen.deleteID(id);
	levelsMap.erase(id);

	for (int i = 0; i < levels.size();) {
		if (levels[i] == id) {
			removeLevelFromPlayableLevelsList(i);
		} else {
			i++;
		}
	}

	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::LEVEL, id);
}

void LevelPack::deleteAttack(int id) {
	attackIDGen.deleteID(id);
	attacks.erase(id);
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK, id);
}

void LevelPack::deleteAttackPattern(int id) {
	attackPatternIDGen.deleteID(id);
	attackPatterns.erase(id);
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK_PATTERN, id);
}

void LevelPack::deleteEnemy(int id) {
	enemyIDGen.deleteID (id);
	enemies.erase(id);
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY, id);
}

void LevelPack::deleteEnemyPhase(int id) {
	enemyPhaseIDGen.deleteID(id);
	enemyPhases.erase(id);
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY_PHASE, id);
}

void LevelPack::deleteBulletModel(int id) {
	bulletModelIDGen.deleteID(id);
	bulletModels.erase(id);
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::BULLET_MODEL, id);
}

std::vector<int> LevelPack::getEnemyUsers(int enemyID) {
	std::vector<int> results;
	for (auto it = levelsMap.begin(); it != levelsMap.end(); it++) {
		if (it->second->usesEnemy(enemyID)) {
			results.push_back(it->first);
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

std::vector<int> LevelPack::getAttackPatternEnemyUsers(int attackPatternID) {
	std::vector<int> results;
	for (auto it = enemyPhases.begin(); it != enemyPhases.end(); it++) {
		if (it->second->usesAttackPattern(attackPatternID)) {
			results.push_back(it->first);
		}
	}
	return results;
}

bool LevelPack::getAttackPatternIsUsedByPlayer(int attackPatternID) {
	return player->usesAttackPattern(attackPatternID);
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
	return enemies.find(id) != enemies.end();
}

bool LevelPack::hasEnemyPhase(int id) {
	return enemyPhases.find(id) != enemyPhases.end();
}

bool LevelPack::hasAttackPattern(int id) {
	return attackPatterns.find(id) != attackPatterns.end();
}

bool LevelPack::hasAttack(int id) {
	return attacks.find(id) != attacks.end();
}

bool LevelPack::hasBulletModel(int id) {
	return bulletModels.find(id) != bulletModels.end();
}

bool LevelPack::hasLevel(int levelIndex) {
	return levels.size() > levelIndex;
}

std::string LevelPack::getName() {
	return name;
}

std::shared_ptr<Level> LevelPack::getLevel(int levelIndex) const {
	return levelsMap.at(levels[levelIndex]);
}

std::shared_ptr<Level> LevelPack::getGameplayLevel(int levelIndex) const {
	auto level = levelsMap.at(levels[levelIndex])->clone();
	// Level is a top-level object so every expression it uses should be in terms of only its own
	// unredelegated, well-defined symbols
	auto derived = std::dynamic_pointer_cast<Level>(level);
	derived->compileExpressions({});
	return derived;
}

std::shared_ptr<EditorAttack> LevelPack::getAttack(int id) const {
	return attacks.at(id);
}

std::shared_ptr<EditorAttack> LevelPack::getGameplayAttack(int id, exprtk::symbol_table<float> symbolsDefiner) const {
	auto attack = attacks.at(id)->clone();
	auto derived = std::dynamic_pointer_cast<EditorAttack>(attack);
	derived->compileExpressions({ symbolsDefiner });
	return derived;
}

std::shared_ptr<EditorAttackPattern> LevelPack::getAttackPattern(int id) const {
	return attackPatterns.at(id);
}

std::shared_ptr<EditorAttackPattern> LevelPack::getGameplayAttackPattern(int id, exprtk::symbol_table<float> symbolsDefiner) const {
	auto attackPattern = attackPatterns.at(id)->clone();
	auto derived = std::dynamic_pointer_cast<EditorAttackPattern>(attackPattern);
	derived->compileExpressions({ symbolsDefiner });
	return derived;
}

std::shared_ptr<EditorEnemy> LevelPack::getEnemy(int id) const {
	return enemies.at(id);
}

std::shared_ptr<EditorEnemy> LevelPack::getGameplayEnemy(int id, exprtk::symbol_table<float> symbolsDefiner) const {
	auto enemy = enemies.at(id)->clone();
	auto derived = std::dynamic_pointer_cast<EditorEnemy>(enemy);
	derived->compileExpressions({ symbolsDefiner });
	return derived;
}

std::shared_ptr<EditorEnemyPhase> LevelPack::getEnemyPhase(int id) const {
	return enemyPhases.at(id);
}

std::shared_ptr<EditorEnemyPhase> LevelPack::getGameplayEnemyPhase(int id, exprtk::symbol_table<float> symbolsDefiner) const {
	auto phase = enemyPhases.at(id)->clone();
	auto derived = std::dynamic_pointer_cast<EditorEnemyPhase>(phase);
	derived->compileExpressions({ symbolsDefiner });
	return derived;
}

std::shared_ptr<BulletModel> LevelPack::getBulletModel(int id) const {
	return bulletModels.at(id);
}

std::shared_ptr<EditorPlayer> LevelPack::getPlayer() const {
	return player;
}

std::shared_ptr<EditorPlayer> LevelPack::getGameplayPlayer() const {
	auto clonedPlayer = player->clone();
	// EditorPlayer is a top-level object so every expression it uses should be in terms of only its own
	// unredelegated, well-defined symbols
	auto derived = std::dynamic_pointer_cast<EditorPlayer>(clonedPlayer);
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

bool LevelPack::getAttemptedLoad() const {
	return attemptedLoad;
}

bool LevelPack::getSuccessfulLoad() const {
	return successfulLoad;
}

std::shared_ptr<entt::SigH<void(LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE, int)>> LevelPack::getOnChange() {
	if (!onChange) {
		onChange = std::make_shared<entt::SigH<void(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE, int)>>();
	}
	return onChange;
}

bool LevelPack::hasBulletModel(int id) const {
	return bulletModels.find(id) != bulletModels.end();
}

void LevelPack::setPlayer(std::shared_ptr<EditorPlayer> player) {
	this->player = player;
	onChange->publish(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::PLAYER, player->getID());
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
	for (int levelID : levels) {
		if (levelsMap.find(levelID) == levelsMap.end()) {
			continue;
		}

		std::shared_ptr<Level> level = levelsMap.at(levelID);
		max = std::max(max, level->getHealthPack()->getActivationRadius());
		max = std::max(max, level->getPointsPack()->getActivationRadius());
		max = std::max(max, level->getPowerPack()->getActivationRadius());
		max = std::max(max, level->getBombItem()->getActivationRadius());
	}
	return max;
}

float LevelPack::searchLargestItemCollectionHitbox() const {
	float max = 0;
	for (int levelID : levels) {
		if (levelsMap.find(levelID) == levelsMap.end()) {
			continue;
		}
		
		std::shared_ptr<Level> level = levelsMap.at(levelID);
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

std::shared_ptr<sf::Music> LevelPack::playMusic(const MusicSettings & musicSettings) const {
	if (musicSettings.getFileName() == "") return nullptr;
	MusicSettings alteredPath = MusicSettings(musicSettings);
	alteredPath.setFileName("Level Packs/" + name + "/Music/" + alteredPath.getFileName());
	return audioPlayer.playMusic(alteredPath);
}

void LevelPack::playMusic(std::shared_ptr<sf::Music> music, const MusicSettings& musicSettings) const {
	if (!music || musicSettings.getFileName() == "") return;
	MusicSettings alteredPath = MusicSettings(musicSettings);
	alteredPath.setFileName("Level Packs/" + name + "/Music/" + alteredPath.getFileName());
	audioPlayer.playMusic(music, alteredPath);
}

std::set<int> LevelPack::getAllExistingLevelPackObjectFilesIDs(std::string folderPath, std::string levelPackObjectFilePrefix, std::string levelPackObjectFileExtension) {
	std::set<int> results;
	
	char fileName[MAX_PATH + 1];
	char extension[MAX_PATH + 1];

	for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
		_splitpath(entry.path().string().c_str(), NULL, NULL, fileName, extension);
		std::string fileNameString = std::string(fileName);
		if (strcmp(extension, levelPackObjectFileExtension.c_str()) == 0 && fileNameString._Starts_with(levelPackObjectFilePrefix) 
			&& fileNameString.length() > levelPackObjectFilePrefix.length()) {

			try {
				// Extract ID
				results.insert(std::stoi(fileNameString.substr(levelPackObjectFilePrefix.length())));
			} catch (std::invalid_argument e) {
				// Ignore
			} catch (std::out_of_range e) {
				// Ignore
			}
		}
	}

	return results;
}

void LevelPack::deleteLevelPackObjectFiles(std::string folderPath, std::string levelPackObjectFilePrefix, std::string levelPackObjectFileExtension, std::set<int> ids) {
	for (int id : ids) {
		if (std::remove(format("%s\\%s%d%s", folderPath.c_str(), levelPackObjectFilePrefix.c_str(),
			id, levelPackObjectFileExtension.c_str()).c_str()) != 0) {

			// TODO: log warning: unable to delete file
		}
	}
}

std::string LevelPack::LoadMetrics::formatForUser() {
	return format("%d/%d levels failed to load.\n\
%d/%d bullet models failed to load.\n\
%d/%d attacks failed to load.\n\
%d/%d attack patterns failed to load.\n\
%d/%d enemies failed to load.\n\
%d/%d enemy phases failed to load.\n", levelsFailed, levelsTotal,
bulletModelsFailed, bulletModelsTotal, attacksFailed, attacksTotal, attackPatternsFailed, attackPatternsTotal,
enemiesFailed, enemiesTotal, enemyPhasesFailed, enemyPhasesTotal);
}

bool LevelPack::LoadMetrics::containsFailedLoads() {
	return !playerSuccess || levelsFailed > 0 || enemiesFailed > 0
		|| enemyPhasesFailed > 0 || attackPatternsFailed > 0 || attacksFailed > 0 || bulletModelsFailed > 0;
}
