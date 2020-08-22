#pragma once
#include <Editor/Attack/AttackEditorPanel.h>

/*
Empty panel that captures undo/redo/copy/paste commands and whose purpose is for
AttackEditorPanel to show the EMP tree of an EditorAttack.
Should be used only by AttackEditorPanel.

Signals:
EMPModified - emitted when the an EMP belonging to the EditorAttack being edited is modified by this widget.
	Optional parameter - a shared_ptr to the modified EMP. This should only be used to reload EMP tabs, so this
		will be nullptr if the tab doesn't need to be reloaded.
MainEMPChildDeleted - emitted when some child (but not necessarily direct child) EMP of the mainEMP of the EditorAttack
	being edited is deleted. The EMPModified signal will be emitted right before this one is.
	Optional parameter: the ID of the EMP that was deleted.
*/
class EditorMovablePointTreePanel : public tgui::Panel, public EventCapturable, public CopyPasteable {
public:
	/*
	parentAttackEditorPanel - the AttackEditorPanel this widget is a child of
	clipboard - the parent Clipboard
	attack - the EditorAttack whose EMP tree is being viewed
	*/
	EditorMovablePointTreePanel(AttackEditorPanel& parentAttackEditorPanel, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50);
	static std::shared_ptr<EditorMovablePointTreePanel> create(AttackEditorPanel& parentAttackEditorPanel, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50) {
		return std::make_shared<EditorMovablePointTreePanel>(parentAttackEditorPanel, clipboard, attack, undoStackSize);
	}

	std::pair<std::shared_ptr<CopiedObject>, std::string> copyFrom() override;
	std::string pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	std::string paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	void manualCopy();
	void manualPaste();
	void manualPaste2();
	void manualDelete();

	tgui::Signal& getSignal(std::string signalName) override;
	std::shared_ptr<tgui::TreeView> getEmpsTreeView();

private:
	AttackEditorPanel& parentAttackEditorPanel;
	UndoStack undoStack;
	Clipboard& clipboard;
	std::shared_ptr<EditorAttack> attack;
	std::shared_ptr<tgui::TreeView> empsTreeView;

	tgui::SignalEditorMovablePoint onEMPModify = { "EMPModified" };
	tgui::SignalInt onMainEMPChildDeletion = { "MainEMPChildDeleted" };

	void manualUndo();
	void manualRedo();
};