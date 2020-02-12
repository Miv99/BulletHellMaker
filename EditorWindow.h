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
#include "DebugRenderSystem.h"
#include "EditorUtilities.h"
#include "Constants.h"
#include "LevelPack.h"
#include "Attack.h"
#include "EditorMovablePoint.h"
#include "AudioPlayer.h"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <entt/entt.hpp>

/*
If the underlying RenderWindow is closed, one only needs to call start() or startAndHide() again to reopen the RenderWindow.
*/
class EditorWindow : public std::enable_shared_from_this<EditorWindow> {
public:
	/*
	tguiMutex - mutex for when creating TGUI objects
	renderInterval - time between each render call. If the gui has a ListBox, renderInterval should be some relatively large number (~0.1) because tgui gets
		messed up with multithreading.
	*/
	EditorWindow(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);

	/*
	Starts the main loop.
	This function blocks the current thread.
	*/
	void start();
	/*
	Starts the main loop and then hides the window.
	This function blocks the current thread.
	*/
	void startAndHide();
	/*
	Closes the window.
	*/
	void close();
	/*
	Pauses the EditorWindow and hides it. The EditorWindow object data is maintained.
	*/
	void hide();
	/*
	Shows the EditorWindow.
	*/
	void show();

	/*
	Disables all widgets and then prompts the user with a custom message to which the user can respond with either a yes or a no.
	A signal with one parameter is returned. The signal will be published only once, when the user responds. The bool parameter
	will be true if the user responds with yes, false if no. After the user response, all widgets' enabled/disabled statuses are
	set back to what they were before this function call.
	Each call to promptConfirmation() returns a new signal.
	*/
	std::shared_ptr<entt::SigH<void(bool)>> promptConfirmation(std::string message);

	/*
	Add a popup as a top-level widget in the Gui. If part of the popup 

	preferredX, preferredY - the preferred position of the popup; cannot be less than 0
	preferredWidth, preferredHeight - the preferred size of the popup; cannot be less than 0
	maxWidthFraction, maxHeightFraction - the maximum size of the popup as a multiplier of the window width/height
	*/
	void addPopupWidget(std::shared_ptr<tgui::Widget> popup, float preferredX, float preferredY, float preferredWidth, float preferredHeight, float maxWidthFraction = 1.0f, float maxHeightFraction = 1.0f);
	/*
	Add a popup as a child of the popupContainer.
	*/
	void addPopupWidget(std::shared_ptr<tgui::Container> popupContainer, std::shared_ptr<tgui::Widget> popup);
	void closePopupWidget();

	/*
	Returns the ID of the added vertex array.
	*/
	int addVertexArray(sf::VertexArray vertexArray);
	void removeVertexArray(int id);
	/*
	Sets the old VertexArray with the given id to be newVertexArray.
	If there was no VertexArray with the given id, nothing will happen.
	*/
	void modifyVertexArray(int id, sf::VertexArray newVertexArray);
	void removeAllVertexArrays();

	/*
	Called every time window size changes.
	*/
	virtual void updateWindowView(int windowWidth, int windowHeight);

	std::shared_ptr<tgui::Gui> getGui() { return gui; }
	inline int getWindowWidth() { return windowWidth; }
	inline int getWindowHeight() { return windowHeight; }
	std::shared_ptr<entt::SigH<void(float)>> getRenderSignal();
	std::shared_ptr<entt::SigH<void(int, int)>> getResizeSignal();
	std::shared_ptr<entt::SigH<void()>> getCloseSignal();
	inline std::shared_ptr<sf::RenderWindow> getWindow() { return window; }

protected:
	std::shared_ptr<sf::RenderWindow> window;
	std::shared_ptr<tgui::Gui> gui;

	// The last known position of the mouse
	sf::Vector2f mousePos = sf::Vector2f(0, 0);
	// The position of the mouse at the last mouse press
	sf::Vector2f lastMousePressPos = sf::Vector2f(0, 0);

	virtual void physicsUpdate(float deltaTime);
	virtual void render(float deltaTime);
	virtual void handleEvent(sf::Event event);

	/*
	Called when the RenderWindow window is initialized.
	*/
	inline virtual void onRenderWindowInitialization() {}

private:
	const float GUI_PADDING_X = 20;
	const float GUI_PADDING_Y = 10;

