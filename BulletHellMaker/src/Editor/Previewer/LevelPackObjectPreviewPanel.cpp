#include <Editor/Previewer/LevelPackObjectPreviewPanel.h>

#include <Mutex.h>
#include <LevelPack/LevelPackObject.h>
#include <Editor/Windows/MainEditorWindow.h>

LevelPackObjectPreviewPanel::LevelPackObjectPreviewPanel(EditorWindow& parentWindow, std::string levelPackName)
	: SimpleEngineRenderer(*parentWindow.getWindow(), true, true), gui(parentWindow.getGui()) {

	loadLevelPack(levelPackName);
	levelPack->getOnChange()->sink().connect<LevelPackObjectPreviewPanel, &LevelPackObjectPreviewPanel::onLevelPackChange>(this);

	currentCursor = sf::CircleShape(-1);
	currentCursor.setOutlineColor(sf::Color::Green);
	currentCursor.setOutlineThickness(2.0f);
	currentCursor.setFillColor(sf::Color::Transparent);
	currentCursor.setOrigin(CURSOR_RADIUS, CURSOR_RADIUS);

	Animatable defaultEnemyAnimatable = Animatable("Enemy Placeholder", "Default", true, ROTATION_TYPE::LOCK_ROTATION);
	EntityAnimatableSet defaultEnemyAnimatableSet(defaultEnemyAnimatable, defaultEnemyAnimatable, defaultEnemyAnimatable);

	{
		emptyLevel = std::make_shared<Level>();
		emptyLevel->setBackgroundFileName("Default1.png");
		emptyLevel->compileExpressions({});
	}

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

	// Attack pattern preview reserves enemy ID -2, enemy phase ID -2, and attack pattern ID -2
	{
		levelForAttackPattern = std::make_shared<Level>();
		std::shared_ptr<EnemySpawnInfo> enemySpawnInfo = std::make_shared<EnemySpawnInfo>(-2, std::to_string(previewSourceX), std::to_string(previewSourceY), std::vector<std::pair<std::shared_ptr<Item>, std::string>>());
		std::vector<std::shared_ptr<EnemySpawnInfo>> enemySpawnInfoVector = { enemySpawnInfo };
		std::shared_ptr<SpawnEnemiesLevelEvent> spawnEnemyLevelEvent = std::make_shared<SpawnEnemiesLevelEvent>(enemySpawnInfoVector);
		levelForAttackPattern->insertEvent(0, std::make_shared<GlobalTimeBasedEnemySpawnCondition>("0"), spawnEnemyLevelEvent);
		levelForAttackPattern->setBackgroundFileName("Default1.png");
		levelForAttackPattern->compileExpressions({});

		std::shared_ptr<EditorEnemy> enemy = std::make_shared<EditorEnemy>(-2);
		enemy->setHitboxRadius("0");
		enemy->setHealth("99999");
		enemy->setDespawnTime("2000000000");
		enemy->setIsBoss(false);
		enemy->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>("0"), -2, defaultEnemyAnimatableSet, ExprSymbolTable());
		enemy->compileExpressions({});
		levelPack->updateEnemy(enemy);

		enemyPhaseForAttackPattern = std::make_shared<EditorEnemyPhase>(-2);
		enemyPhaseForAttackPattern->addAttackPatternID("0", -2, ExprSymbolTable());
		enemyPhaseForAttackPattern->setAttackPatternLoopDelay(std::to_string(attackLoopDelay));
		enemyPhaseForAttackPattern->setPhaseBeginAction(std::make_shared<NullEPA>());
		enemyPhaseForAttackPattern->setPhaseEndAction(std::make_shared<NullEPA>());
		enemyPhaseForAttackPattern->setPlayMusic(false);
		enemyPhaseForAttackPattern->compileExpressions({});
		levelPack->updateEnemyPhase(enemyPhaseForAttackPattern);
	}

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

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
	levelPack->getOnChange()->sink().disconnect<LevelPackObjectPreviewPanel, &LevelPackObjectPreviewPanel::onLevelPackChange>(this);
	gui->remove(symbolTableEditorWindow);
}

void LevelPackObjectPreviewPanel::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	// Viewport is set here because tgui::Gui's draw function changes it right before renderSystem is updated or something
	sf::View originalView = parentWindow.getView();
	parentWindow.setView(viewFromViewController);
	if (currentCursor.getRadius() > 0) {
		auto pos = sf::Mouse::getPosition(parentWindow);
		auto worldPos = parentWindow.mapPixelToCoords(pos);
		sf::CircleShape currentCursor = sf::CircleShape(this->currentCursor);
		// Show red cursor if setting player spawn in some invalid area
		if (settingPlayerSpawn && ((worldPos.x < 0 || worldPos.x > MAP_WIDTH) || (worldPos.y < 0 || worldPos.y > MAP_HEIGHT))) {
			currentCursor.setOutlineColor(sf::Color::Red);
		}

		currentCursor.setPosition(worldPos + sf::Vector2f(CURSOR_RADIUS/2.0f, CURSOR_RADIUS/2.0f));
		parentWindow.draw(currentCursor);
	}
	parentWindow.setView(originalView);
}

