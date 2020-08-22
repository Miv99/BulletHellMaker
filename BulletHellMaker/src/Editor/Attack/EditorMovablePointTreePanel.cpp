#include <Editor/Attack/EditorMovablePointTreePanel.h>

#include <GuiConfig.h>

EditorMovablePointTreePanel::EditorMovablePointTreePanel(AttackEditorPanel& parentAttackEditorPanel, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize) : CopyPasteable("EditorMovablePoint"), parentAttackEditorPanel(parentAttackEditorPanel), clipboard(clipboard),
attack(attack), undoStack(UndoStack(undoStackSize)) {
	empsTreeView = tgui::TreeView::create();
	std::shared_ptr<tgui::Label> empsTreeViewLabel = tgui::Label::create();

	empsTreeViewLabel->setText("Movable points");

	empsTreeViewLabel->setTextSize(TEXT_SIZE);
	empsTreeView->setTextSize(TEXT_SIZE);

	empsTreeViewLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	empsTreeView->setPosition(tgui::bindLeft(empsTreeViewLabel), tgui::bindBottom(empsTreeViewLabel) + GUI_LABEL_PADDING_Y);
	connect("SizeChanged", [&](sf::Vector2f newSize) {
		empsTreeView->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - tgui::bindTop(empsTreeView) - GUI_PADDING_Y);
	});

	add(empsTreeViewLabel);
	add(empsTreeView);
}

std::pair<std::shared_ptr<CopiedObject>, std::string> EditorMovablePointTreePanel::copyFrom() {
	auto selected = empsTreeView->getSelectedItem();
	if (selected.size() > 0) {
		return std::make_pair(std::make_shared<CopiedEditorMovablePoint>(getID(), attack->searchEMP(parentAttackEditorPanel.getEMPIDFromTreeViewText(selected[selected.size() - 1]))), "Copied 1 movable point");
	}
	return std::make_pair(nullptr, "");
}

std::string EditorMovablePointTreePanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	auto derived = std::static_pointer_cast<CopiedEditorMovablePoint>(pastedObject);
	if (derived) {
		auto selected = empsTreeView->getSelectedItem();
		if (selected.size() > 0) {
			std::shared_ptr<EditorMovablePoint> emp = derived->getEMP();
			emp->onNewParentEditorAttack(attack);

			std::shared_ptr<EditorMovablePoint> selectedEMP = attack->searchEMP(parentAttackEditorPanel.getEMPIDFromTreeViewText(selected[selected.size() - 1]));

			undoStack.execute(UndoableCommand([this, emp, selectedEMP]() {
				// Add the copied EMP as a child of the selected EMP
				selectedEMP->addChild(emp);

				onEMPModify.emit(this, emp);
			}, [this, emp, selectedEMP]() {
				selectedEMP->removeChild(emp->getID());

				onEMPModify.emit(this, emp);
				// Emit onMainEMPChildDeletion for the deleted EMP and all its children
				onMainEMPChildDeletion.emit(this, emp->getID());
				std::vector<int> deletedEmpIDs = emp->getChildrenIDs();
				for (int deletedID : deletedEmpIDs) {
					onMainEMPChildDeletion.emit(this, deletedID);
				}
			}));

			return "Pasted 1 movable point";
		}
	}
	return "";
}