	int nextVertexArrayID = 0;
	std::map<int, sf::VertexArray> vertexArrays;

	std::string windowTitle;
	int windowWidth, windowHeight;

	// The popup widget, if one exists.
	// Only one popup widget can exist at any time. If a new widget pops up,
	// the old one is removed from the Gui. When the user clicks outside the popup
	// widget, it closes.
	std::shared_ptr<tgui::Widget> popup;
	// The container that contains the popup
	std::shared_ptr<tgui::Container> popupContainer;

	// The widgets for the confirmation popup in promptConfirmation()
	std::shared_ptr<tgui::Panel> confirmationPanel; // Not added to gui until promptConfirmation() is called; removed after user responds
	std::shared_ptr<tgui::Label> confirmationText;
	std::shared_ptr<tgui::Button> confirmationYes;
	std::shared_ptr<tgui::Button> confirmationNo;

	// A list of all Widgets in gui whose value of isEnabled() was true right before promptConfirmation() was called
	std::list<std::shared_ptr<tgui::Widget>> widgetsToBeEnabledAfterConfirmationPrompt;

	bool letterboxingEnabled;
	bool scaleWidgetsOnResize;

	// Mutex used to make sure multiple tgui widgets aren't being instantiated at the same time in different threads.
	// tgui::Gui draw() calls also can't be done at the same time.
	// Apparently tgui gets super messed up with multithreading.
	std::shared_ptr<std::recursive_mutex> tguiMutex;
	
	float renderInterval;
	// Signal that's emitted every time a render call is made
	// function accepts 1 argument: the time since the last render
	std::shared_ptr<entt::SigH<void(float)>> renderSignal;
	// Signal that's emitted every time the window resizes
	std::shared_ptr<entt::SigH<void(int, int)>> resizeSignal;
	// Signal that's emitted right before the window closes
	std::shared_ptr<entt::SigH<void()>> closeSignal;
	// Signal for confirmation popup. See promptConfirmation().
	std::shared_ptr<entt::SigH<void(bool)>> confirmationSignal;

	/*
	Call to stop the confirmation prompt started by promptConfirmation().
	*/
	void closeConfirmationPanel();
};

/*
An EditorWindow that has the capability of redoing and undoing commands in the UndoStack.
Undo is done with control+z and redo with control+y.
Commands must be added to the UndoStack by the user.
*/
class UndoableEditorWindow : public EditorWindow {
public:
	inline UndoableEditorWindow(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, UndoStack& undoStack, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL) :
		EditorWindow(tguiMutex, windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval), undoStack(undoStack) {
	}

protected:
	virtual void handleEvent(sf::Event event);

private:
	UndoStack& undoStack;
};

class GameplayTestWindow : public UndoableEditorWindow {
public:
	GameplayTestWindow(std::shared_ptr<LevelPack> levelPack, std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<std::recursive_mutex> tguiMutex,
		std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false,
		float renderInterval = RENDER_INTERVAL);

	/*
	Adds a placeholder for testing EMPs. The EMP being tested for a particular placeholder cannot be changed.
	See EMPTestEntityPlaceholder for explanations of parameters.
	*/
	void addEMPTestPlaceholder(std::shared_ptr<EditorMovablePoint> emp, bool empIsFromAttack, int sourceID);

	/*
	bezierEMPA - the MoveCustomBezierEMPA whose control points will be edited
	*/
	void beginEditingBezierControlPoints(MoveCustomBezierEMPA* bezierEMPA);
	/*
	saveChanges - whether changes to the control points should be saved by whoever called beginEditingBezierControlPoints()
	newControlPoints - the new control points; ignored if saveChanges is false
	*/
	void endEditingBezierControlPoints(bool saveChanges, std::vector<sf::Vector2f> newControlPoints);

	/*
	id - the desired ID of the new placeholder
	*/
	void insertBezierControlPointPlaceholder(int id, float x, float y);

	inline std::shared_ptr<entt::SigH<void(bool, std::vector<sf::Vector2f>)>> getOnBezierControlPointEditingEndSignal() { return onBezierControlPointEditingEnd; }

protected:
	void handleEvent(sf::Event event) override;
	void physicsUpdate(float deltaTime) override;
	void render(float deltaTime) override;

