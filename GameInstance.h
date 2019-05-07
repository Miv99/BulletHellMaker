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
	GameInstance(sf::RenderWindow& window, std::string levelPackName);
	void physicsUpdate(float deltaTime);
	void render(float deltaTime);

	/*
	Starts a level.
	*/
	void startLevel(int levelIndex);

	/*
	Ends a level.
	*/
	void endLevel();

	void handleEvent(sf::Event event);
	void pause();
	void resume();

private:
	std::unique_ptr<LevelPack> levelPack;
	std::unique_ptr<SpriteLoader> spriteLoader;
	std::unique_ptr<EntityCreationQueue> queue;

	entt::DefaultRegistry registry;
	sf::RenderWindow& window;

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
	float guiRegionX;
	float guiPaddingX;
	float guiPaddingY;

	std::shared_ptr<tgui::Gui> gui;
	std::shared_ptr<tgui::Label> levelNameLabel;
	std::shared_ptr<tgui::Label> scoreLabel;
	std::shared_ptr<tgui::Label> powerLabel;

	// For smooth player HP bar
	std::shared_ptr<tgui::ProgressBar> playerHPProgressBar;

	// For discrete player HP bar
	// Player HP picture is a square
	float playerHPPictureSize;
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

	void createPlayer(EditorPlayer params);
};