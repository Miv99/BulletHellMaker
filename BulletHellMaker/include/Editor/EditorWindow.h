#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <thread>

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <entt/entt.hpp>

#include <DataStructs/MovablePoint.h>
#include <DataStructs/UndoStack.h>
#include <Game/EntityCreationQueue.h>
#include <Game/Systems/MovementSystem.h>
#include <Game/Systems/DespawnSystem.h>
#include <Game/Systems/EnemySystem.h>
#include <Game/Systems/SpriteAnimationSystem.h>
#include <Game/Systems/ShadowTrailSystem.h>
#include <Game/Systems/PlayerSystem.h>
#include <Game/Systems/CollectibleSystem.h>
#include <Game/Systems/DebugRenderSystem.h>
#include <Editor/CustomWidgets/SliderWithEditBox.h>
#include <Editor/CustomWidgets/TextNotification.h>
#include <Editor/CustomWidgets/TabsWithPanel.h>
#include <Constants.h>
#include <LevelPack/LevelPack.h>
#include <LevelPack/Attack.h>
#include <LevelPack/EditorMovablePoint.h>
#include <Game/AudioPlayer.h>
#include <Editor/LevelPackObjectList/LevelPackObjectsListView.h>
#include <Editor/CopyPaste.h>
#include <Editor/Previewer/LevelPackObjectPreviewPanel.h>

class AttackEditorPanel;
class LevelPackObjectsListPanel;
class LevelPackObjectPreviewWindow;

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
	Disables all widgets and then prompts the user with a custom message to which the user can respond with either a yes or a no.
	A signal with two parameters is returned. The signal will be published only once, when the user responds. The bool parameter
	will be true if the user responds with yes, false if no. The user object parameter will contain a copy of the userObject parameter.
	After the user response, all widgets' enabled/disabled statuses are set back to what they were before this function call.
	Each call to promptConfirmation() returns a new signal.
	*/
	template<class T>
	std::shared_ptr<entt::SigH<void(bool, T)>> promptConfirmation(std::string message, T userObject);

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
	inline sf::Vector2f getMousePos() { return mousePos; }
	inline sf::Vector2f getLastMousePressPos() { return lastMousePressPos; }

protected:
	std::shared_ptr<sf::RenderWindow> window;
	std::shared_ptr<tgui::Gui> gui;

	// Mutex used to make sure multiple tgui widgets aren't being instantiated at the same time in different threads.
	// tgui::Gui draw() calls also can't be done at the same time.
	// Apparently tgui gets super messed up with multithreading.
	std::shared_ptr<std::recursive_mutex> tguiMutex;

	// The last known position of the mouse
	sf::Vector2f mousePos = sf::Vector2f(0, 0);
	// The position of the mouse at the last mouse press
	sf::Vector2f lastMousePressPos = sf::Vector2f(0, 0);

	virtual void physicsUpdate(float deltaTime);
	virtual void render(float deltaTime);
	/*
	Returns whether the input was consumed.
	*/
	virtual bool handleEvent(sf::Event event);

	/*
	Called when the RenderWindow window is initialized.
	*/
	virtual void onRenderWindowInitialization();

private:
	int nextVertexArrayID = 0;
	std::map<int, sf::VertexArray> vertexArrays;

	std::string windowTitle;
	int windowWidth, windowHeight;

	bool windowCloseQueued = false;

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

	bool confirmationPanelOpen = false;

	// A list of all Widgets in gui whose value of isEnabled() was true right before promptConfirmation() was called
	std::list<std::shared_ptr<tgui::Widget>> widgetsToBeEnabledAfterConfirmationPrompt;

	bool letterboxingEnabled;
	bool scaleWidgetsOnResize;
	
	float renderInterval;
	// Signal that's emitted every time a render call is made
	// function accepts 1 argument: the time since the last render
	std::shared_ptr<entt::SigH<void(float)>> renderSignal;
	// Signal that's emitted every time the window resizes
	std::shared_ptr<entt::SigH<void(int, int)>> resizeSignal;
	// Signal that's emitted right before the window closes
	std::shared_ptr<entt::SigH<void()>> closeSignal;

	/*
	Call to stop the confirmation prompt started by promptConfirmation().
	*/
	void closeConfirmationPanel();
};

class MainEditorWindow : public EditorWindow {
public:
	MainEditorWindow(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);
	~MainEditorWindow();
	inline static std::shared_ptr<MainEditorWindow> create(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL) {
		return std::make_shared<MainEditorWindow>(tguiMutex, windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval);
	}

	void loadLevelPack(std::string levelPackName);

