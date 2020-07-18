#include "LevelPackObjectPreviewPanel.h"
#include "LevelPackObject.h"
#include "EditorWindow.h"

LevelPackObjectPreviewPanel::LevelPackObjectPreviewPanel(EditorWindow& parentWindow, std::string levelPackName)
	: SimpleEngineRenderer(*parentWindow.getWindow(), true, true), gui(parentWindow.getGui()) {
	loadLevelPack(levelPackName);
	levelPack->getOnChange()->sink().connect<LevelPackObjectPreviewPanel, &LevelPackObjectPreviewPanel::resetPreview>(this);

	Animatable defaultEnemyAnimatable = Animatable("Enemy Placeholder", "Default", true, ROTATION_TYPE::LOCK_ROTATION);
	EntityAnimatableSet defaultEnemyAnimatableSet(defaultEnemyAnimatable, defaultEnemyAnimatable, defaultEnemyAnimatable);

	// Player reserves attack pattern ID -2
	{
		defaultPlayer = std::make_shared<EditorPlayer>();
		defaultPlayer->setInitialHealth("2000000000");
		defaultPlayer->setMaxHealth("2000000000");
		defaultPlayer->setSpeed("400");
		defaultPlayer->setFocusedSpeed("200");
		defaultPlayer->setHitboxRadius("1");
		defaultPlayer->setInvulnerabilityTime("2");
		defaultPlayer->setMaxBombs("0");
		defaultPlayer->setInitialBombs("0");

		Animatable defaultPlayerAnimatable = Animatable("Player Placeholder", "Default", true, ROTATION_TYPE::LOCK_ROTATION);
		EntityAnimatableSet defaultPlayerAnimatableSet(defaultPlayerAnimatable, defaultPlayerAnimatable, defaultPlayerAnimatable);
		std::shared_ptr<PlayerPowerTier> powerTier = std::make_shared<PlayerPowerTier>(defaultPlayerAnimatableSet, -2, "2", -2, "0.5", -2, "5", "2000000000");
		defaultPlayer->insertPowerTier(0, powerTier);

		std::shared_ptr<EditorAttackPattern> attackPattern = std::make_shared<EditorAttackPattern>(-2);
		attackPattern->setShadowTrailInterval("0");
		attackPattern->setShadowTrailLifespan("0");
		//attackPattern->addAttack("0", -1, ExprSymbolTable());
		attackPattern->compileExpressions({});
		levelPack->updateAttackPattern(attackPattern);

		defaultPlayer->compileExpressions({});
	}

	// Attack preview reserves enemy ID -1, enemy phase ID -1, attack pattern ID -1, and attack ID -1
	{
		levelForAttack = std::make_shared<Level>();
		std::shared_ptr<EnemySpawnInfo> enemySpawnInfo = std::make_shared<EnemySpawnInfo>(-1, std::to_string(previewSourceX), std::to_string(previewSourceY), std::vector<std::pair<std::shared_ptr<Item>, std::string>>());
		std::vector<std::shared_ptr<EnemySpawnInfo>> enemySpawnInfoVector = { enemySpawnInfo };
		std::shared_ptr<SpawnEnemiesLevelEvent> spawnEnemyLevelEvent = std::make_shared<SpawnEnemiesLevelEvent>(enemySpawnInfoVector);
		levelForAttack->insertEvent(0, std::make_shared<GlobalTimeBasedEnemySpawnCondition>("0"), spawnEnemyLevelEvent);
		levelForAttack->setBackgroundFileName("Default1.png");
		levelForAttack->compileExpressions({});

		std::shared_ptr<EditorEnemy> enemy = std::make_shared<EditorEnemy>(-1);
		enemy->setHitboxRadius("0");
		enemy->setHealth("99999");
		enemy->setDespawnTime("2000000000");
		enemy->setIsBoss(false);
		enemy->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>("0"), -1, defaultEnemyAnimatableSet, ExprSymbolTable());
		enemy->compileExpressions({});
		levelPack->updateEnemy(enemy);

		enemyPhaseForAttack = std::make_shared<EditorEnemyPhase>(-1);
		enemyPhaseForAttack->addAttackPatternID("0", -1, ExprSymbolTable());
		enemyPhaseForAttack->setAttackPatternLoopDelay(std::to_string(attackLoopDelay));
		enemyPhaseForAttack->setPhaseBeginAction(std::make_shared<NullEPA>());
		enemyPhaseForAttack->setPhaseEndAction(std::make_shared<NullEPA>());
		enemyPhaseForAttack->setPlayMusic(false);
		enemyPhaseForAttack->compileExpressions({});
		levelPack->updateEnemyPhase(enemyPhaseForAttack);

		attackPatternForAttack = std::make_shared<EditorAttackPattern>(-1);
		attackPatternForAttack->setShadowTrailInterval("0");
		attackPatternForAttack->setShadowTrailLifespan("0");
		attackPatternForAttack->insertAction(0, std::make_shared<StayStillAtLastPositionEMPA>(attackLoopDelay));
		attackPatternForAttack->addAttack("0", -1, ExprSymbolTable());
		attackPatternForAttack->compileExpressions({});
		levelPack->updateAttackPattern(attackPatternForAttack);
	}

	symbolTableEditorWindow = tgui::ChildWindow::create();
	symbolTableEditor = ValueSymbolTableEditor::create(true, true);
	symbolTableEditorWindow->setKeepInParent(false);
	symbolTableEditorWindow->add(symbolTableEditor);
	symbolTableEditorWindow->setSize("50%", "50%");
	symbolTableEditorWindow->setTitle("Test Variables");

	symbolTableEditor->connect("ValueChanged", [this](ValueSymbolTable table) {
		this->testTable = table;
		resetPreview();
	});

	symbolTableEditor->setSymbolTablesHierarchy({ testTable });
}