	void updateWindowView(int width, int height) override;
	void onRenderWindowInitialization() override;

private:
	class EntityPlaceholder {
	public:
		inline EntityPlaceholder(int id, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, LevelPack& levelPack) : id(id), registry(registry), 
			spriteLoader(spriteLoader), levelPack(levelPack) {}

		void moveTo(float x, float y);
		virtual void runTest(std::shared_ptr<std::recursive_mutex> registryMutex) = 0;
		void endTest(std::shared_ptr<std::recursive_mutex> registryMutex);
		bool wasClicked(int worldX, int worldY);
		// Should be called when the EntityPlaceholder is removed from the GameplayTestWindow
		void removePlaceholder(std::shared_ptr<std::recursive_mutex> registryMutex);
		virtual void spawnVisualEntity() = 0;
		// Called when this placeholder is selected
		void onSelection();
		// Called when this placeholder is deselected
		void onDeselection();

		virtual Animatable getVisualEntityAnimatable() = 0;
		virtual sf::VertexArray getMovementPath(float timeResolution, float playerX, float playerY) = 0;

		inline void setID(int id) { this->id = id; }
		inline int getID() { return id; }
		inline float getX() { return x; }
		inline float getY() { return y; }
		inline void setUseDefaultTestPlayer(bool useDefaultTestPlayer) { this->useDefaultTestPlayer = useDefaultTestPlayer; }

	protected:
		const float CLICK_HITBOX_SIZE = 15;

		entt::DefaultRegistry& registry;
		SpriteLoader& spriteLoader;
		LevelPack& levelPack;

		// ID of the placeholder
		int id;
		// The entity used to show the placeholder's location
		uint32_t visualEntity;
		bool visualEntityExists = false;
		// The entity spawned as a result of the test
		uint32_t testEntity;
		// Location of the placeholder
		float x, y;
		// Whether to use default test player when running test
		bool useDefaultTestPlayer = false;
	};
	class PlayerEntityPlaceholder : public EntityPlaceholder {
	public:
		inline PlayerEntityPlaceholder(int id, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, LevelPack& levelPack) : 
			EntityPlaceholder(id, registry, spriteLoader, levelPack) {}
	
		void runTest(std::shared_ptr<std::recursive_mutex> registryMutex) override;
		void spawnVisualEntity() override;
		Animatable getVisualEntityAnimatable() override;
		sf::VertexArray getMovementPath(float timeResolution, float playerX, float playerY) override;
	};
	class EnemyEntityPlaceholder : public EntityPlaceholder {
	public:
		enum TEST_MODE { ATTACK, ATTACK_PATTERN, PHASE, ENEMY };

		inline EnemyEntityPlaceholder(int id, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, LevelPack& levelPack, EntityCreationQueue& queue) : 
			EntityPlaceholder(id, registry, spriteLoader, levelPack), queue(queue) {}

		void runTest(std::shared_ptr<std::recursive_mutex> registryMutex) override;
		void spawnVisualEntity() override;
		Animatable getVisualEntityAnimatable() override;
		bool legalCheck(std::string& message, LevelPack& levelPack, SpriteLoader& spriteLoader);
		sf::VertexArray getMovementPath(float timeResolution, float playerX, float playerY) override;

		inline bool testModeIDSet() { return testModeIDHasBeenSet; }
		inline TEST_MODE getTestMode() { return testMode; }
		inline int getTestModeID() { return testModeID; }
		inline void setTestMode(TEST_MODE testMode) { this->testMode = testMode; }
		inline void setTestModeID(int id) { testModeID = id; testModeIDHasBeenSet = true; }

	private:
		EntityCreationQueue& queue;

