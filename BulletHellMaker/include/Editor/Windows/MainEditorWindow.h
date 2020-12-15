#pragma once
#include <DataStructs/UndoStack.h>
#include <Game/AudioPlayer.h>
#include <Editor/CopyPaste.h>
#include <Editor/Windows/EditorWindow.h>
#include <Editor/CustomWidgets/TextNotification.h>
#include <Editor/CustomWidgets/LevelPackSearchChildWindow.h>
#include <Editor/CustomWidgets/TabsWithPanel.h>
#include <Editor/SpriteSheets/SpriteSheetsListPanel.h>
#include <Editor/SpriteSheets/SpriteSheetMetafileEditor.h>
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

	/*
	Saves an attack if it has unsaved changes.
	*/
	void saveAttackChanges(int id);
	/*
	Saves attacks if they have unsaved changes.
	*/
	void saveAttackChanges(std::set<size_t> ids);
	/*
	Saves an attack pattern if it has unsaved changes.
	*/
	void saveAttackPatternChanges(int id);
	/*
	Saves attack patterns if they have unsaved changes.
	*/
	void saveAttackPatternChanges(std::set<size_t> ids);
	/*
	Saves a sprite sheet if it has unsaved changes.
	*/
	void saveSpriteSheetChanges(std::string spriteSheetName);
	/*
	Saves a sprite sheet if it has unsaved changes.
	*/
	void saveSpriteSheetChanges(std::set<std::string> spriteSheetNames);
	/*
	Saves all unsaved changes.
	*/
	void saveAllChanges();

	std::shared_ptr<LevelPackObjectsListView> getAttacksListView();
	std::shared_ptr<LevelPackObjectsListPanel> getAttacksListPanel();

	std::shared_ptr<LevelPackObjectsListView> getAttackPatternsListView();
	std::shared_ptr<LevelPackObjectsListPanel> getAttackPatternsListPanel();

	std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& getUnsavedAttacks();
	std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& getUnsavedAttackPatterns();
	std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& getUnsavedEnemies();
	std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& getUnsavedEnemyPhases();
	std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& getUnsavedBulletModels();
	std::map<std::string, std::shared_ptr<SpriteSheet>>& getUnsavedSpriteSheets();

	/*
	Returns whether there are any unsaved changes.
	*/
	bool hasUnsavedChanges();

	/*
	Opens the preview window.
	*/
	void openPreviewWindow();

	/*
	Opens a sprite sheet in the left panel's sprite sheets list so that
	its corresponding tab appears in the main panel.

	spriteSheetName - the name of the sprite sheet in SpriteLoader
	*/
	void openLeftPanelSpriteSheet(std::string spriteSheetName);
	/*
	Opens a single attack in the left panel's attack list so that
	its corresponding tab appears in the main panel.
	*/
	void openLeftPanelAttack(int attackID);
	/*
	Opens a single attack pattern in the left panel's attack pattern list
	so that its corresponding tab appears in the main panel.
	*/
	void openLeftPanelAttackPattern(int attackPatternID);
	/*
	Opens a single enemy phase in the left panel's enemy phase list
	so that its corresponding tab appears in the main panel.
	*/
	void openLeftPanelEnemyPhase(int enemyPhaseID);
	/*
	Opens the player so that its corresponding tab appears in the main panel.
	*/
	void openLeftPanelPlayer();

	/*
	Opens the find/replace all child window.
	*/
	void openSearchChildWindow();

	/*
	Returns all instances of a sprite name in the LevelPack being edited.
	*/
	LevelPackSearchChildWindow::LevelPackSearchFindAllResult findAllInstancesOfSpriteName(std::string spriteSheetName, std::string spriteName);
	/*
	Returns all instances of an animation name in the LevelPack being edited.
	*/
	LevelPackSearchChildWindow::LevelPackSearchFindAllResult findAllInstancesOfAnimationName(std::string spriteSheetName, std::string spriteName);

	// ---------- Operations that affect both the level pack and the preview window's level pack --------------

	void updateAttack(std::shared_ptr<EditorAttack> attack);
	void updateAttackPattern(std::shared_ptr<EditorAttackPattern> attackPattern);

	void deleteAttack(int id);
	void deleteAttackPattern(int id);

	/*
	Reloads the level pack's SpriteLoader to match changes in the sprite sheets folder.
	*/
	void reloadSpriteLoader();

protected:
	bool handleEvent(sf::Event event) override;