void LevelPackObjectPreviewPanel::previewNothing() {
	currentPreviewObjectType = PREVIEW_OBJECT::NONE;

	loadLevel(emptyLevel);
	unpause();
}

void LevelPackObjectPreviewPanel::previewAttack(const std::shared_ptr<EditorAttack> attack) {
	exprtk::symbol_table<float> convertedTestTable = testTable.toExprtkSymbolTable();

	// TODO: explicitly check every redefined symbol in attack is defined in testTable

	auto legal = attack->legal(*levelPack, *spriteLoader, { convertedTestTable });
	if (legal.first == LevelPackObject::LEGAL_STATUS::ILLEGAL) {
		legal.second.insert(legal.second.begin(), "Failed to load preview for attack ID " + std::to_string(attack->getID()) + ": \"" + attack->getName() + "\"");

		loadLevel(emptyLevel);
		unpause();

		onPreview.emit(this, legal.first, legal.second);
		return;
	}

	clearAllConnectedLevelPackObjects();
	addConnectedAttack(attack->getID());

	std::shared_ptr<EditorAttack> currentPreviewObject = std::dynamic_pointer_cast<EditorAttack>(attack->clone());
	currentPreviewObjectID = currentPreviewObject->getID();
	currentPreviewObjectType = PREVIEW_OBJECT::ATTACK;

	currentPreviewObject->setID(-1);
	levelPack->updateAttack(currentPreviewObject, false);
	currentPreviewObject->compileExpressions({ convertedTestTable });

	loadLevel(levelForAttack);
	unpause();

	legal.second.insert(legal.second.begin(), "Successfully loaded preview for attack ID " + std::to_string(attack->getID()) + ": \"" + attack->getName() + "\"");
	onPreview.emit(this, legal.first, legal.second);
}

void LevelPackObjectPreviewPanel::previewAttackPattern(const std::shared_ptr<EditorAttackPattern> attackPattern) {
	exprtk::symbol_table<float> convertedTestTable = testTable.toExprtkSymbolTable();

	// TODO: explicitly check every redefined symbol in attackPattern is defined in testTable

	auto legal = attackPattern->legal(*levelPack, *spriteLoader, { convertedTestTable });
	if (legal.first == LevelPackObject::LEGAL_STATUS::ILLEGAL) {
		legal.second.insert(legal.second.begin(), "Failed to load preview for attack pattern ID " + std::to_string(attackPattern->getID()) + ": \"" + attackPattern->getName() + "\"");

		loadLevel(emptyLevel);
		unpause();

		onPreview.emit(this, legal.first, legal.second);
		return;
	}

	clearAllConnectedLevelPackObjects();
	addConnectedAttackPattern(attackPattern->getID());

	std::shared_ptr<EditorAttackPattern> currentPreviewObject = std::dynamic_pointer_cast<EditorAttackPattern>(attackPattern->clone());
	currentPreviewObjectID = currentPreviewObject->getID();
	currentPreviewObjectType = PREVIEW_OBJECT::ATTACK_PATTERN;

	currentPreviewObject->setID(-2);
	levelPack->updateAttackPattern(currentPreviewObject, false);
	currentPreviewObject->compileExpressions({ convertedTestTable });

	loadLevel(levelForAttackPattern);
	unpause();

	legal.second.insert(legal.second.begin(), "Successfully loaded preview for attack pattern ID " + std::to_string(attackPattern->getID()) + ": \"" + attackPattern->getName() + "\"");
	onPreview.emit(this, legal.first, legal.second);
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
		}
	} else if ((settingPlayerSpawn || settingSource) && event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Right) {
			if (settingPlayerSpawn) {
				setSettingPlayerSpawn(false);
			} else if (settingSource) {
				setSettingSource(false);
			}
			return true;
		} else if (event.mouseButton.button == sf::Mouse::Left) {
			sf::View originalView = parentWindow.getView();
			parentWindow.setView(viewFromViewController);
			sf::Vector2f mouseWorldPos = parentWindow.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
			// Convert to coordinate system the game uses
			mouseWorldPos.y = -mouseWorldPos.y + MAP_HEIGHT;
			parentWindow.setView(originalView);

			if (settingPlayerSpawn) {
				if (!(mouseWorldPos.x < 0 || mouseWorldPos.x > MAP_WIDTH || mouseWorldPos.y < 0 || mouseWorldPos.y > MAP_HEIGHT)) {
					setPlayerSpawn(mouseWorldPos.x, mouseWorldPos.y);
					setSettingPlayerSpawn(false);
				}
			} else if (settingSource) {
				setPreviewSource(mouseWorldPos.x, mouseWorldPos.y);
				setSettingSource(false);
			}
			return true;
		}
	}
	return false;
}

void LevelPackObjectPreviewPanel::setSettingPlayerSpawn(bool settingPlayerSpawn) {
	this->settingPlayerSpawn = settingPlayerSpawn;

	if (settingPlayerSpawn) {
		settingSource = false;
		currentCursor.setRadius(CURSOR_RADIUS);
	} else {
		currentCursor.setRadius(-1);
	}
}