		TEST_MODE testMode = ENEMY;
		// ID of the thing being tested, depending on testMode
		// ATTACK -> EditorAttack ID
		// ATTACK_PATTERN -> EditorAttackPattern ID
		// PHASE -> EditorEnemyPhase ID
		// ENEMY -> EditorEnemy ID
		int testModeID;
		bool testModeIDHasBeenSet = false;
	};
	/*
	Placeholder for entities that represent control points when editing bezier EMPAs.
	*/
	class BezierControlPointPlaceholder : public EntityPlaceholder {
	public:
		/*
		Note that the control point being represented is not specified. When bezier control point editing ends in GameplayTestWindow, all existing
		BezierControlPointPlaceholders will be converted to coordinates and back-inserted into the list of control points in the EMPA starting with
		the placeholder with the lowest ID.
		*/
		inline BezierControlPointPlaceholder(int id, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, LevelPack& levelPack) : EntityPlaceholder(id, registry, spriteLoader, levelPack) {
		}

		void runTest(std::shared_ptr<std::recursive_mutex> registryMutex) override;
		void spawnVisualEntity() override;
		Animatable getVisualEntityAnimatable() override;
		sf::VertexArray getMovementPath(float timeResolution, float playerX, float playerY) override;
	};
	class EMPTestEntityPlaceholder : public EntityPlaceholder {
	public:
		/*
		Constructor for placeholders that test EMPs that aren't part of an EditorAttack.

		emp - EMP being tested
		sourceID - ID of the EditorAttack emp comes from, if empFromAttack is true. If empFromAttack is false, sourceID is the EditorAttackPattern emp is from
		empFromAttack - whether the emp comes from an EditorAttack (true) or an EditorAttackPattern (false)
		*/
		inline EMPTestEntityPlaceholder(int id, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, LevelPack& levelPack, EntityCreationQueue& queue, std::shared_ptr<EditorMovablePoint> emp, int sourceID, bool empFromAttack) :
			EntityPlaceholder(id, registry, spriteLoader, levelPack), queue(queue), emp(emp), sourceID(sourceID), empFromAttack(empFromAttack) {
		}

		void runTest(std::shared_ptr<std::recursive_mutex> registryMutex) override;
		void spawnVisualEntity() override;
		Animatable getVisualEntityAnimatable() override;
		bool legalCheck(std::string& message, LevelPack& levelPack, SpriteLoader& spriteLoader);
		sf::VertexArray getMovementPath(float timeResolution, float playerX, float playerY) override;

		inline int getEMPID() { return emp->getID(); }
		inline bool empIsFromAttack() { return empFromAttack; }
		inline int getSourceID() { return sourceID; }

	private:
		EntityCreationQueue& queue;
		std::shared_ptr<EditorMovablePoint> emp;
		int sourceID;
		bool empFromAttack;

		// List of precomputed positions of the EMP relative to its starting position. The list ends at either the first nondeterministic EMPA or the EMP's lifespan.
		// These positions are calculated when runTest() is called.
		std::vector<sf::Vector2f> positions;
	};

	const float GUI_PADDING_X = 20;
	const float GUI_PADDING_Y = 10;
	const int UNDO_STACK_MAX = 100;
	const float LEFT_PANEL_WIDTH = 0.25f;
	const float RIGHT_PANEL_WIDTH = 0.2f;
	const float CAMERA_SPEED = 100; // World units per second
	const float MOVEMENT_PATH_TIME_RESOLUTION = 0.05f; // Time between each visualized movement path vertex

	// Mutex to make sure entities aren't destroyed while being iterated through
	std::shared_ptr<std::recursive_mutex> registryMutex;

	std::shared_ptr<sf::Sprite> currentCursor; // nullptr if default cursor

	// This should be modified only by setCameraZoom()
	float cameraZoom = 1.0f;
	bool draggingCamera = false;
	
	bool draggingPlaceholder = false;
	// World pos of the placeholder being dragged at the moment it started being dragged
	sf::Vector2f placeholderPosBeforeDragging;

	// Set to true while left mouse is held down right after selecting a placeholder.
	// Used for preventing deselection from occurring in the same mouse press/release sequence that triggers selection.
	bool justSelectedPlaceholder = false;

	// Screen coordinates of the mouse in the last MouseMove event while
	// draggingCamera was true
	int previousCameraDragCoordsX, previousCameraDragCoordsY;
	// Screen coordinates of the mouse in the last MouseMove event while
	// draggingPlaceholder was true
	int previousPlaceholderDragCoordsX, previousPlaceholderDragCoordsY;
	// Screen coordinates of the first left  mouse pressed event since the last left mouse released event
	int initialMousePressX, initialMousePressY;
	// Camera position right before gameplay test started
	sf::Vector2f preTestCameraCenter;
	// Camera zoom level right before gameplay test started
	float preTestCameraZoom;

