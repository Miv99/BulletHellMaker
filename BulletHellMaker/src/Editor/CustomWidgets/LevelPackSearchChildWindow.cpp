#include <Editor/CustomWidgets/LevelPackSearchChildWindow.h>

#include <GuiConfig.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <Editor/CustomWidgets/EditBox.h>

const std::string LevelPackSearchChildWindow::RESULTS_TAB_NAME = "Results";

LevelPackSearchChildWindow::LevelPackSearchChildWindow(MainEditorWindow& mainEditorWindow) 
	: mainEditorWindow(mainEditorWindow) {

	tabs = TabsWithPanel::create(mainEditorWindow);
	tabs->setSize("100%", "100%");
	add(tabs);


	std::shared_ptr<tgui::Panel> findAllPanel = tgui::Panel::create();
	findAllPanel->setSize("100%", "100%");
	tabs->addTab("Find all", findAllPanel, true, false);

	std::shared_ptr<tgui::Label> valueTypeLabel = tgui::Label::create();
	valueTypeLabel->setText("Search for:");
	valueTypeLabel->setTextSize(TEXT_SIZE);
	valueTypeLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	findAllPanel->add(valueTypeLabel);

	std::shared_ptr<tgui::ComboBox> valueTypeComboBox = tgui::ComboBox::create();
	valueTypeComboBox->setTextSize(TEXT_SIZE);
	valueTypeComboBox->setPosition(tgui::bindLeft(valueTypeLabel), tgui::bindBottom(valueTypeLabel) + GUI_LABEL_PADDING_Y);
	findAllPanel->add(valueTypeComboBox);

	valueTypeComboBox->addItem("Sprite name");
	valueTypeComboBox->addItem("Animation name");
	// TODO: add to this

	std::shared_ptr<tgui::Label> findLabel = tgui::Label::create();
	findLabel->setText("Find");
	findLabel->setTextSize(TEXT_SIZE);
	findLabel->setPosition(tgui::bindLeft(valueTypeComboBox), tgui::bindBottom(valueTypeComboBox) + GUI_PADDING_Y);
	findAllPanel->add(findLabel);

	std::shared_ptr<EditBox> findEditBox = EditBox::create();
	findEditBox->setTextSize(TEXT_SIZE);
	findEditBox->setPosition(tgui::bindLeft(findLabel), tgui::bindBottom(findLabel) + GUI_LABEL_PADDING_Y);
	findAllPanel->add(findEditBox);

	std::shared_ptr<tgui::Label> secondaryFindLabel = tgui::Label::create();
	secondaryFindLabel->setTextSize(TEXT_SIZE);
	secondaryFindLabel->setPosition(tgui::bindLeft(findEditBox), tgui::bindBottom(findEditBox) + GUI_PADDING_Y);
	secondaryFindLabel->setVisible(false);
	findAllPanel->add(secondaryFindLabel);

	std::shared_ptr<EditBox> secondaryFindEditBox = EditBox::create();
	secondaryFindEditBox->setTextSize(TEXT_SIZE);
	secondaryFindEditBox->setPosition(tgui::bindLeft(secondaryFindLabel), tgui::bindBottom(secondaryFindLabel) + GUI_LABEL_PADDING_Y);
	secondaryFindEditBox->setVisible(false);
	findAllPanel->add(secondaryFindEditBox);

	valueTypeComboBox->onItemSelect([this, secondaryFindLabel, secondaryFindEditBox](tgui::String item) {
		if (item == "Sprite name" || item == "Animation name") {
			secondaryFindLabel->setVisible(true);
			secondaryFindEditBox->setVisible(true);

			secondaryFindLabel->setText("Sprite sheet name");
		} else {
			secondaryFindLabel->setVisible(false);
			secondaryFindEditBox->setVisible(false);
		}
	});

	std::shared_ptr<tgui::Button> findAllButton = tgui::Button::create();
	findAllButton->setText("Find all");
	findAllButton->setTextSize(TEXT_SIZE);
	findAllButton->setPosition("100%" - tgui::bindWidth(findAllButton) - GUI_PADDING_X, "100%" - tgui::bindHeight(findAllButton) - GUI_PADDING_Y);
	findAllButton->setSize(100, TEXT_BUTTON_HEIGHT);
	findAllButton->onPress([this, valueTypeComboBox, findEditBox, secondaryFindEditBox]() {
		LevelPackSearchFindAllResult result = findAll(static_cast<std::string>(valueTypeComboBox->getSelectedItem()),
			static_cast<std::string>(findEditBox->getText()), static_cast<std::string>(secondaryFindEditBox->getText()));
		displayFindAllResult(result);
	});
	findAllPanel->add(findAllButton);
	
	valueTypeComboBox->setSize(getSize().x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	findEditBox->setSize(getSize().x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	secondaryFindEditBox->setSize(getSize().x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);

	findAllPanel->onSizeChange.connect([this, valueTypeComboBox, findEditBox, secondaryFindEditBox](sf::Vector2f newSize) {
		valueTypeComboBox->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		findEditBox->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		secondaryFindEditBox->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	});


	findAllResultsPanel = tgui::Panel::create();
	findAllResultsPanel->setSize("100%", "100%");

	findAllResultsTreeView = tgui::TreeView::create();
	findAllResultsTreeView->setTextSize(TEXT_SIZE);
	findAllResultsTreeView->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	findAllResultsPanel->add(findAllResultsTreeView);

	findAllResultsTreeView->setSize(getSize().x - GUI_PADDING_X * 2, getSize().y - GUI_PADDING_Y * 2);
	findAllResultsPanel->onSizeChange.connect([this](sf::Vector2f newSize) {
		findAllResultsTreeView->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - GUI_PADDING_Y * 2);
	});
	// TODO: right click menu for find all results tree view


	replaceAllResultsPanel = tgui::Panel::create();
	replaceAllResultsPanel->setSize("100%", "100%");
	
	replaceAllResultsTreeView = tgui::TreeView::create();
	replaceAllResultsTreeView->setTextSize(TEXT_SIZE);
	replaceAllResultsTreeView->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	replaceAllResultsPanel->add(replaceAllResultsTreeView);
	// TODO: right click menu for replace all results tree view

	replaceAllResultsCancelButton = tgui::Button::create();
	replaceAllResultsCancelButton->setText("Cancel");
	replaceAllResultsCancelButton->setTextSize(TEXT_SIZE);
	replaceAllResultsCancelButton->setSize(100, TEXT_BUTTON_HEIGHT);
	replaceAllResultsCancelButton->setPosition("100%" - tgui::bindWidth(replaceAllResultsCancelButton) - GUI_PADDING_X, 
		"100%" - tgui::bindHeight(replaceAllResultsCancelButton) - GUI_PADDING_Y);
	replaceAllResultsPanel->add(replaceAllResultsCancelButton);

	replaceAllResultsApplyButton = tgui::Button::create();
	replaceAllResultsApplyButton->setText("Apply");
	replaceAllResultsApplyButton->setTextSize(TEXT_SIZE);
	replaceAllResultsApplyButton->setSize(100, TEXT_BUTTON_HEIGHT);
	replaceAllResultsApplyButton->setPosition(tgui::bindLeft(replaceAllResultsCancelButton) 
		- tgui::bindWidth(replaceAllResultsApplyButton) - GUI_PADDING_X, tgui::bindTop(replaceAllResultsCancelButton));
	replaceAllResultsPanel->add(replaceAllResultsApplyButton);

	replaceAllResultsTreeView->setSize(getSize().x - GUI_PADDING_X * 2, getSize().y - tgui::bindTop(replaceAllResultsCancelButton) - GUI_PADDING_Y);
	replaceAllResultsPanel->onSizeChange.connect([this](sf::Vector2f newSize) {
		replaceAllResultsTreeView->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - tgui::bindTop(replaceAllResultsCancelButton) - GUI_PADDING_Y);
	});
}

