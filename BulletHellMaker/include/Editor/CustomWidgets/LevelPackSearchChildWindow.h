#pragma once
#include <LevelPack/LevelPack.h>
#include <Editor/CustomWidgets/ChildWindow.h>
#include <Editor/CustomWidgets/TabsWithPanel.h>

class MainEditorWindow;

class LevelPackSearchChildWindow : public ChildWindow {
public:
	enum class LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE {
		SPRITE_SHEET,
		ATTACK,
		EMP,
		PLAYER,
		PLAYER_POWER_TIER,
		ENEMY,
		ENEMY_PHASE_USAGE,
		BULLET_MODEL,

		DESCRIPTOR	// Used to indicate that a LevelPackSearchResultNode contains 
					// only text (the name field) and doesn't represent an actual object type
	};

	struct LevelPackSearchResultNode {
		LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE type;
		int id; // Unused when type is SPRITE_SHEET, PLAYER; when type is PLAYER_POWER_TIER or ENEMY_PHASE_USAGE, the id is the tier/phase's index
		std::string name; // Unused when type is PLAYER

		std::vector<std::shared_ptr<LevelPackSearchResultNode>> children;

		std::string toTreeViewItem();
	};

	struct LevelPackSearchFindAllResult {
		std::vector<std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode>> resultNodes;
	};

	LevelPackSearchChildWindow(MainEditorWindow& mainEditorWindow);
	static std::shared_ptr<LevelPackSearchChildWindow> create(MainEditorWindow& mainEditorWindow) {
		return std::make_shared<LevelPackSearchChildWindow>(mainEditorWindow);
	}

	/*
	Should be called whenever the parent MainEditorWindow loads a new LevelPack.
	*/
	void onMainEditorWindowLevelPackLoad();

	/*
	Should be called whenever the parent MainEditorWindow's level pack is modified.
	*/
	void onMainEditorWindowLevelPackModify(LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE type, int id);

private:
	static const std::string RESULTS_TAB_NAME;

	MainEditorWindow& mainEditorWindow;
	std::shared_ptr<TabsWithPanel> tabs;

	LevelPackSearchFindAllResult currentResults;

	std::shared_ptr<tgui::Panel> findAllResultsPanel;
	std::shared_ptr<tgui::TreeView> findAllResultsTreeView;

	std::shared_ptr<tgui::Panel> replaceAllResultsPanel;
	std::shared_ptr<tgui::TreeView> replaceAllResultsTreeView;
	std::shared_ptr<tgui::Button> replaceAllResultsApplyButton;
	std::shared_ptr<tgui::Button> replaceAllResultsCancelButton;

	LevelPackSearchFindAllResult findAll(std::string valueType, std::string value, std::string secondaryValue);
	void displayFindAllResult(LevelPackSearchFindAllResult result);
	void populateTreeViewWithSearchResult(std::shared_ptr<tgui::TreeView> treeView, LevelPackSearchFindAllResult result);

	/*
	Helper function for populateTreeViewWithSearchResult().
	*/
	void populateTreeViewWithSearchResultRecursive(std::shared_ptr<tgui::TreeView> treeView, std::vector<tgui::String> pathToNode,
		std::shared_ptr<LevelPackSearchResultNode> node);
};