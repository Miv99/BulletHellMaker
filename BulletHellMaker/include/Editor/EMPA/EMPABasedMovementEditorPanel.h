#pragma once
#include <TGUI/TGUI.hpp>

#include <Editor/CustomWidgets/EMPAListVisualizer.h>
#include <Editor/CopyPaste.h>
#include <DataStructs/UndoStack.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <LevelPack/EditorMovablePointAction.h>
#include <Editor/CustomWidgets/SymbolTableEditor.h>
#include <Editor/CustomWidgets/TabsWithPanel.h>

class EMPABasedMovementEditorPanel;

/*
Helper class for EMPABasedMovementEditorPanel.

Panel containing a ListView used to edit a vector of EMPActions.
*/
class EditorMovablePointActionsListView : public ListViewScrollablePanel, public EventCapturable, public CopyPasteable {
public:
	EditorMovablePointActionsListView(EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel, Clipboard& clipboard, int undoStackSize = 50);
	static std::shared_ptr<EditorMovablePointActionsListView> create(EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel, Clipboard& clipboard, int undoStackSize = 50) {
		return std::make_shared<EditorMovablePointActionsListView>(empaBasedMovementEditorPanel, clipboard, undoStackSize);
	}

	CopyOperationResult copyFrom() override;
	PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	UndoStack& getUndoStack();

	CopyOperationResult manualCopy();
	PasteOperationResult manualPaste();
	PasteOperationResult manualPaste2();

private:
	EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel;
	Clipboard& clipboard;
	UndoStack undoStack;
};

/*
Helper class for EMPABasedMovementEditorPanel.

An EventCapturable basic tgui::Panel to be used by EMPABasedMovementEditorPanel for viewing a EditorMovablePointActionsListView.
This widget's main purpose is to pass events down to the child EditorMovablePointActionsListView widget and hold
utility buttons that can't be scrolled past.
*/
class EditorMovablePointActionsListPanel : public tgui::Panel, public EventCapturable {
public:
	EditorMovablePointActionsListPanel(EditorWindow& parentWindow, EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel, Clipboard& clipboard);
	static std::shared_ptr<EditorMovablePointActionsListPanel> create(EditorWindow& parentWindow, EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel, Clipboard& clipboard) {
		return std::make_shared<EditorMovablePointActionsListPanel>(parentWindow, empaBasedMovementEditorPanel, clipboard);
	}

	bool handleEvent(sf::Event event) override;

	std::shared_ptr<EditorMovablePointActionsListView> getEmpasListView();
	std::shared_ptr<tgui::Button> getEMPAAddButton();
	std::shared_ptr<tgui::Button> getEMPADeleteButton();

private:
	EditorWindow& parentWindow;
	Clipboard& clipboard;
	EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel;

	std::shared_ptr<EditorMovablePointActionsListView> empasListView;
	std::shared_ptr<tgui::Button> empaAddButton;
	std::shared_ptr<tgui::Button> empaDeleteButton;
};

/*
Helper class for EMPABasedMovementEditorPanel.

A tgui::Panel used as a tab in EMPABasedMovementEditorPanel to view all EMPAs and their combined movement path.
*/
class EMPAsVisualizerPanel : public tgui::Panel, public EventCapturable {
public:
	EMPAsVisualizerPanel(EditorWindow& parentWindow, EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel, Clipboard& clipboard);
	static std::shared_ptr<EMPAsVisualizerPanel> create(EditorWindow& parentWindow, EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel, Clipboard& clipboard) {
		return std::make_shared<EMPAsVisualizerPanel>(parentWindow, empaBasedMovementEditorPanel, clipboard);
	}

	bool handleEvent(sf::Event event) override;

	void updatePath(std::vector<std::shared_ptr<EMPAction>> empas);

	std::shared_ptr<EditorMovablePointActionsListView> getEmpasListView();
	std::shared_ptr<EditorMovablePointActionsListPanel> getEmpasListPanel();

	/*
	Sets the starting position of the first EMPA in the visualizer.
	If the string cannot be converted to a float, it will default to 0.
	*/
	void setVisualizerStartPosX(std::string startX);
	/*
	Sets the starting position of the first EMPA in the visualizer.
	If the string cannot be converted to a float, it will default to 0.
	*/
	void setVisualizerStartPosY(std::string startY);

private:
	// The left panel
	std::shared_ptr<EditorMovablePointActionsListPanel> empasListPanel;
	// Stores the actual visualizer
	std::shared_ptr<tgui::Panel> mainPanel;
	std::shared_ptr<EMPAListVisualizer> visualizer;
};