void LevelPackSearchChildWindow::onMainEditorWindowLevelPackLoad() {
	if (tabs->hasTab(RESULTS_TAB_NAME)) {
		tabs->removeTab(RESULTS_TAB_NAME);
	}
}

void LevelPackSearchChildWindow::onMainEditorWindowLevelPackModify(LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE type, int id) {
	// To prevent find/replace operation results from displaying outdated info, just remove all result tabs no matter the type of change
	if (tabs->hasTab(RESULTS_TAB_NAME)) {
		tabs->removeTab(RESULTS_TAB_NAME);
	}
}

LevelPackSearchChildWindow::LevelPackSearchFindAllResult LevelPackSearchChildWindow::findAll(std::string valueType, std::string value, std::string secondaryValue) {
	if (valueType == "") {
		return LevelPackSearchFindAllResult();
	} else if (valueType == "Sprite name") {
		return mainEditorWindow.findAllInstancesOfSpriteName(secondaryValue, value);
	} else if (valueType == "Animation name") {
		return mainEditorWindow.findAllInstancesOfAnimationName(secondaryValue, value);
	} else {
		return LevelPackSearchFindAllResult();
	}
	// TODO: add to this as valueTypeComboBox types expand
}

void LevelPackSearchChildWindow::displayFindAllResult(LevelPackSearchFindAllResult result) {
	this->currentResults = result;

	if (tabs->hasTab(RESULTS_TAB_NAME)) {
		tabs->selectTab(RESULTS_TAB_NAME);
	} else {
		tabs->addTab(RESULTS_TAB_NAME, findAllResultsPanel, true, true);
	}

	populateTreeViewWithSearchResult(findAllResultsTreeView, result);
}