	/*
	Create an attack in the LevelPack as an undoable/redoable action.
	*/
	void createAttack();
	/*
	Create an attack pattern in the LevelPack as an undoable/redoable action.
	*/
	void createAttackPattern();
	/*
	Overwrite existing EditorAttacks with deep copies of new ones as an undoable/redoable action.
	attacks - the list of only the new EditorAttacks
	undoStack - the UndoStack to add the action to; set to nullptr if the action should not be able to be undone
	*/
	void overwriteAttacks(std::vector<std::shared_ptr<EditorAttack>> attacks, UndoStack* undoStack);
	/*
	Overwrite existing EditorAttackPatterns with deep copies of new ones as an undoable/redoable action.
	objects - the list of only the new EditorAttackPatterns
	undoStack - the UndoStack to add the action to; set to nullptr if the action should not be able to be undone
	*/
	void overwriteAttackPatterns(std::vector<std::shared_ptr<EditorAttackPattern>> objects, UndoStack* undoStack);
	/*
	Reloads an attack tab to reflect new changes to the associated EditorAttack that didn't come from
	the tab itself. If the EditorAttack no longer exists, the tab will be removed.
	*/
	void reloadAttackTab(int attackID);
	/*
	Reloads an attack pattern tab to reflect new changes to the associated EditorAttackPattern that didn't come from
	the tab itself. If the EditorAttackPattern no longer exists, the tab will be removed.
	*/
	void reloadAttackPatternTab(int id);

	std::shared_ptr<LevelPackObjectsListView> getAttacksListView();
	std::shared_ptr<LevelPackObjectsListPanel> getAttacksListPanel();

	std::map<int, std::shared_ptr<LevelPackObject>>& getUnsavedAttacks();
	std::map<int, std::shared_ptr<LevelPackObject>>& getUnsavedAttackPatterns();
	std::map<int, std::shared_ptr<LevelPackObject>>& getUnsavedEnemies();
	std::map<int, std::shared_ptr<LevelPackObject>>& getUnsavedEnemyPhases();
	std::map<int, std::shared_ptr<LevelPackObject>>& getUnsavedBulletModels();

	/*
	Open a single attack in the left panel's attack list so that
	its corresponding tab appears in the main panel.
	*/
	void openLeftPanelAttack(int attackID);
	/*
	Open a single attack pattern in the left panel's attack pattern list
	so that its corresponding tab appears in the main panel.
	*/
	void openLeftPanelAttackPattern(int attackPatternID);
	/*
	Open a single enemy phase in the left panel's enemy phase list
	so that its corresponding tab appears in the main panel.
	*/
	void openLeftPanelEnemyPhase(int enemyPhaseID);
	/*
	Open the player so that its corresponding tab appears in the main panel.
	*/
	void openLeftPanelPlayer();

protected:
	bool handleEvent(sf::Event event) override;

private:
	const std::string LEFT_PANEL_ATTACK_LIST_TAB_NAME = "Attacks";
	// The string format used as the tab set identifier for caching the mainPanel
	// tabs when closing an opened attack.
	// %d = the attack's ID
	const std::string LEFT_PANEL_ATTACK_TABS_SET_IDENTIFIER_FORMAT = "Attack %d";
	const std::string LEFT_PANEL_ATTACK_PATTERN_LIST_TAB_NAME = "Attack patterns";
	const std::string LEFT_PANEL_ATTACK_PATTERN_TABS_SET_IDENTIFIER_FORMAT = "Atk. Pattern %d";

	const std::string MAIN_PANEL_ATTACK_TAB_NAME_FORMAT = "Attack %d";
	const std::string MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT = "Atk. Pattern %d";

	std::shared_ptr<AudioPlayer> audioPlayer;
	std::shared_ptr<LevelPack> levelPack;
	std::shared_ptr<SpriteLoader> spriteLoader;

	std::shared_ptr<TabsWithPanel> mainPanel;
	std::shared_ptr<TabsWithPanel> leftPanel;

	std::shared_ptr<TextNotification> clipboardNotification;

	// For copy-pasting
	Clipboard clipboard;

	// -------------------- Part of leftPanel --------------------
	std::shared_ptr<LevelPackObjectsListPanel> attacksListPanel; // Container for attacksListView
	std::shared_ptr<LevelPackObjectsListView> attacksListView; // Child of attacksListPanel
	std::shared_ptr<LevelPackObjectsListPanel> attackPatternsListPanel; // Container for attackPatternsListView
	std::shared_ptr<LevelPackObjectsListView> attackPatternsListView; // Child of attackPatternsListPanel

