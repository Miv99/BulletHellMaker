#pragma once
#include <mutex>

#include <TGUI/TGUI.hpp>

#include <Editor/EventCapturable.h>
#include <LevelPack/Level.h>
#include <Game/Systems/Systems.h>
#include <Editor/ViewController.h>

/*
A panel that can load Levels and play them while rendering it either normally or in debug mode.
A LevelPack must be loaded before anything can be done.

Paused upon construction.

start() or startAsync() must be called to begin the main loop, then unpause() to unpause the systems.

Note that SFML requires the underlying RenderWindow to be created in the same thread as the
render calls, so something like LevelPackObjectPreviewWindow must be used specifically for this widget.
*/
class SimpleEngineRenderer : public tgui::Panel, public EventCapturable {
public:
	SimpleEngineRenderer(sf::RenderWindow& parentWindow, bool userControlleView = true, bool useDebugRenderSystem = true);
	static std::shared_ptr<SimpleEngineRenderer> create(sf::RenderWindow& parentWindow) {
		return std::make_shared<SimpleEngineRenderer>(parentWindow);
	}

	bool update(sf::Time elapsedTime) override;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	virtual bool handleEvent(sf::Event event) override;

	virtual void loadLevelPack(std::string name);

	void loadLevel(int levelIndex);
	void loadLevel(std::shared_ptr<Level> level);

	void pause();
	void unpause();

	void resetCamera();

	void physicsUpdate(float deltaTime);
	void renderUpdate(float deltaTime);

	void setUseDebugRenderSystem(bool useDebugRenderSystem);
	/*
	timeMultiplier - in range [0, 1]
	*/
	void setTimeMultiplier(float timeMultiplier);
	void setPlayerSpawn(float x, float y);
	void setInvinciblePlayer(bool invinciblePlayer);

	bool getUseDebugRenderSystem() const;
	float getTimeMultiplier() const;

protected:
	sf::RenderWindow& parentWindow;

	std::shared_ptr<LevelPack> levelPack;
	mutable entt::DefaultRegistry registry;
	std::shared_ptr<SpriteLoader> spriteLoader;

	std::unique_ptr<MovementSystem> movementSystem;
	std::unique_ptr<DebugRenderSystem> debugRenderSystem;
	std::unique_ptr<RenderSystem> renderSystem;
	std::unique_ptr<CollisionSystem> collisionSystem;
	std::unique_ptr<DespawnSystem> despawnSystem;
	std::unique_ptr<EnemySystem> enemySystem;
	std::unique_ptr<SpriteAnimationSystem> spriteAnimationSystem;
	std::unique_ptr<ShadowTrailSystem> shadowTrailSystem;
	std::unique_ptr<PlayerSystem> playerSystem;
	std::unique_ptr<CollectibleSystem> collectibleSystem;
	std::unique_ptr<AudioPlayer> audioPlayer;

	bool useDebugRenderSystem;
	sf::View viewFromViewController;

	virtual std::shared_ptr<EditorPlayer> getPlayer();

private:
	std::mutex registryMutex;

	bool paused;

	std::unique_ptr<EntityCreationQueue> queue;

	sf::FloatRect viewportFloatRect, viewFloatRect;

	bool userControlledView;
	std::unique_ptr<ViewController> viewController;

	float timeMultiplier = 1.0f;

	// Spawn location of player for previews
	float playerSpawnX = PLAYER_SPAWN_X;
	float playerSpawnY = PLAYER_SPAWN_Y;

	bool invinciblePlayer = false;

	void updateWindowView();
};