void LevelPackSearchChildWindow::populateTreeViewWithSearchResult(std::shared_ptr<tgui::TreeView> treeView, LevelPackSearchFindAllResult result) {
	treeView->removeAllItems();

	for (std::shared_ptr<LevelPackSearchResultNode> node : result.resultNodes) {
		populateTreeViewWithSearchResultRecursive(treeView, {}, node);
	}
}

void LevelPackSearchChildWindow::populateTreeViewWithSearchResultRecursive(std::shared_ptr<tgui::TreeView> treeView, 
	std::vector<tgui::String> pathToNode, std::shared_ptr<LevelPackSearchResultNode> node) {

	pathToNode.push_back(node->toTreeViewItem());
	treeView->addItem(pathToNode, true);

	for (std::shared_ptr<LevelPackSearchResultNode> child : node->children) {
		populateTreeViewWithSearchResultRecursive(treeView, pathToNode, child);
	}
}

std::string LevelPackSearchChildWindow::LevelPackSearchResultNode::toTreeViewItem() {
	if (type == LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::SPRITE_SHEET) {
		return format("Sprite sheet: %s", name.c_str());
	} else if (type == LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::PLAYER) {
		return "Player";
	} else if (type == LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::PLAYER_POWER_TIER) {
		return format("Player power tier #%d", id);
	} else if (type == LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::ENEMY_PHASE_USAGE) {
		return format("Enemy phase usage #%d", id);
	} else if (type == LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR) {
		return name;
	} else {
		std::string prefix;
		switch (type) {
		case LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::ATTACK:
			prefix = "Attack";
			break;
		case LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::EMP:
			prefix = "Movable point";
			break;
		case LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::ENEMY:
			prefix = "Enemy";
			break;
		case LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::BULLET_MODEL:
			prefix = "Bullet model";
			break;
		// TODO: add to this as LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE expands
		}

		if (name.empty()) {
			return format("%s %d", prefix.c_str(), id);
		} else {
			return format("%s %d: %s", prefix.c_str(), id, name.c_str());
		}
	}
}
