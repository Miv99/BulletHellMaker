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
#include "AttacksListView.h"
#include "CopyPaste.h"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <entt/entt.hpp>

class AttackEditorPanel;

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
	inline sf::Vector2f getMousePos() { return mousePos; }

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



/*
An EventCapturable basic tgui::Panel to be used by MainEditorWindow for viewing the EditorAttacks list. Handles undo/redo and copy/paste.
*/
class AttacksListPanel : public tgui::Panel, public EventCapturable {
public:
	AttacksListPanel(std::shared_ptr<AttacksListView> attacksListView, Clipboard& clipboard, int undoStackSize = 50);
	static std::shared_ptr<AttacksListPanel> create(std::shared_ptr<AttacksListView> attacksListView, Clipboard& clipboard, int undoStackSize = 50) {
		return std::make_shared<AttacksListPanel>(attacksListView, clipboard, undoStackSize);
	}

	bool handleEvent(sf::Event event) override;
	UndoStack& getUndoStack();

private:
	std::shared_ptr<AttacksListView> attacksListView;
	UndoStack undoStack;
	Clipboard& clipboard;
};

class MainEditorWindow : public EditorWindow {
public:
	MainEditorWindow(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);
	inline static std::shared_ptr<MainEditorWindow> create(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL) {
		return std::make_shared<MainEditorWindow>(tguiMutex, windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval);
	}

	void loadLevelPack(std::string levelPackName);

	/*
	Create an attack in the LevelPack as an undoable/redoable action.
	*/
	void createAttack();
	/*
	Create an attack in the LevelPack as an undoable/redoable action.
	copyOf - the EditorAttack to be deep-copied into the new EditorAttack
	*/
	void createAttack(std::shared_ptr<EditorAttack> copyOf);
	/*
	Overwrite existing EditorAttacks with deep copies of new ones as an undoable/redoable action.
	attacks - the list of only the new EditorAttacks
	*/
	void overwriteAttacks(std::vector<std::shared_ptr<EditorAttack>> attacks);
	/*
	Reloads an attack tab to reflect new changes to the associated EditorAttack that didn't come from
	the tab itself.
	*/
	void reloadAttackTab(int attackID);

protected:
	void handleEvent(sf::Event event) override;

private:
	const std::string LEFT_PANEL_ATTACK_LIST_TAB_NAME = "Attacks";
	// The string format used as the tab set identifier for caching the mainPanel
	// tabs when closing an opened attack.
	// %d = the attack's ID
	const std::string LEFT_PANEL_ATTACK_TABS_SET_IDENTIFIER_FORMAT = "Attack %d";

	const std::string MAIN_PANEL_ATTACK_TAB_NAME_FORMAT = "Attack %d";

	std::shared_ptr<AudioPlayer> audioPlayer;
	std::shared_ptr<LevelPack> levelPack;
	std::shared_ptr<SpriteLoader> spriteLoader;

	std::shared_ptr<TabsWithPanel> mainPanel;
	std::shared_ptr<TabsWithPanel> leftPanel;

	// For copy-pasting
	Clipboard clipboard;

	// -------------------- Part of leftPanel --------------------
	std::shared_ptr<AttacksListPanel> attacksListViewPanel;
	std::shared_ptr<AttacksListView> attacksListView;
	
	// -------------------- Part of mainPanel --------------------
	// Maps an EditorAttack ID to the EditorAttack object that has unsaved changes.
	// If the ID doesn't exist in this map, then there are no unsaved changes
	// for that ID.
	std::map<int, std::shared_ptr<EditorAttack>> unsavedAttacks;
	// ------------------------------------------------------------

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
};