	// Should be modified only through setter
	bool placingNewEnemy = false;

	// Should be modified only through setter
	bool manuallySettingPlaceholderPosition = false;

	// This bool is used to prevent infinite loops (eg selectEMP() causing emplTree's connect("ItemSelected") to fire, which
	// calls selectEMP() again)
	bool ignoreSignal = false;

	std::shared_ptr<sf::Sprite> movingEnemyPlaceholderCursor;
	std::shared_ptr<sf::Sprite> movingPlayerPlaceholderCursor;

	std::shared_ptr<PlayerEntityPlaceholder> playerPlaceholder;
	// Maps placeholder ID to placeholder; contains all EnemyEntityPlaceholders and EMPTestEntityPlaceholders
	// If editingBezierControlPoints is true, all placeholders' positions in here correspond to bezier control point positions, maintaining order
	std::map<int, std::shared_ptr<EntityPlaceholder>> nonplayerPlaceholders; 
	std::shared_ptr<EntityPlaceholder> selectedPlaceholder;
	bool selectedPlaceholderIsPlayer;
	int nextPlaceholderID = 0;
	int mostRecentNewEnemyPlaceholderID;

	bool paused = false;
	bool testInProgress = false;

	UndoStack undoStack;

	std::shared_ptr<LevelPack> levelPack;
	std::shared_ptr<SpriteLoader> spriteLoader;
	std::shared_ptr<EntityCreationQueue> queue;

	entt::DefaultRegistry registry;

	std::unique_ptr<MovementSystem> movementSystem;
	std::unique_ptr<RenderSystem> renderSystem;
	std::unique_ptr<DebugRenderSystem> debugRenderSystem;
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
	std::shared_ptr<ScrollableListBox> entityPlaceholdersList; // Item ID is placeholder ID
	std::shared_ptr<tgui::Button> newEnemyPlaceholder;
	std::shared_ptr<tgui::Button> deleteEnemyPlaceholder;
	std::shared_ptr<tgui::Button> startAndEndTest;
	std::shared_ptr<tgui::Button> toggleBottomPanelDisplay;
	// These 2 buttons are only used when editing bezier control points
	// if no placeholder is selected, this button adds a new placeholder at index 0; otherwise add at selectedEMPAIndex - 1
	std::shared_ptr<tgui::Button> addControlPointPlaceholderAbove;
	// if no EMPA is selected, this button adds a new EMPA at last index; otherwise add at selectedEMPAIndex + 1
	std::shared_ptr<tgui::Button> addControlPointPlaceholderBelow;

	std::shared_ptr<tgui::ScrollablePanel> rightPanel;
	std::shared_ptr<tgui::Label> entityPlaceholderXLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> entityPlaceholderX; // This and entityPlaceholderY should have their value changed only by setEntityPlaceholderXWidgetValue() and setEntityPlaceholderYWidgetValue()
	std::shared_ptr<tgui::Label> entityPlaceholderYLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> entityPlaceholderY;
	std::shared_ptr<tgui::Button> entityPlaceholderManualSet;
	std::shared_ptr<tgui::Button> setEnemyPlaceholderTestMode;
	std::shared_ptr<tgui::Label> testModeIDLabel;
	std::shared_ptr<ScrollableListBox> testModeID; // contains all Editor____ objects in the LevelPack (______ part depends on currently selected placeholder's test mode)
	std::shared_ptr<ScrollableListBox> testModePopup; // popup created when setEnemyPlaceholderTestMode is clicked
	std::shared_ptr<tgui::Label> showMovementPathLabel;
	std::shared_ptr<tgui::CheckBox> showMovementPath;

	std::shared_ptr<tgui::ScrollablePanel> bottomPanel;
	std::shared_ptr<tgui::Label> logs;

	std::shared_ptr<tgui::Button> externalEndTest;
	// Only visible when editing bezier control points
	std::shared_ptr<tgui::Button> bezierFinishEditing;

