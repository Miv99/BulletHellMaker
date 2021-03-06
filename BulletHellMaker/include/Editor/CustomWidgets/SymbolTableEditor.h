#pragma once
#include <vector>
#include <memory>

#include <TGUI/TGUI.hpp>

#include <DataStructs/SymbolTable.h>
#include <DataStructs/UndoStack.h>
#include <Editor/Util/ExtraSignals.h>
#include <Editor/EventCapturable.h>
#include <Editor/CustomWidgets/NumericalEditBoxWithLimits.h>
#include <Editor/CustomWidgets/EditBox.h>
#include <Editor/CustomWidgets/ListView.h>

/*
A widget that allows the user to edit a ValueSymbolTable.

Signals:
	ValueChanged - emitted when the ValueSymbolTable being edited is modified.
		Optional parameter - the ValueSymbolTable
*/
class ValueSymbolTableEditor : public tgui::Panel, public EventCapturable {
public:
	tgui::SignalValueSymbolTable onValueChange = { "ValueChanged" };

	/*
	isTableForTopLevelObject - indicates whether the ValueSymbolTable being edited is for a top-level
		LevelPackObject (Player, Level).
	isTableForObjectInTopOfLayer - only used if isTableForTopLevelObject is false. Indicates whether the
		ValueSymbolTable being edited is for a LevelPackObject at the top-level of its layer 
		(Enemy, EnemyPhase, AttackPattern, Attack) and isn't a top-level object.
		The user will be able to redelegate symbols if and only if this is true.
	*/
	ValueSymbolTableEditor(bool isTableForTopLevelObject, bool isTableForObjectInTopOfLayer, int undoStackSize = 50);
	static std::shared_ptr<ValueSymbolTableEditor> create(bool isTableForTopLevelObject, bool isTableForObjectInTopOfLayer, int undoStackSize = 50) {
		return std::make_shared<ValueSymbolTableEditor>(isTableForTopLevelObject, isTableForObjectInTopOfLayer, undoStackSize);
	}

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(tgui::String signalName) override;

	/*
	Does not emit the ValueChanged signal.
	The last ValueSymbolTable in the vector will be the only ValueSymbolTable that
	can be edited. ValueSymbolTables are in ascending order of importance.
	*/
	void setSymbolTablesHierarchy(std::vector<ValueSymbolTable> symbolTablesHierarchy);

private:
	UndoStack undoStack;

	std::vector<ValueSymbolTable> symbolTablesHierarchy;

	bool canShowParentSymbols;

	std::shared_ptr<tgui::CheckBox> showParentSymbols;
	// ID for each item is in format "[index in symbolTablesHierarchy]|[symbol string]"
	std::shared_ptr<ListView> symbolsList;
	std::shared_ptr<tgui::CheckBox> redelegateCheckBox;
	std::shared_ptr<NumericalEditBoxWithLimits> valueEditBox;

	// The index in symbolsList where the editable portion begins
	int editableIndexBegin;

	bool ignoreSignals = false;

	/*
	Returns whether the add was successful.
	*/
	bool manualAdd(std::string symbol);
	void manualDelete();

	std::string getSymbolFromSymbolsListItemID(std::string id);
};

/*
When inherited, this allows a LevelPackObject editor widget to propagate changes to its LevelPackObject's
ValueSymbolTable to all widgets that require it.

Whenever this editor widget creates a new editor widget that edits a child LevelPackObject,
childWidget.updateSymbolTables(this->symbolTables) should be called.

updateSymbolTables({ }) should be called in the constructor of
this editor widget if the LevelPackObject being edited is at the top of its layer.
*/
class ValueSymbolTablesChangePropagator {
public:
	/*
	Called when any parent LevelPackObjects with a 1:1 relationship
	with the LevelPackObject being edited by the widget inheriting SymbolTablesChangePropagator
	has its ValueSymbolTable modified.

	symbolTables - a list of ValueSymbolTables starting from the root unique LevelPackObject parent
		(eg If the widget inheriting this class is editing an EMPA and the relationship
		is Attack-EMP-EMPA, then symbolTables is { attack.symbolTable, emp.symbolTable })
	*/
	void updateSymbolTables(std::vector<ValueSymbolTable> parentSymbolTables);

protected:
	// See onChange() for explanation
	std::vector<ValueSymbolTable> symbolTables;

	/*
	Should be called when a change is made to the ValueSymbolTable of the LevelPackObject being edited by this widget.
	*/
	void onChange(ValueSymbolTable symbolTable);

	/*
	Should be overriden to call updateSymbolTables(this->symbolTables) on all child widgets that use the ValueSymbolTable
	of the LevelPackObject being edited and call ValueSymbolTableEditor::setSymbolTablesHierarchy(this->symbolTables)
	on the ValueSymbolTableEditor that edits this widget's LevelPackObject's ValueSymbolTable.
	*/
	virtual void propagateChangesToChildren() = 0;

	virtual ValueSymbolTable getLevelPackObjectSymbolTable() = 0;
};

/*
A widget that allows the user to edit a ExprSymbolTable.

Signals:
	ValueChanged - emitted when the ExprSymbolTable being edited is modified.
		Optional parameter - the ExprSymbolTable
*/
class ExprSymbolTableEditor : public tgui::Panel {
public:
	tgui::SignalExprSymbolTable onValueChange = { "ValueChanged" };

	ExprSymbolTableEditor(UndoStack& undoStack);
	static std::shared_ptr<ExprSymbolTableEditor> create(UndoStack& undoStack) {
		return std::make_shared<ExprSymbolTableEditor>(undoStack);
	}

	tgui::Signal& getSignal(tgui::String signalName) override;

	/*
	Sets the ValueSymbolTable hierarchy that can define symbols used in
	the editable ExprSymbolTable's expressions.

	Does not emit the ValueChanged signal.
	ValueSymbolTables are in ascending order of importance.
	These tables will not be editable.
	*/
	void setSymbolTablesHierarchy(std::vector<ValueSymbolTable> symbolTablesHierarchy);

	/*
	Sets the ExprSymbolTable being edited.
	Does not emit the ValueChanged signal.
	*/
	void setExprSymbolTable(ExprSymbolTable symbolTable);

private:
	UndoStack& undoStack;

	std::vector<ValueSymbolTable> symbolTablesHierarchy;
	ExprSymbolTable exprSymbolTable;

	// ID for each item is in format "[index in symbolTablesHierarchy]|[symbol string]"
	// Items from exprSymbolTable have index -1.
	std::shared_ptr<ListView> symbolsList;
	std::shared_ptr<EditBox> valueEditBox;

	// The index in symbolsList where the editable portion begins
	int editableIndexBegin;

	bool ignoreSignals = false;

	/*
	Clears and populates symbolsList depending on symbolTablesHierarchy and exprSymbolTable.
	*/
	void repopulateListView();

	/*
	Returns whether the add was successful.
	*/
	bool manualAdd(std::string symbol);
	void manualDelete();

	std::string getSymbolFromSymbolsListItemID(std::string id);
};