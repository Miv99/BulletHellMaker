#include "SymbolTableEditor.h"

ValueSymbolTableEditor::ValueSymbolTableEditor() {
	// TODO
}

bool ValueSymbolTableEditor::handleEvent(sf::Event event) {
	// TODO
	return false;
}

tgui::Signal& ValueSymbolTableEditor::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return tgui::ChildWindow::getSignal(signalName);
}

void SymbolTablesChangePropagator::onChange(std::vector<exprtk::symbol_table<float>> symbolTables) {
	ValueSymbolTable mySymbolTable = getLevelPackObjectSymbolTable();
	if (!mySymbolTable.isEmpty()) {
		symbolTables.push_back(mySymbolTable.toExprtkSymbolTable());
	}
	this->symbolTables = std::vector<exprtk::symbol_table<float>>(symbolTables);
}