/*
A tgui::Panel used to edit a list of EMPActions.

Signals:
	EMPAListModified - emitted when the list of EMPAs is modified
		Optional parameter: a vector of shared_ptrs of EMPActions representing the new actions and the sum of durations of every EMPA as a float
*/
class EMPABasedMovementEditorPanel : public tgui::Panel, public EventCapturable, public ValueSymbolTablesChangePropagator {
public:
	EMPABasedMovementEditorPanel(EditorWindow& parentWindow, Clipboard& clipboard);
	static std::shared_ptr<EMPABasedMovementEditorPanel> create(EditorWindow& parentWindow, Clipboard& clipboard) {
		return std::make_shared<EMPABasedMovementEditorPanel>(parentWindow, clipboard);
	}

	tgui::Signal& getSignal(std::string signalName) override;

	bool handleEvent(sf::Event event) override;

	/*
	Returns the sum of durations of every EMPA in the list being edited.
	*/
	float getSumOfDurations();
	/*
	Set the actions to be edited.
	This will deep-copy every action in the vector and will not cause the EMPAListModified signal to emit.
	*/
	void setActions(std::vector<std::shared_ptr<EMPAction>> actions);
	/*
	Sets the starting position of the first EMPA in the visualizer.
	If the string cannot be converted to a float, it will default to 0.
	*/
	void setVisualizerStartPosX(std::string startX);
	/*
	Sets the starting position of the first EMPA in the visualizer.
	If the string cannot be converted to a float, it will default to 0.
	*/
	void setVisualizerStartPosY(std::string startY);

	/*
	Does the delete command depending on what is selected in the EditorMovablePointActionsListView.
	*/
	void manualDelete();
	/*
	Does the copy command depending on what is selected in the EditorMovablePointActionsListView.
	*/
	CopyOperationResult manualCopy();
	/*
	Does the paste command depending on what is selected in the EditorMovablePointActionsListView.
	*/
	PasteOperationResult manualPaste(std::shared_ptr<CopiedObject> pastedObject);
	/*
	Does the paste2 command depending on what is selected in the EditorMovablePointActionsListView.
	*/
	PasteOperationResult manualPaste2(std::shared_ptr<CopiedObject> pastedObject);
	/*
	Opens a new EMPA tab for the EMPA at some index in actions.
	*/
	void manualOpenEMPA(int index);

	void propagateChangesToChildren() override;
	ValueSymbolTable getLevelPackObjectSymbolTable() override;

private:
	static const std::string EMPA_TAB_NAME_FORMAT;
	// The index in EMPA_TAB_NAME_FORMAT that marks the beginning of the action index.
	// This is used to extract the action index out of a tab name.
	static const int EMPA_TAB_NAME_FORMAT_NUMBER_INDEX;

	tgui::SignalEMPAVectorAndFloat onEMPAListModify = { "EMPAListModified" };

	// The EMPAs list being edited
	std::vector<std::shared_ptr<EMPAction>> actions;
	// Sum of durations in actions
	float sumOfDurations;
	// Maps EditorMovablePointActionPanels to the index of the action it edits
	std::map<std::shared_ptr<tgui::Panel>, int> panelToActionIndexMap;

	EditorWindow& parentWindow;
	Clipboard& clipboard;

	std::shared_ptr<TabsWithPanel> tabs;
	std::shared_ptr<EMPAsVisualizerPanel> visualizer;

	/*
	Create a panel for editing an EMPA.

	empa - the EMPA for which the panel will edit. This will be deep-copied.
	index - the index of empa in actions
	empiActions - the ListBoxScrollablePanel which will display all of the EMPAs
	*/
	std::shared_ptr<tgui::Panel> createEMPAPanel(std::shared_ptr<EMPAction> empa, int index, std::shared_ptr<ListViewScrollablePanel> empiActions);
	/*
	Update the EditorMovablePointActionsListView to match actions.
	Should be called whenever actions is modified.
	Also updates sumOfDurations.

	setSelectedIndices - if true, this will set the selected indices to newSelectedIndices. If false, this will select whatever was
		selected before this function call.
	newSelectedIndices - the new selected indices. Only used if setSelectedIndices is true
	*/
	void updateEMPAList(bool setSelectedIndices = false, std::set<size_t> newSelectedIndices = std::set<size_t>());

	/*
	Returns a deep copy of actions.
	*/
	std::vector<std::shared_ptr<EMPAction>> cloneActions();

	/*
	Reloads an EMPA tab.  If the EMPA no longer exists, the tab will be removed.
	*/
	void reloadEMPATab(int empaIndex);

	/*
	Called when the confirmation popup from doing paste2 is answered.
	*/
	void onPasteIntoConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, std::vector<std::shared_ptr<EMPAction>> newEMPAs);
};