	// -------------------- Part of mainPanel --------------------
	// Maps an EditorAttack ID to the EditorAttack object that has unsaved changes.
	// If the ID doesn't exist in this map, then there are no unsaved changes
	// for that ID.
	std::map<int, std::shared_ptr<LevelPackObject>> unsavedAttacks;
	std::map<int, std::shared_ptr<LevelPackObject>> unsavedAttackPatterns;
	std::map<int, std::shared_ptr<LevelPackObject>> unsavedEnemies;
	std::map<int, std::shared_ptr<LevelPackObject>> unsavedEnemyPhases;
	std::map<int, std::shared_ptr<LevelPackObject>> unsavedBulletModels;
	// ------------------------------------------------------------

	std::shared_ptr<LevelPackObjectPreviewWindow> previewWindow;

	void showClipboardResult(std::string notification);
	void populateLeftPanelLevelPackObjectListPanel(std::shared_ptr<LevelPackObjectsListView> listView, std::shared_ptr<LevelPackObjectsListPanel> listPanel, std::function<void()> createLevelPackObject, std::function<void(int)> openLevelPackObjectTab);
};

template<class T>
inline std::shared_ptr<entt::SigH<void(bool, T)>> EditorWindow::promptConfirmation(std::string message, T userObject) {
	// Don't allow 2 confirmation prompts at the same time
	if (confirmationPanelOpen) {
		return nullptr;
	}

	std::shared_ptr<entt::SigH<void(bool, T)>> confirmationSignal = std::make_shared<entt::SigH<void(bool, T)>>();

	// Disable all widgets
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
	for (std::shared_ptr<tgui::Widget> ptr : gui->getWidgets()) {
		if (ptr->isEnabled()) {
			widgetsToBeEnabledAfterConfirmationPrompt.push_back(ptr);
		}
		ptr->setEnabled(false);
	}
	
	confirmationYes->connect("Pressed", [this, confirmationSignal, userObject]() {
		confirmationSignal->publish(true, userObject);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();
	});
	confirmationNo->connect("Pressed", [this, confirmationSignal, userObject]() {
		confirmationSignal->publish(false, userObject);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();
	});

	confirmationText->setText(message);
	gui->add(confirmationPanel);
	confirmationPanelOpen = true;

	return confirmationSignal;
}

/*
Window used to show a LevelPackObjectPreviewPanel.

This is necessary for LevelPackObjectPreviewPanel because SFML requires render calls
to be made on the same thread as the construction of the RenderWindow. This object also acts
as a controller between whatever uses it and the underlying LevelPackObjectPreviewPanel by
limiting resource-intensive actions when the window is closed.
*/
class LevelPackObjectPreviewWindow : public EditorWindow {
public:
	LevelPackObjectPreviewWindow(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, std::string levelPackName, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);

	void previewNothing();
	void previewAttack(const std::shared_ptr<EditorAttack> attack);
	void previewAttackPattern(const std::shared_ptr<EditorAttackPattern> attackPattern);

	/*
	Should be called whenever an EditorAttack in the LevelPack being edited is modified.
	*/
	void onOriginalLevelPackAttackModified(const std::shared_ptr<EditorAttack> attack);
	/*
	Should be called whenever an EditorAttackPattern in the LevelPack being edited is modified.
	*/
	void onOriginalLevelPackAttackPatternModified(const std::shared_ptr<EditorAttackPattern> attackPattern);

private:
	std::shared_ptr<LevelPackObjectPreviewPanel> previewPanel;

	std::shared_ptr<tgui::Panel> infoPanel;
	std::shared_ptr<tgui::Label> previewObjectLabel;
	std::shared_ptr<tgui::Label> delayLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> delay;
	std::shared_ptr<tgui::Label> timeMultiplierLabel;
	std::shared_ptr<SliderWithEditBox> timeMultiplier;
	std::shared_ptr<tgui::Button> resetPreview;
	std::shared_ptr<tgui::Button> resetCamera;
	std::shared_ptr<tgui::Button> setPlayerSpawn;
	std::shared_ptr<tgui::Button> setSource;
	std::shared_ptr<tgui::CheckBox> useDebugRenderSystem;
	std::shared_ptr<tgui::CheckBox> lockCurrentPreviewCheckBox;
	std::shared_ptr<tgui::CheckBox> invinciblePlayer;

	std::shared_ptr<tgui::TextBox> logs;

	std::string levelPackName;

	// While this is checked, new previews cannot start
	bool lockCurrentPreview = false;

	std::thread previewThread;

	bool ignoreSignals = false;

	/*
	Update widget positions/sizes depending on which ones are visible.
	*/
	void updateWidgetsPositionsAndSizes();

	bool handleEvent(sf::Event event) override;

	void onRenderWindowInitialization() override;

	void physicsUpdate(float deltaTime) override;
	void render(float deltaTime) override;
};