LevelPackObjectPreviewPanel::~LevelPackObjectPreviewPanel() {
	levelPack->getOnChange()->sink().disconnect(this);
	gui->remove(symbolTableEditorWindow);
}

void LevelPackObjectPreviewPanel::previewAttack(const std::shared_ptr<EditorAttack> attack) {
	exprtk::symbol_table<float> convertedTestTable = testTable.toExprtkSymbolTable();

	// TODO: explicitly check every redefined symbol in attack is defined in testTable

	auto legal = attack->legal(*levelPack, *spriteLoader, { convertedTestTable });
	if (legal.first == LevelPackObject::LEGAL_STATUS::ILLEGAL) {
		// TODO: handle illegal attack
		return;
	}

	std::shared_ptr<EditorAttack> currentPreviewObject = std::dynamic_pointer_cast<EditorAttack>(attack->clone());
	currentPreviewObjectID = currentPreviewObject->getID();
	currentPreviewObjectType = PREVIEW_OBJECT::ATTACK;

	currentPreviewObject->setID(-1);
	levelPack->updateAttack(currentPreviewObject, false);
	currentPreviewObject->compileExpressions({ convertedTestTable });

	loadLevel(levelForAttack);
	unpause();
}

bool LevelPackObjectPreviewPanel::handleEvent(sf::Event event) {
	if (symbolTableEditorWindow->isFocused()) {
		return symbolTableEditor->handleEvent(event);
	} else if (SimpleEngineRenderer::handleEvent(event)) {
		return true;
	} else if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::V) {
			gui->add(symbolTableEditorWindow);
			return true;
		} else if (event.key.code == sf::Keyboard::I) {
			// TODO: show info
			return true;
		} else if (event.key.code == sf::Keyboard::T) {
			setUseDebugRenderSystem(!useDebugRenderSystem);
			return true;
		}
	}
}

void LevelPackObjectPreviewPanel::setPreviewSource(float x, float y) {
	previewSourceX = x;
	previewSourceY = y;

	std::shared_ptr<EnemySpawnInfo> enemySpawnInfo = std::make_shared<EnemySpawnInfo>(-1, std::to_string(previewSourceX), std::to_string(previewSourceY), std::vector<std::pair<std::shared_ptr<Item>, std::string>>());
	std::vector<std::shared_ptr<EnemySpawnInfo>> enemySpawnInfoVector = { enemySpawnInfo };
	std::shared_ptr<SpawnEnemiesLevelEvent> spawnEnemyLevelEvent = std::make_shared<SpawnEnemiesLevelEvent>(enemySpawnInfoVector);
	levelForAttack->removeEvent(0);
	levelForAttack->insertEvent(0, std::make_shared<GlobalTimeBasedEnemySpawnCondition>("0"), spawnEnemyLevelEvent);
	levelForAttack->compileExpressions({});
}

void LevelPackObjectPreviewPanel::setAttackLoopDelay(float attackLoopDelay) {
	this->attackLoopDelay = attackLoopDelay;
	attackPatternForAttack->removeAction(0);
	attackPatternForAttack->insertAction(0, std::make_shared<StayStillAtLastPositionEMPA>(attackLoopDelay));
	enemyPhaseForAttack->setAttackPatternLoopDelay(std::to_string(attackLoopDelay));
	enemyPhaseForAttack->compileExpressions({});
}

void LevelPackObjectPreviewPanel::setAttackPatternLoopDelay(float attackPatternLoopDelay) {
	//TODO
	this->attackPatternLoopDelay = attackPatternLoopDelay;
}

float LevelPackObjectPreviewPanel::getPreviewSourceX() const {
	return previewSourceX;
}

float LevelPackObjectPreviewPanel::getPreviewSourceY() const {
	return previewSourceY;
}

float LevelPackObjectPreviewPanel::getAttackLoopDelay() const {
	return attackLoopDelay;
}

float LevelPackObjectPreviewPanel::getAttackPatternLoopDelay() const {
	return attackPatternLoopDelay;
}

std::shared_ptr<LevelPack> LevelPackObjectPreviewPanel::getLevelPack() {
	return levelPack;
}

void LevelPackObjectPreviewPanel::resetPreview() {
	// TODO
	if (currentPreviewObjectType == PREVIEW_OBJECT::ATTACK) {
		previewAttack(levelPack->getAttack(currentPreviewObjectID));
	}
}

std::shared_ptr<EditorPlayer> LevelPackObjectPreviewPanel::getPlayer() {
	auto levelPackPlayer = levelPack->getGameplayPlayer();
	auto legal = levelPackPlayer->legal(*levelPack, *spriteLoader, {});
	if (legal.first == LevelPackObject::LEGAL_STATUS::ILLEGAL) {
		// TODO: show errors to user
		return defaultPlayer;
	} else {
		return levelPackPlayer;
	}
}
