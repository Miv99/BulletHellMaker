#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <TGUI/TGUI.hpp>
#include <memory>
#include <vector>
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "CollisionSystem.h"
#include "EnemySystem.h"
#include "DespawnSystem.h"
#include "SpriteAnimationSystem.h"
#include "EntityCreationQueue.h"
#include "ShadowTrailSystem.h"
#include "PlayerSystem.h"
#include "AudioPlayer.h"
#include "CollectibleSystem.h"

class LevelPack;
class LevelManagerTag;

class GameInstance {
public:
	GameInstance(std::string levelPackName);

	/*
	Starts the game instance with the loaded level.

	This function will block the current thread until the game instance is closed.
	*/
	void start();
	/*
	Closes the game instance.
	*/
	void close();

	/*
	Loads a level.
	*/
	void loadLevel(int levelIndex);

	/*
	Ends a level.
	*/
	void endLevel();

	void handleEvent(sf::Event event);
	void pause();
	void resume();

private:
	void updateWindowView(int windowWidth, int windowHeight);

	void physicsUpdate(float deltaTime);
	void render(float deltaTime);

	bool gameInstanceCloseQueued = false;

	std::unique_ptr<LevelPack> levelPack;
	std::unique_ptr<SpriteLoader> spriteLoader;
	std::unique_ptr<EntityCreationQueue> queue;

	entt::DefaultRegistry registry;
	std::unique_ptr<sf::RenderWindow> window;

	std::unique_ptr<MovementSystem> movementSystem;
	std::unique_ptr<RenderSystem> renderSystem;
	std::unique_ptr<CollisionSystem> collisionSystem;
	std::unique_ptr<DespawnSystem> despawnSystem;
	std::unique_ptr<EnemySystem> enemySystem;
	std::unique_ptr<SpriteAnimationSystem> spriteAnimationSystem;
	std::unique_ptr<ShadowTrailSystem> shadowTrailSystem;
	std::unique_ptr<PlayerSystem> playerSystem;
	std::unique_ptr<CollectibleSystem> collectibleSystem;
	std::unique_ptr<AudioPlayer> audioPlayer;

	bool paused;

	// Total amount of points earned so far across all past levels.
	// Does not include points from the current level.
	int points = 0;

	bool smoothPlayerHPBar;

	float guiRegionWidth;
	float guiRegionHeight;
	// screen x-coordinate of the leftmost part of the play area
	// playAreaYLow and playAreaYHigh are unnecessary since GUI is always aligned
	// vertically with the play area, so playAreaY____ == guiRegionY____
	float playAreaX;
	float playAreaWidth;
	float playAreaHeight;
	// The screen x-coordinate of the leftmost section of the GUI
	float guiRegionX;
	// The upper/lower y-axis bounds of the GUI region
	float guiRegionYLow, guiRegionYHigh;
	const float guiPaddingX = 20;
	const float guiPaddingY = 10;

	float windowHeight;
	std::shared_ptr<tgui::Gui> gui;

	// levelNameLabel is also used to locate the right border of the play area (same as left border of the gui region)
	// It is always guiPaddingX to the right of the right border of the play area
	std::shared_ptr<tgui::Label> levelNameLabel;
	std::shared_ptr<tgui::Label> scoreLabel;
	std::shared_ptr<tgui::Label> powerLabel;

	// For smooth player HP bar
	std::shared_ptr<tgui::ProgressBar> playerHPProgressBar;

	// For discrete player HP bar
	// Player HP picture is a square
	float playerHPPictureSize;
	const float playerHPPictureSizeMax = 30;
	// Player HP picture objects; one for each health the player can have, up to player's max health amount
	std::vector<std::shared_ptr<tgui::Picture>> playerHPPictures;
	// Current number of player HP pictures in the grid
	int hpPicturesInGrid = 0;
	// The grid of player HP pictures
	std::shared_ptr<tgui::Grid> playerHPPictureGrid;
	// Padding inbetween each picture
	const float playerHPGridPadding = 1.2f;
	// Label for player HP
	std::shared_ptr<tgui::Label> playerHPLabel;
	// Label for discrete player HP when >= playerHPPictureDisplayMax
	std::shared_ptr<tgui::Label> playerDiscreteHPCountLabel;
	// Maximum number of pictures before playerDiscreteHPCountLabel starts saying "x10" or something
	int playerHPPictureDisplayMax;

	// Same as discrete player HP bar, but for bombs
	float bombPictureSize;
	const float bombPictureSizeMax = playerHPPictureSizeMax;
	std::vector<std::shared_ptr<tgui::Picture>> bombPictures;
	int bombPicturesInGrid = 0;
	std::shared_ptr<tgui::Grid> bombPictureGrid;
	const float bombGridPadding = playerHPGridPadding;
	std::shared_ptr<tgui::Label> bombLabel;
	// Label for when bombs >= bombPictureDisplayMax
	std::shared_ptr<tgui::Label> bombCountLabel;
	int bombPictureDisplayMax;

	// Boss stuff
	// Displays boss name
	std::shared_ptr<tgui::Label> bossLabel;
	// Time until next phase
	std::shared_ptr<tgui::Label> bossPhaseTimeLeft;
	// Health bar for current phase
	std::shared_ptr<tgui::ProgressBar> bossPhaseHealthBar;
	uint32_t currentBoss;
	// Time after last phase change for next boss phase to start
	float bossNextPhaseStartTime;
	const float bossPhaseHealthBarHeight = 12;


	/*
	newHP - the player's new health
	*/
	void onPlayerHPChange(int newHP, int maxHP);
	/*
	levelPoints - points earned so far in the current level
	*/
	void onPointsChange(int levelPoints);
	/*
	info - a tuple of player's current power tier index, player's total power tier count, and player's current power, in that order
	*/
	void onPlayerPowerLevelChange(int powerLevelIndex, int powerLevelMaxTierCount, int powerLevel);
	/*
	bombs - the player's new amount of bombs
	*/
	void onPlayerBombCountChange(int newBombCount);
	void onEnemySpawn(uint32_t enemy);
	/*
	previousPhaseStartCondition - nullptr if the new phase is the first phase; otherwise, this is the start condition of the phase that just ended
	nextPhaseStartCondition - nullptr if the new phase is the last phase
	*/
	void onBossPhaseChange(uint32_t boss, std::shared_ptr<EditorEnemyPhase> newPhase, std::shared_ptr<EnemyPhaseStartCondition> previousPhaseStartCondition, std::shared_ptr<EnemyPhaseStartCondition> nextPhaseStartCondition);
	// Death is the same thing as despawn
	void onBossDespawn(uint32_t boss);

	void createPlayer(EditorPlayer params);
};