std::string EditorMovablePointTreePanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	auto derived = std::static_pointer_cast<CopiedEditorMovablePoint>(pastedObject);
	if (derived) {
		auto selected = empsTreeView->getSelectedItem();
		if (selected.size() > 0) {
			// Overwrite the selected EMP with the copied EMP but keep the ID

			std::shared_ptr<EditorMovablePoint> selectedEMP = attack->searchEMP(parentAttackEditorPanel.getEMPIDFromTreeViewText(selected[selected.size() - 1]));
			std::string selectedEMPOldFormat = selectedEMP->format();

			undoStack.execute(UndoableCommand([this, derived, selectedEMP]() {
				std::shared_ptr<EditorMovablePoint> pastedEMP = derived->getEMP();

				// Gather the ID of every EMP that will be deleted
				std::vector<int> deletedEmpIDs = selectedEMP->getChildrenIDs();

				// Do the actual pasting
				int oldID = selectedEMP->getID();
				selectedEMP->load(pastedEMP->format());
				selectedEMP->setID(oldID);

				onEMPModify.emit(this, selectedEMP);
				// Emit onMainEMPChildDeletion for every deleted EMP
				for (int deletedID : deletedEmpIDs) {
					onMainEMPChildDeletion.emit(this, deletedID);
				}
			}, [this, selectedEMP, selectedEMPOldFormat]() {
				// Gather the ID of every EMP that will be deleted
				std::vector<int> deletedEmpIDs = selectedEMP->getChildrenIDs();

				int oldID = selectedEMP->getID();
				selectedEMP->load(selectedEMPOldFormat);
				selectedEMP->setID(oldID);

				onEMPModify.emit(this, selectedEMP);
				// Emit onMainEMPChildDeletion for every deleted EMP
				for (int deletedID : deletedEmpIDs) {
					onMainEMPChildDeletion.emit(this, deletedID);
				}
			}));

			return "Replaced 1 movable point";
		}
	}
	return "";
}

bool EditorMovablePointTreePanel::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				manualUndo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				manualRedo();
				return true;
			} else if (event.key.code == sf::Keyboard::C) {
				manualCopy();
				return true;
			} else if (event.key.code == sf::Keyboard::V) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
					manualPaste2();
				} else {
					manualPaste();
				}
				return true;
			}
		}
		if (event.key.code == sf::Keyboard::Delete) {
			manualDelete();
		}
	}
	return false;
}

tgui::Signal& EditorMovablePointTreePanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onEMPModify.getName())) {
		return onEMPModify;
	} else if (signalName == tgui::toLower(onMainEMPChildDeletion.getName())) {
		return onMainEMPChildDeletion;
	}
	return tgui::Panel::getSignal(signalName);
}

std::shared_ptr<tgui::TreeView> EditorMovablePointTreePanel::getEmpsTreeView() {
	return empsTreeView;
}

void EditorMovablePointTreePanel::manualDelete() {
	auto selected = empsTreeView->getSelectedItem();
	// The hierarchy size must be > 1 because a size of 1 means
	// the main EMP is selected, which should not be able to be deleted
	if (selected.size() > 1) {
		std::shared_ptr<EditorMovablePoint> removedEMP = attack->searchEMP(AttackEditorPanel::getEMPIDFromTreeViewText(selected[selected.size() - 1]));
		std::shared_ptr<EditorMovablePoint> parentEMP = attack->searchEMP(AttackEditorPanel::getEMPIDFromTreeViewText(selected[selected.size() - 2]));

		undoStack.execute(UndoableCommand([this, removedEMP, parentEMP]() {
			// Remove the EMP from attack
			int empID = removedEMP->getID();
			parentEMP->removeChild(empID);

			// Tab deletion will be taken care of by the onMainEMPChildDeletion signal, so no need to reload any tabs with the onEMPModify signal
			onEMPModify.emit(this, nullptr);
			// Emit onMainEMPChildDeletion for the deleted EMP and all its children
			onMainEMPChildDeletion.emit(this, empID);
			std::vector<int> deletedEmpIDs = removedEMP->getChildrenIDs();
			for (int deletedID : deletedEmpIDs) {
				onMainEMPChildDeletion.emit(this, deletedID);
			}
		}, [this, selected, removedEMP, empParentID = parentEMP->getID()]() {
			// Add removedEMP back
			int empParentID = AttackEditorPanel::getEMPIDFromTreeViewText(selected[selected.size() - 2]);
			attack->searchEMP(empParentID)->addChild(removedEMP);

			// An EMP is being added back, so no need to reload any tabs in the parent AttackEditorPanel
			onEMPModify.emit(this, nullptr);
		}));
	}
}

void EditorMovablePointTreePanel::manualUndo() {
	undoStack.undo();
}

void EditorMovablePointTreePanel::manualRedo() {
	undoStack.redo();
}

void EditorMovablePointTreePanel::manualCopy() {
	clipboard.copy(this);
}

void EditorMovablePointTreePanel::manualPaste() {
	clipboard.paste(this);
}

void EditorMovablePointTreePanel::manualPaste2() {
	clipboard.paste2(this);
}