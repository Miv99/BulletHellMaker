#include <Editor/LevelPackObjectUseRelationship/LevelPackObjectUseRelationshipEditor.h>

#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>

LevelPackObjectUseRelationshipEditor::LevelPackObjectUseRelationshipEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, LevelPack& levelPack, int undoStackSize) 
	: mainEditorWindow(mainEditorWindow), clipboard(clipboard), levelPack(levelPack) {
	listPanel = tgui::Panel::create();
	listPanel->setSize("30%", "100%");

	// Add button
	auto addButton = tgui::Button::create();
	addButton->setText("+");
	addButton->setPosition(0, 0);
	addButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	addButton->connect("Pressed", [this]() {
		// TODO
	});
	listPanel->add(addButton);

	// List view
	listView = LevelPackObjectUseRelationshipListView::create(mainEditorWindow, clipboard);
	listView->setPosition(0, tgui::bindBottom(addButton));
	listView->setSize("100%", tgui::bindHeight(listPanel) - tgui::bindBottom(addButton));
	{
		// Right click menu
		// Menu for single selection
		auto rightClickMenuPopupSingleSelection = createMenuPopup({
			std::make_pair("Open", [this]() {
				// TODO
			}),
			std::make_pair("Copy", [this]() {
				listView->manualCopy();
			}),
			std::make_pair("Paste", [this]() {
				listView->manualPaste();
			}),
			std::make_pair("Paste (override this)", [this]() {
				listView->manualPaste2();
			}),
			std::make_pair("Delete", [this]() {
				listView->manualDelete();
			})
			});
		// Menu for multiple selections
		auto rightClickMenuPopupMultiSelection = createMenuPopup({
			std::make_pair("Copy", [this]() {
				listView->manualCopy();
			}),
			std::make_pair("Paste", [this]() {
				listView->manualPaste();
			}),
			std::make_pair("Paste (override these)", [this]() {
				listView->manualPaste2();
			}),
			std::make_pair("Delete", [this]() {
				listView->manualDelete();
			})
			});
		listView->getListView()->connect("RightClicked", [this, rightClickMenuPopupSingleSelection, rightClickMenuPopupMultiSelection](int index) {
			std::set<std::size_t> selectedItemIndices = listView->getListView()->getSelectedItemIndices();
			auto mousePos = this->mainEditorWindow.getMousePos();
			if (selectedItemIndices.find(index) != selectedItemIndices.end()) {
				// Right clicked a selected item

				// Open the corresponding menu
				if (selectedItemIndices.size() == 1) {
					this->mainEditorWindow.addPopupWidget(rightClickMenuPopupSingleSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupSingleSelection->getSize().y);
				} else {
					this->mainEditorWindow.addPopupWidget(rightClickMenuPopupMultiSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupMultiSelection->getSize().y);
				}
			} else {
				// Right clicked a nonselected item

				// Select the right clicked item
				listView->getListView()->setSelectedItem(index);

				// Open the menu normally
				this->mainEditorWindow.addPopupWidget(rightClickMenuPopupSingleSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupSingleSelection->getSize().y);
			}
		});
	}
	listView->getListView()->connect("DoubleClicked", [this](int index) {
		// TODO
	});
	listPanel->add(listView);
}

LevelPackObjectUseRelationshipListView::LevelPackObjectUseRelationshipListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize) 
	: ListViewScrollablePanel(), CopyPasteable("LevelPackObjectUseRelationship"), mainEditorWindow(mainEditorWindow), clipboard(clipboard), undoStack(UndoStack(undoStackSize)) {
}

std::pair<std::shared_ptr<CopiedObject>, std::string> LevelPackObjectUseRelationshipListView::copyFrom() {
	//TODO
	return std::pair<std::shared_ptr<CopiedObject>, std::string>();
}

std::string LevelPackObjectUseRelationshipListView::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	//TODO
	return std::string();
}

std::string LevelPackObjectUseRelationshipListView::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	//TODO
	return std::string();
}

bool LevelPackObjectUseRelationshipListView::handleEvent(sf::Event event) {
	//TODO
	return false;
}

void LevelPackObjectUseRelationshipListView::manualCopy() {
	//TODO
}

void LevelPackObjectUseRelationshipListView::manualPaste() {
	//TODO
}

void LevelPackObjectUseRelationshipListView::manualPaste2() {
	//TODO
}

void LevelPackObjectUseRelationshipListView::manualDelete() {
	//TODO
}

void LevelPackObjectUseRelationshipListView::manualSelectAll() {
	//TODO
}

UndoStack& LevelPackObjectUseRelationshipListView::getUndoStack() {
	return undoStack;
}