private:
	const static std::string LEFT_PANEL_SPRITE_SHEETS_TAB_NAME;
	const static std::string LEFT_PANEL_ATTACK_LIST_TAB_NAME;
	// The string format used as the tab set identifier for caching the mainPanel tabs when closing an opened attack.
	const static std::string LEFT_PANEL_ATTACK_TABS_SET_IDENTIFIER_FORMAT;
	const static std::string LEFT_PANEL_ATTACK_PATTERN_LIST_TAB_NAME;
	const static std::string LEFT_PANEL_ATTACK_PATTERN_TABS_SET_IDENTIFIER_FORMAT;

	const static std::string MAIN_PANEL_ATTACK_TAB_NAME_FORMAT;
	const static std::string MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT;

	const static std::string MAIN_PANEL_SPRITE_SHEET_TAB_NAME_FORMAT;

	std::shared_ptr<AudioPlayer> audioPlayer;
	std::shared_ptr<LevelPack> levelPack;
	std::shared_ptr<SpriteLoader> spriteLoader;

	// Whether levelPack should be able to be edited
	bool loadedEditableLevelPack = false;

	std::shared_ptr<TabsWithPanel> mainPanel;
	std::shared_ptr<TabsWithPanel> leftPanel;

	// ChildWindow for doing "Find all" and "Replace all"
	std::shared_ptr<LevelPackSearchChildWindow> searchChildWindow;

	std::shared_ptr<TextNotification> clipboardNotification;

	// For copy-pasting
	Clipboard clipboard;

	// -------------------- Part of leftPanel --------------------
	std::shared_ptr<SpriteSheetsListPanel> spriteSheetsListPanel;
	std::shared_ptr<LevelPackObjectsListPanel> attacksListPanel; // Container for attacksListView
	std::shared_ptr<LevelPackObjectsListView> attacksListView; // Child of attacksListPanel
	std::shared_ptr<LevelPackObjectsListPanel> attackPatternsListPanel; // Container for attackPatternsListView
	std::shared_ptr<LevelPackObjectsListView> attackPatternsListView; // Child of attackPatternsListPanel

	// -------------------- Part of mainPanel --------------------
	// Maps an EditorAttack ID to the EditorAttack object that has unsaved changes.
	// If the ID doesn't exist in this map, then there are no unsaved changes
	// for that ID.
	std::map<int, std::shared_ptr<LayerRootLevelPackObject>> unsavedAttacks;
	std::map<int, std::shared_ptr<LayerRootLevelPackObject>> unsavedAttackPatterns;
	std::map<int, std::shared_ptr<LayerRootLevelPackObject>> unsavedEnemies;
	std::map<int, std::shared_ptr<LayerRootLevelPackObject>> unsavedEnemyPhases;
	std::map<int, std::shared_ptr<LayerRootLevelPackObject>> unsavedBulletModels;
	
	// Nullptr when no unsaved player
	std::shared_ptr<EditorPlayer> unsavedPlayer;

	// Maps a SpriteSheet name to the SpriteSheet object that has unsaved changes.
	// If the name doesn't exist in this map, then there are no unsaved changes
	// for that name.
	std::map<std::string, std::shared_ptr<SpriteSheet>> unsavedSpriteSheets;
	// ------------------------------------------------------------

	std::shared_ptr<LevelPackObjectPreviewWindow> previewWindow;

	void showClipboardResult(std::string notification);
	void populateLeftPanelLevelPackObjectListPanel(std::shared_ptr<LevelPackObjectsListView> listView, std::shared_ptr<LevelPackObjectsListPanel> listPanel, 
		std::function<void()> createLevelPackObject, std::function<void(int)> openLevelPackObjectTab);

	/*
	Returns all instances of an animatable in the LevelPack being edited.

	mustBeSprite - if true, find only Animatables that are sprites. If false, find only Animatables that are animations.
	*/
	LevelPackSearchChildWindow::LevelPackSearchFindAllResult findAllInstancesOfAnimatable(std::string spriteSheetName, std::string animatableName, bool mustBeSprite);
	/*
	Returns a LevelPackSearchResultNode containing only children descriptor nodes describing matching Animatables in an EntityAnimatableSet.
	Helper function for findAllInstancesOfAnimatable().
	*/
	std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> findAllInstancesOfAnimatableInAnimatableSet(const EntityAnimatableSet& animatableSet,
		const std::string& spriteSheetName, const std::string& animatableName, bool mustBeSprite);

	void onWindowResize(int width, int height);
};