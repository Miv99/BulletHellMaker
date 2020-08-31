#pragma once
#include <TGUI/TGUI.hpp>

#include <GuiConfig.h>
#include <Editor/CustomWidgets/ListViewScrollablePanel.h>
#include <Editor/EditorWindow.h>
#include <Editor/Util/EditorUtils.h>
#include <LevelPack/LevelPack.h>

class LevelPackObjectUseRelationship {
};

class LevelPackObjectUseRelationshipEditor;

/*
A ListViewScrollablePanel of EditorAttacks, for use by MainEditorWindow.
Items should not be added or removed from this with the exception of from reload().
handleEvent() is called from this widget's container, EditorWindow::LevelPackObjectsListPanel.

Deletes in this widget are handled by LevelPackObjectUseRelationshipEditor.

Signals:
	ListModified - emitted whenever the list view is modified internally by the paste or paste2 operation.
*/
class LevelPackObjectUseRelationshipListView : public ListViewScrollablePanel, public EventCapturable, public CopyPasteable {
public:
	/*
	mainEditorWindow - the parent MainEditorWindow using this widget
	clipboard - the parent Clipboard
	*/
	LevelPackObjectUseRelationshipListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, UndoStack& undoStack,
		LevelPackObjectUseRelationshipEditor& parentRelationshipEditor, std::string copyPasteableID, int undoStackSize = 50);
	virtual ~LevelPackObjectUseRelationshipListView();

	bool handleEvent(sf::Event event) override;

	/*
	Does the copy command on this widget.
	*/
	void manualCopy();
	/*
	Does the paste command on this widget.
	*/
	void manualPaste();
	/*
	Does the paste2 command on this widget.
	*/
	void manualPaste2();
	/*
	Does the delete command on this widget.
	*/
	void manualDelete();
	/*
	Does the select all command on this widget.
	*/
	void manualSelectAll();

	/*
	Clears and populates the list view with some relationships
	without emitting the ListModified signal.
	*/
	void repopulateRelationships(std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> relationships);

	tgui::Signal& getSignal(std::string signalName) override;

protected:
	MainEditorWindow& mainEditorWindow;
	LevelPackObjectUseRelationshipEditor& parentRelationshipEditor;
	UndoStack& undoStack;
	Clipboard& clipboard;

	tgui::Signal onListModify = { "ListModified" };

	/*
	Returns the text for an entry in this list view corresponding to a relationship.
	*/
	virtual std::string getRelationshipListViewText(std::shared_ptr<LevelPackObjectUseRelationship> relationship) = 0;
};

/*
A widget used to edit how an object uses LevelPackObjects.

Due to limitations with calling virtual functions in base class constructors,
setupRelationshipListView() and then relationshipsListView->repopulateRelationships() 
should be called in subclasses' constructors.
*/
class LevelPackObjectUseRelationshipEditor : public tgui::Group, public EventCapturable, public CopyPasteable {
public:
	/*
	enableMultipleRelationships - whether to allow the user to edit multiple relationships using relationshipsListView
	*/
	LevelPackObjectUseRelationshipEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, UndoStack& undoStack,
		std::string copyPasteableID, bool enableMultipleRelationships);
	virtual ~LevelPackObjectUseRelationshipEditor();

	virtual bool handleEvent(sf::Event event) override;

	std::pair<std::shared_ptr<CopiedObject>, std::string> copyFrom() override;
	std::string pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;

	/*
	Deletes the selected items in relationshipsListView.
	*/
	void deleteSelectedListItems();

	/*
	Inserts new relationships at some index.
	Does not call onRelationshipsChange().
	*/
	void insertRelationships(int insertAt, std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> insertedRelationships);

	/*
	Deletes relationships by indices.
	Does not call onRelationshipsChange().

	indices - must be sorted in ascending order
	*/
	void deleteRelationships(std::set<size_t> indices);

	/*
	Replaces a subset of relationships with new ones.
	Does not call onRelationshipsChange().

	indices and replacements must be the same size and correspond to each other such that
	each relationships[indices[i]] is replaced by replacements[i].
	*/
	void replaceRelationships(std::set<size_t> indices, std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> replacements);

	/*
	Returns a subset of relationships.
	*/
	std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> getRelationshipsSubset(std::set<size_t> indices);

	/*
	Returns all relationships.
	*/
	std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> getRelationships();

	/*
	Returns the total number of relationships.
	*/
	int getRelationshipsCount();

protected:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	UndoStack& undoStack;

	// These two widgets are instantiated only if enableMultipleRelationships was true in this widget's constructor.
	// Panel to hold relationshipsListView
	std::shared_ptr<tgui::Panel> relationshipsListViewPanel;
	std::shared_ptr<LevelPackObjectUseRelationshipListView> relationshipsListView;

	// The index in relationships that is currently being edited by relationshipEditorPanel
	int relationshipEditorPanelCurrentRelationshipIndex = -1;

	// Main panel containing widgets necessary to edit a single relationship
	std::shared_ptr<tgui::Panel> relationshipEditorPanel;

	bool ignoreSignals = false;

	/*
	Sets the values of widgets in relationshipEditorPanel to correspond to
	the parameter relationship.

	Make sure to set ignoreSignals to true while setting widget values.
	*/
	virtual void initializeRelationshipEditorPanelWidgetsData(std::shared_ptr<LevelPackObjectUseRelationship> relationship, int relationshipIndex) = 0;

	/*
	Called whenever the list of relationships is changed.

	newRelationships - the new relationships
	*/
	virtual void onRelationshipsChange(std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> newRelationships) = 0;

	/*
	Called to instantiate relationshipsListView.
	Doesn't have to be overriden if multiple relationships aren't allowed.
	*/
	virtual void instantiateRelationshipListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard) = 0;

	/*
	Called to instantiate a new default relationship.
	Doesn't have to be overriden if multiple relationships aren't allowed.
	*/
	virtual std::shared_ptr<LevelPackObjectUseRelationship> instantiateDefaultRelationship();

	/*
	Should be called by widgets inheriting this class whenever the relationship being edited by
	relationshipEditorPanel is modified.
	*/
	void onRelationshipEditorPanelRelationshipModify(std::shared_ptr<LevelPackObjectUseRelationship> newRelationship);

	/*
	Sets up the relationship list view.

	Should be called in the constructor of subclasses, since it calls a virtual method and so cannot be called
	in the base class's constructor.
	*/
	void setupRelationshipListView();

	/*
	Opens the relationship editor panel associated with some index in the list of relationships.
	*/
	void openRelationshipEditorPanelIndex(int index);

	/*
	Returns the relationship that is currently being edited by relationshipEditorPanel.
	*/
	std::shared_ptr<LevelPackObjectUseRelationship> getCurrentlySelectedRelationship();

	/*
	Helper function to call onRelationshipsChange();
	*/
	void onRelationshipsChangeHelper();

private:
	std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> relationships;

	/*
	Adds a new relationship based on the currently selected index in the list view.
	*/
	void addNewRelationship();

	/*
	Helper function to safely insert to relationships.
	*/
	void relationshipsInsert(int index, std::shared_ptr<LevelPackObjectUseRelationship> item);

	/*
	Helper function to safely erase from relationships.
	*/
	void relationshipsErase(int index);
};