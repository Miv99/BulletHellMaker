#pragma once
#include <DataStructs/UndoStack.h>
#include <Game/AudioPlayer.h>
#include <Editor/CopyPaste.h>
#include <Editor/Windows/EditorWindow.h>
#include <Editor/CustomWidgets/TextNotification.h>
#include <Editor/CustomWidgets/TabsWithPanel.h>
#include <LevelPack/LevelPackObject.h>
#include <LevelPack/Attack.h>
#include <LevelPack/AttackPattern.h>
#include <LevelPack/Enemy.h>
#include <LevelPack/EnemyPhase.h>
#include <LevelPack/Level.h>

class AttackEditorPanel;
class LevelPackObjectsListView;
class LevelPackObjectsListPanel;
class LevelPackObjectPreviewWindow;

class MainEditorWindow : public EditorWindow {
public:
	MainEditorWindow(std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);
	~MainEditorWindow();
	static std::shared_ptr<MainEditorWindow> create(std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL) {
		return std::make_shared<MainEditorWindow>(windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval);
	}

	void loadLevelPack(std::string levelPackName);

	/*
	Create an attack in the LevelPack.
	*/
	void createAttack(bool undoable);
	/*
	Create an attack pattern in the LevelPack.
	*/
	void createAttackPattern(bool undoable);
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

	std::shared_ptr<LevelPackObjectsListView> getAttackPatternsListView();
	std::shared_ptr<LevelPackObjectsListPanel> getAttackPatternsListPanel();

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


	// ---------- Operations that affect both the level pack and the preview window's level pack --------------

	void updateAttack(std::shared_ptr<EditorAttack> attack);
	void updateAttackPattern(std::shared_ptr<EditorAttackPattern> attackPattern);

	void deleteAttack(int id);
	void deleteAttackPattern(int id);

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