	// Signal that is emitted right before the user stops editing
	// The bool parameter is whether any changes were made to the control points and the vector is the new list of
	// control points. The vector should be ignored if the bool parameter is false.
	std::shared_ptr<entt::SigH<void(bool, std::vector<sf::Vector2f>)>> onBezierControlPointEditingEnd;
	bool editingBezierControlPoints = false;
	// The lifespan of the MoveCustomBezierEMPA being edited
	float editingBezierEMPALifespan;
	// The desired ID of the next new control point placeholder
	int nextBezierControlPointPlaceholderDesiredID;
	// nonplayerPlaceholders cached at the moment right before it is replaced by bezier control points
	std::map<int, std::shared_ptr<EntityPlaceholder>> cachedNonplayerPlaceholdersForEditingBezierControlPoints;
	// nonplayerPlaceholders cached at the moment right before gameplay test while editing bezier control points
	std::map<int, std::shared_ptr<EntityPlaceholder>> cachedNonplayerPlaceholdersForBezierControlPointsTest;
	// The last-saved control points of the EMPA being edited
	std::vector<sf::Vector2f> lastSavedBezierControlPoints;

	// Maps placeholder IDs to the VertexArray that represents the placeholder's movement path
	// All VertexArrays in this map are drawn on-screen while testInProgress is false
	std::map<int, sf::VertexArray> placeholderMovementPaths;
	// Movement path for bezier control point editing
	sf::VertexArray bezierMovementPath;

	// Used for the save changes confirmation signal
	void onBezierFinishEditingConfirmationPrompt(bool saveChanges);

	void setEntityPlaceholderXWidgetValue(float value);
	void setEntityPlaceholderYWidgetValue(float value);
	void onEntityPlaceholderXValueSet(float value);
	void onEntityPlaceholderYValueSet(float value);

	void onLevelPackChange();

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
	void runGameplayTest();
	void endGameplayTest();
	void selectPlaceholder(std::shared_ptr<EntityPlaceholder> placeholder);
	void deselectPlaceholder();
	void deletePlaceholder(int placeholderID);
	void updateEntityPlaceholdersList();

	void setPlacingNewEnemy(bool placingNewEnemy);
	void setManuallySettingPlaceholderPosition(std::shared_ptr<EntityPlaceholder> placeholder, bool manuallySettingPlaceholderPosition);
    /*
    Toggle depends on whether bottomPanel is visible at the time of this function call.
    */
	void toggleLogsDisplay();
	void clearLogs();
	void logMessage(std::string message);

	/*
	Show the movement path of some placeholder.
	This function only does something for placeholders that are one of the following:
		-BezierControlPointPlaceholder
		-EMPTestEntityPlaceholder
		-EnemyEntityPlaceholder with test mode EnemyEntityPlaceholder::ATTACK_PATTERN
	*/
	void showPlaceholderMovementPath(std::shared_ptr<EntityPlaceholder> placeholder);
	/*
	Show the movement path of the bezier control points being edited.
	Should only be called when editing bezier control points.
	This function also recalculates the path if the path is already visible.
	*/
	void showBezierMovementPath();
};

class MainEditorWindow : public EditorWindow {
public:
	MainEditorWindow(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);
	inline static std::shared_ptr<MainEditorWindow> create(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL) {
		return std::make_shared<MainEditorWindow>(tguiMutex, windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval);
	}

	void loadLevelPack(std::string levelPackName);

private:
	std::shared_ptr<AudioPlayer> audioPlayer;
	std::shared_ptr<LevelPack> levelPack;
	std::shared_ptr<SpriteLoader> spriteLoader;

	std::shared_ptr<TabsWithPanel> mainPanel;
	std::shared_ptr<TabsWithPanel> leftPanel;

	std::shared_ptr<tgui::Panel> attacksTreeViewPanel;
	std::shared_ptr<tgui::TreeView> attacksTreeView;

	/*
	Returns the string to be shown for each EditorAttack in the attack list in the attack tab.
	*/
	static sf::String getAttackTextInAttackList(const EditorAttack& attack);
	/*
	Returns the string to be shown for each EditorMovablePoint in the attack list in the attack tab.
	*/
	static sf::String getEMPTextInAttackList(const EditorMovablePoint& emp);
};