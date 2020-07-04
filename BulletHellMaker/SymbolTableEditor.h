#pragma once
#include <vector>
#include <TGUI/TGUI.hpp>
#include <memory>
#include "SymbolTable.h"
#include "ExtraSignals.h"
#include "EventCapturable.h"

/*
A widget that allows the user to edit a ValueSymbolTable.

Signals:
	ValueChanged - emitted when the ValueSymbolTable being edited is modified.
		Optional parameter - the ValueSymbolTable
*/
class ValueSymbolTableEditor : tgui::ChildWindow, public EventCapturable {
public:
	ValueSymbolTableEditor();
	static std::shared_ptr<ValueSymbolTableEditor> create() {
		return std::make_shared<ValueSymbolTableEditor>();
	}

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

private:
	tgui::SignalValueSymbolTable onValueChange = { "ValueChanged" };
};

class SymbolTablesChangePropagator {
public:


protected:
	// See onChange() for explanation
	std::vector<exprtk::symbol_table<float>> symbolTables;

	/*
	Called when any parent LevelPackObjects with a 1:1 relationship
	with the LevelPackObject being edited by the widget inheriting SymbolTablesChangePropagator
	has its ValueSymbolTable modified. Should also be called whenever a change is made to the ValueSymbolTable
	of the LevelPackObject being edited by this widget.

	In this function, you should call super(), set the values/configurations for any widget that require
	concrete numbers then call onChange() for all child widgets that inherit SymbolTablesChangePropagator.

	symbolTables - a list of symbol_tables starting from the root unique LevelPackObject parent
		(eg If the widget inheriting this class is editing an EMPA and the relationship
		is Attack-EMP-EMPA, then symbolTables is { attack.symbolTable, emp.symbolTable })
	*/
	virtual void onChange(std::vector<exprtk::symbol_table<float>> symbolTables);

	virtual ValueSymbolTable getLevelPackObjectSymbolTable() = 0;
};