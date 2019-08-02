#pragma once
#include "MovablePoint.h"
#include "UndoStack.h"
#include "EntityCreationQueue.h"
#include "MovementSystem.h"
#include "DespawnSystem.h"
#include "EnemySystem.h"
#include "SpriteAnimationSystem.h"
#include "ShadowTrailSystem.h"
#include "PlayerSystem.h"
#include "CollectibleSystem.h"
#include "EditorUtilities.h"
#include "Constants.h"
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <entt/entt.hpp>

class EditorWindow {
public:
	/*
	renderInterval - time between each render call. If the gui has a ListBox, renderInterval should be some relatively large number (~0.1) because tgui gets
		messed up with multithreading.
	*/
	EditorWindow(std::shared_ptr<std::mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);

	/*
	Starts the main loop.
	This function blocks the current thread.
	*/
	void start();
	/*
	Closes the window.
	*/
	void close();
	/*
	Pauses the EditorWindow and hides it. The EditorWindow object data is maintained.
	*/
	void hide();

	void addPopupWidget(std::shared_ptr<tgui::Container> popupContainer, std::shared_ptr<tgui::Widget> popup);
	void closePopupWidget();

	/*
	Called every time window size changes.
	*/
	virtual void updateWindowView(int windowWidth, int windowHeight);

	std::shared_ptr<tgui::Gui> getGui() { return gui; }
	inline int getWindowWidth() { return windowWidth; }
	inline int getWindowHeight() { return windowHeight; }
	std::shared_ptr<entt::SigH<void(float)>> getRenderSignal();
	std::shared_ptr<entt::SigH<void(int, int)>> getResizeSignal();
	inline std::shared_ptr<sf::RenderWindow> getWindow() { return window; }

protected:
	std::shared_ptr<sf::RenderWindow> window;

	virtual void physicsUpdate(float deltaTime);
	virtual void render(float deltaTime);
	virtual void handleEvent(sf::Event event);

	/*
	Called when the RenderWindow window is initialized.
	*/
	inline virtual void onRenderWindowInitialization() {}

private:
	std::string windowTitle;
	int windowWidth, windowHeight;

	std::shared_ptr<tgui::Gui> gui;
	// The popup widget, if one exists.
	// Only one popup widget can exist at any time. If a new widget pops up,
	// the old one is removed from the Gui. When the user clicks outside the popup
	// widget, it closes.
	std::shared_ptr<tgui::Widget> popup;
	// The container that contains the popup
	std::shared_ptr<tgui::Container> popupContainer;

	bool letterboxingEnabled;
	bool scaleWidgetsOnResize;

	// Mutex used to make sure multiple tgui widgets aren't being instantiated at the same time in different threads.
	// tgui::Gui draw() calls also can't be done at the same time.
	// Apparently tgui gets super messed up with multithreading.
	std::shared_ptr<std::mutex> tguiMutex;

	float renderInterval;
	// Signal that's emitted every time a render call is made
	// function accepts 1 argument: the time since the last render
	std::shared_ptr<entt::SigH<void(float)>> renderSignal;
	// Signal that's emitted every time the window resizes
	std::shared_ptr<entt::SigH<void(int, int)>> resizeSignal;
};

/*
An EditorWindow that has the capability of redoing and undoing commands in the UndoStack.
Undo is done with control+z and redo with control+y.
Commands must be added to the UndoStack by the user.
*/
class UndoableEditorWindow : public EditorWindow {
public:
	inline UndoableEditorWindow(std::shared_ptr<std::mutex> tguiMutex, std::string windowTitle, int width, int height, UndoStack& undoStack, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL) :
		EditorWindow(tguiMutex, windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval), undoStack(undoStack) {
	}

protected:
	virtual void handleEvent(sf::Event event);

private:
	UndoStack& undoStack;
};

class GameplayTestWindow : public UndoableEditorWindow {
public:
	GameplayTestWindow(std::shared_ptr<LevelPack> levelPack, std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<std::mutex> tguiMutex,
		std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false,
		float renderInterval = RENDER_INTERVAL);

protected:
	void handleEvent(sf::Event event) override;
	void physicsUpdate(float deltaTime) override;
	void render(float deltaTime) override;

	void updateWindowView(int width, int height) override;
	void onRenderWindowInitialization() override;

private:
	const float GUI_PADDING_X = 20;
	const float GUI_PADDING_Y = 10;
	const int UNDO_STACK_MAX = 100;
	const float LEFT_PANEL_WIDTH = 0.25f;
	const float RIGHT_PANEL_WIDTH = 0.2f;
	const float CAMERA_SPEED = 100; // World units per second

	// This should be modified only by setCameraZoom()
	float cameraZoom = 1.0f;
	bool draggingCamera = false;
	// Screen coordinates of the mouse in the last MouseMove event while
	// draggingCamera was true
	int previousCameraDragCoordsX, previousCameraDragCoordsY;

	bool paused = false;

	UndoStack undoStack;

	std::shared_ptr<LevelPack> levelPack;
	std::shared_ptr<SpriteLoader> spriteLoader;
	std::shared_ptr<EntityCreationQueue> queue;

	entt::DefaultRegistry registry;

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


	std::shared_ptr<tgui::ScrollablePanel> leftPanel;
	std::shared_ptr<tgui::Label> entityPlaceholdersListLabel;
	std::shared_ptr<tgui::ListBox> entityPlaceholdersList;
	std::shared_ptr<tgui::Button> newEnemyPlaceholder;
	std::shared_ptr<tgui::Button> deleteEnemyPlaceholder;

	std::shared_ptr<tgui::ScrollablePanel> rightPanel;
	std::shared_ptr<tgui::Label> enemyPlaceholderXLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> enemyPlaceholderX;
	std::shared_ptr<tgui::Label> enemyPlaceholderYLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> enemyPlaceholderY;
	std::shared_ptr<tgui::Button> enemyPlaceholderManualSet;
	std::shared_ptr<tgui::Button> setEnemyPlaceholderTestMode; // TODO: popup on click: attack, attack pattern, enemy phase, enemy
	std::shared_ptr<tgui::Label> testModeIDLabel;
	std::shared_ptr<tgui::ListBox> testModeID; // contains all Editor____ objects in the LevelPack (______ part depends on currently selected placeholder's test mode)

	/*
	Moves the camera by some amount of world coordinates.
	*/
	void moveCamera(float xOffset, float yOffset);
	/*
	Centers the camera on a world position.
	*/
	void lookAt(float x, float y);
	void setCameraZoom(float zoom);
	/*
	screenX/Y - the screen coordinates of the click
	*/
	void onGameplayAreaMouseClick(float screenX, float screenY);
};