void LevelPackObjectPreviewPanel::setSettingSource(bool settingSource) {
	this->settingSource = settingSource;

	if (settingSource) {
		settingPlayerSpawn = false;
		currentCursor.setRadius(CURSOR_RADIUS);
	} else {
		currentCursor.setRadius(-1);
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

	// For attack
	attackPatternForAttack->removeAction(0);
	attackPatternForAttack->insertAction(0, std::make_shared<StayStillAtLastPositionEMPA>(attackLoopDelay));
	enemyPhaseForAttack->setAttackPatternLoopDelay(std::to_string(attackLoopDelay));
	enemyPhaseForAttack->compileExpressions({});

	// For attack pattern
	enemyPhaseForAttackPattern->setAttackPatternLoopDelay(std::to_string(attackLoopDelay));

	resetPreview();
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

std::shared_ptr<LevelPack> LevelPackObjectPreviewPanel::getLevelPack() {
	return levelPack;
}

tgui::Signal& LevelPackObjectPreviewPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onPreview.getName())) {
		return onPreview;
	}
	return SimpleEngineRenderer::getSignal(signalName);
}

void LevelPackObjectPreviewPanel::resetPreview() {
	// TODO
	if (currentPreviewObjectType == PREVIEW_OBJECT::NONE) {
		previewNothing();
	} else if (currentPreviewObjectType == PREVIEW_OBJECT::ATTACK) {
		previewAttack(levelPack->getAttack(currentPreviewObjectID));
	} else if (currentPreviewObjectType == PREVIEW_OBJECT::ATTACK_PATTERN) {
		previewAttackPattern(levelPack->getAttackPattern(currentPreviewObjectID));
	}
}

void LevelPackObjectPreviewPanel::clearAllConnectedLevelPackObjects() {
	bulletModelsConnectedToCurrentPreview.clear();
	attacksConnectedToCurrentPreview.clear();
	attackPatternsConnectedToCurrentPreview.clear();
	enemiesConnectedToCurrentPreview.clear();
	enemyPhasesConnectedToCurrentPreview.clear();
}

void LevelPackObjectPreviewPanel::addConnectedBulletModel(int id) {
	bulletModelsConnectedToCurrentPreview.insert(id);
}

void LevelPackObjectPreviewPanel::addConnectedAttack(int id) {
	attacksConnectedToCurrentPreview.insert(id);

	const std::map<int, int>* connectedBulletModels = levelPack->getAttack(id)->getBulletModelIDsCount();
	for (auto it = connectedBulletModels->begin(); it != connectedBulletModels->end(); it++) {
		addConnectedBulletModel(it->first);
	}
}

void LevelPackObjectPreviewPanel::addConnectedAttackPattern(int id) {
	attackPatternsConnectedToCurrentPreview.insert(id);

	const std::map<int, int>* connectedAttacks = levelPack->getAttackPattern(id)->getAttackIDsCount();
	for (auto it = connectedAttacks->begin(); it != connectedAttacks->end(); it++) {
		addConnectedAttack(it->first);
	}
}

void LevelPackObjectPreviewPanel::addConnectedEnemyPhase(int id) {
	enemyPhasesConnectedToCurrentPreview.insert(id);

	const std::map<int, int>* connectedAttackPatterns = levelPack->getEnemyPhase(id)->getAttackPatternsIDCount();
	for (auto it = connectedAttackPatterns->begin(); it != connectedAttackPatterns->end(); it++) {
		addConnectedAttackPattern(it->first);
	}
}

void LevelPackObjectPreviewPanel::addConnectedEnemy(int id) {
	attackPatternsConnectedToCurrentPreview.insert(id);

	const std::map<int, int>* connectedEnemyPhases = levelPack->getEnemy(id)->getEnemyPhaseIDsCount();
	for (auto it = connectedEnemyPhases->begin(); it != connectedEnemyPhases->end(); it++) {
		addConnectedEnemyPhase(it->first);
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

void LevelPackObjectPreviewPanel::onLevelPackChange(LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE type, int id) {
	if ((type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::BULLET_MODEL && bulletModelsConnectedToCurrentPreview.find(id) != bulletModelsConnectedToCurrentPreview.end())
		|| (type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK && attacksConnectedToCurrentPreview.find(id) != attacksConnectedToCurrentPreview.end())
		|| (type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK_PATTERN && attackPatternsConnectedToCurrentPreview.find(id) != attackPatternsConnectedToCurrentPreview.end())
		|| (type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY_PHASE && enemyPhasesConnectedToCurrentPreview.find(id) != enemyPhasesConnectedToCurrentPreview.end())
		|| (type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY && enemiesConnectedToCurrentPreview.find(id) != enemiesConnectedToCurrentPreview.end())
		|| type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::LEVEL
		|| type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::PLAYER
		|| currentPreviewObjectType == PREVIEW_OBJECT::NONE) {
		resetPreview();
	}
}
