#include <Editor/CustomWidgets/SymbolTableEditor.h>

#include <GuiConfig.h>
#include <Util/StringUtils.h>
#include <Editor/Util/EditorUtils.h>
#include <Editor/CustomWidgets/EditBox.h>

ValueSymbolTableEditor::ValueSymbolTableEditor(bool isTableForTopLevelObject, bool isTableForObjectInTopOfLayer, int undoStackSize) : undoStack(UndoStack(undoStackSize)) {
	canShowParentSymbols = !isTableForTopLevelObject && !isTableForObjectInTopOfLayer;
	bool canRedelegate = !isTableForTopLevelObject && isTableForObjectInTopOfLayer;

	std::shared_ptr<tgui::Button> addSymbol = tgui::Button::create();
	std::shared_ptr<tgui::Button> deleteSymbol = tgui::Button::create();
	std::shared_ptr<EditBox> symbolNameChooser = EditBox::create();
	symbolsList = ListBoxScrollablePanel::create();
	redelegateCheckBox = tgui::CheckBox::create();
	std::shared_ptr<tgui::Label> valueEditBoxLabel = tgui::Label::create();
	valueEditBox = NumericalEditBoxWithLimits::create();

	redelegateCheckBox->setVisible(canRedelegate);

	symbolsList->setTextSize(TEXT_SIZE);
	valueEditBoxLabel->setTextSize(TEXT_SIZE);
	valueEditBox->setTextSize(TEXT_SIZE);
	redelegateCheckBox->setTextSize(TEXT_SIZE);
	symbolNameChooser->setTextSize(TEXT_SIZE);

	valueEditBoxLabel->setText("Value");
	addSymbol->setText("+");
	deleteSymbol->setText("-");
	redelegateCheckBox->setText("Redelegate");

	addSymbol->setToolTip(createToolTip("Adds a variable."));
	deleteSymbol->setToolTip(createToolTip("Deletes the selected variable."));
	symbolNameChooser->setToolTip(createToolTip("The name of the variable to be added."));
	symbolsList->setToolTip(createToolTip("The list of variables. Variables inherited from parents are marked with [P]. If there are multiple of the same variable, \
higher priority is given to the most recently defined variable starting from the root of the object hierarchy. \
For example, let the object hierarchy be \"attack > movable point > movable point action\" and let variable x be defined in \
both the attack and a movable point in the attack. When x is used in the movable point, that x will be the value as defined by the movable point's x. \
When x is used in the attack, that x will be the value \
as defined by the attack's x. When x is used in the movable point action, that x will be the value as defined by the movable point's x because the movable point \
is closer in the object hierarchy to the movable point action than the attack is. Note that this relationship is one-way; an object cannot use variables defined \
by its children objects. \n\n\
Variables with value \"REDELEGATED\" \
must be defined on a case-by-case basis by objects that use the object that is redelegating its variables. \
Only enemies, enemy phases, attack patterns, and attacks can redelegate its variables."));
	redelegateCheckBox->setToolTip(createToolTip("Redelegates a variable so that it must be defined on a case-by-case basis by objects that use \
this object."));
	valueEditBoxLabel->setToolTip(createToolTip("The value of this variable. Unused if the variable is redelegated."));

	redelegateCheckBox->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	addSymbol->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	deleteSymbol->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	symbolNameChooser->setSize(tgui::bindWidth(symbolsList) - SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	connect("SizeChanged", [this, deleteSymbol, symbolNameChooser](sf::Vector2f newSize) {
		symbolsList->setSize(newSize.x / 2.0f - GUI_PADDING_X * 2, newSize.y - deleteSymbol->getPosition().y - deleteSymbol->getSize().y - symbolNameChooser ->getSize().y - GUI_PADDING_Y);
		valueEditBox->setSize(newSize.x / 2.0f - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	});

	symbolsList->getListBox()->connect("ItemSelected", [this, valueEditBoxLabel, canRedelegate](int index) {
		if (ignoreSignals) {
			return;
		}

		if (index >= 0 && index >= editableIndexBegin) {
			ValueSymbolDefinition definition = symbolTablesHierarchy[symbolTablesHierarchy.size() - 1]
				.getSymbolDefinition(getSymbolFromSymbolsListItemID(symbolsList->getListBox()->getSelectedItemId()));
			redelegateCheckBox->setChecked(definition.redelegated);
			valueEditBox->setValue(definition.value);

			redelegateCheckBox->setVisible(canRedelegate);
			valueEditBoxLabel->setVisible(true);
			valueEditBox->setVisible(true);

			valueEditBox->setEnabled(!definition.redelegated);
		} else {
			redelegateCheckBox->setVisible(false);
			valueEditBoxLabel->setVisible(false);
			valueEditBox->setVisible(false);
		}
	});
	redelegateCheckBox->connect("Changed", [this](bool checked) {
		valueEditBox->setEnabled(!checked);

		if (ignoreSignals) {
			return;
		}

		std::string id = symbolsList->getListBox()->getSelectedItemId();
		std::string symbol = getSymbolFromSymbolsListItemID(id);
		undoStack.execute(UndoableCommand(
			[this, symbol, id, checked]() {
			ValueSymbolDefinition definition = symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].getSymbolDefinition(symbol);
			symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].setSymbol(symbol, definition.value, checked);

			ignoreSignals = true;
			redelegateCheckBox->setChecked(checked);
			ignoreSignals = false;

			// Reload symbolsList
			setSymbolTablesHierarchy(symbolTablesHierarchy);

			onValueChange.emit(this, symbolTablesHierarchy[symbolTablesHierarchy.size() - 1]);
		},
			[this, symbol, id, checked]() {
			ValueSymbolDefinition definition = symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].getSymbolDefinition(symbol);
			symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].setSymbol(symbol, definition.value, !checked);

			ignoreSignals = true;
			redelegateCheckBox->setChecked(!checked);
			ignoreSignals = false;

			// Reload symbolsList
			setSymbolTablesHierarchy(symbolTablesHierarchy);

			onValueChange.emit(this, symbolTablesHierarchy[symbolTablesHierarchy.size() - 1]);
		}));
	});
	valueEditBox->connect("ValueChanged", [this](float value) {
		if (ignoreSignals) {
			return;
		}

		std::string id = symbolsList->getListBox()->getSelectedItemId();
		std::string symbol = getSymbolFromSymbolsListItemID(id);
		float oldValue = symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].getSymbolDefinition(symbol).value;
		if (oldValue == value) {
			return;
		}
		undoStack.execute(UndoableCommand(
			[this, symbol, id, value]() {
			ValueSymbolDefinition definition = symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].getSymbolDefinition(symbol);
			symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].setSymbol(symbol, value, definition.redelegated);

			ignoreSignals = true;
			valueEditBox->setValue(value);
			ignoreSignals = false;

			// Reload symbolsList
			setSymbolTablesHierarchy(symbolTablesHierarchy);

			onValueChange.emit(this, symbolTablesHierarchy[symbolTablesHierarchy.size() - 1]);
		},
			[this, symbol, id, oldValue]() {
			ValueSymbolDefinition definition = symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].getSymbolDefinition(symbol);
			symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].setSymbol(symbol, oldValue, definition.redelegated);

			ignoreSignals = true;
			valueEditBox->setValue(oldValue);
			ignoreSignals = false;

			// Reload symbolsList
			setSymbolTablesHierarchy(symbolTablesHierarchy);

			onValueChange.emit(this, symbolTablesHierarchy[symbolTablesHierarchy.size() - 1]);
		}));
	});
	symbolNameChooser->connect("ReturnKeyPressed", [this, symbolNameChooser]() {
		if (manualAdd(symbolNameChooser->getText())) {
			symbolNameChooser->setText("");
		}
	});
	addSymbol->connect("Pressed", [this, symbolNameChooser]() {
		if (manualAdd(symbolNameChooser->getText())) {
			symbolNameChooser->setText("");
		}
	});
	deleteSymbol->connect("Pressed", [this]() {
		manualDelete();
	});

	redelegateCheckBox->setVisible(false);
	valueEditBoxLabel->setVisible(false);
	valueEditBox->setVisible(false);

	add(addSymbol);
	add(deleteSymbol);
	add(symbolNameChooser);
	add(symbolsList);
	add(redelegateCheckBox);
	add(valueEditBoxLabel);
	add(valueEditBox);

	deleteSymbol->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	symbolsList->setPosition(tgui::bindLeft(deleteSymbol), tgui::bindBottom(deleteSymbol));
	symbolNameChooser->setPosition(tgui::bindLeft(symbolsList), tgui::bindBottom(symbolsList));
	addSymbol->setPosition(tgui::bindRight(symbolNameChooser), tgui::bindTop(symbolNameChooser));
	redelegateCheckBox->setPosition(tgui::bindRight(symbolsList) + GUI_PADDING_X, tgui::bindTop(symbolsList));
	valueEditBoxLabel->setPosition(tgui::bindLeft(redelegateCheckBox), tgui::bindBottom(redelegateCheckBox) + GUI_PADDING_Y);
	valueEditBox->setPosition(tgui::bindLeft(valueEditBoxLabel), tgui::bindBottom(valueEditBoxLabel) + GUI_LABEL_PADDING_Y);

	if (canShowParentSymbols) {
		showParentSymbols = tgui::CheckBox::create();
		showParentSymbols->setTextSize(TEXT_SIZE);
		showParentSymbols->setText("Show resolved variables from parents");

		showParentSymbols->connect("Changed", [this](bool checked) {
			// Reload symbolsList
			setSymbolTablesHierarchy(symbolTablesHierarchy);
		});

		add(showParentSymbols);

		showParentSymbols->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
		deleteSymbol->setPosition(tgui::bindLeft(showParentSymbols), tgui::bindBottom(showParentSymbols) + GUI_PADDING_Y);
	}
}

bool ValueSymbolTableEditor::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				undoStack.undo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				undoStack.redo();
				return true;
			}
		}
	}
	return false;
}

tgui::Signal& ValueSymbolTableEditor::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return tgui::Panel::getSignal(signalName);
}

void ValueSymbolTableEditor::setSymbolTablesHierarchy(std::vector<ValueSymbolTable> symbolTablesHierarchy) {
	this->symbolTablesHierarchy = symbolTablesHierarchy;

	auto listBox = symbolsList->getListBox();
	std::string oldSelected = listBox->getSelectedItemId();

	ignoreSignals = true;
	listBox->removeAllItems();
	editableIndexBegin = 0;
	if (canShowParentSymbols && showParentSymbols->isChecked()) {
		for (int i = 0; i < symbolTablesHierarchy.size() - 1; i++) {
			for (auto it = symbolTablesHierarchy[i].getIteratorBegin(); it != symbolTablesHierarchy[i].getIteratorEnd(); it++) {
				std::pair<std::string, ValueSymbolDefinition> pair = *it;
				if (pair.second.redelegated) {
					listBox->addItem("[P] " + pair.first + " = REDELEGATED", std::to_string(i) + "|" + pair.first);
				} else {
					listBox->addItem("[P] " + pair.first + " = " + formatNum(pair.second.value), std::to_string(i) + "|" + pair.first);
				}
				editableIndexBegin++;
			}
		}
	}
	for (auto it = symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].getIteratorBegin(); it != symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].getIteratorEnd(); it++) {
		std::pair<std::string, ValueSymbolDefinition> pair = *it;
		if (pair.second.redelegated) {
			listBox->addItem(pair.first + " = REDELEGATED", std::to_string(symbolTablesHierarchy.size() - 1) + "|" + pair.first);
		} else {
			listBox->addItem(pair.first + " = " + formatNum(pair.second.value), std::to_string(symbolTablesHierarchy.size() - 1) + "|" + pair.first);
		}
	}

	if (oldSelected != "" && listBox->getItemById(oldSelected) != "") {
		listBox->setSelectedItemById(oldSelected);
	} else {
		listBox->deselectItem();
	}
	ignoreSignals = false;
}

bool ValueSymbolTableEditor::manualAdd(std::string symbol) {
	// TODO: also check if symbol is a valid symbol name (no spaces, no special strings like sin)
	bool success = !symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].hasSymbol(symbol);
	if (!success) {
		return false;
	}
	std::string id = std::to_string(symbolTablesHierarchy.size() - 1) + "|" + symbol;
	undoStack.execute(UndoableCommand(
		[this, symbol, id]() {
		symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].setSymbol(symbol, 0, false);

		// Reload symbolsList
		setSymbolTablesHierarchy(symbolTablesHierarchy);

		onValueChange.emit(this, symbolTablesHierarchy[symbolTablesHierarchy.size() - 1]);
		
	},
		[this, symbol, id]() {
		symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].removeSymbol(symbol);
		if (getSymbolFromSymbolsListItemID(symbolsList->getListBox()->getSelectedItemId()) == symbol) {
			symbolsList->getListBox()->deselectItem();
		}

		// Reload symbolsList
		setSymbolTablesHierarchy(symbolTablesHierarchy);

		onValueChange.emit(this, symbolTablesHierarchy[symbolTablesHierarchy.size() - 1]);
	}));
	return true;
}

void ValueSymbolTableEditor::manualDelete() {
	std::string id = symbolsList->getListBox()->getSelectedItemId();
	if (id != "") {
		std::string symbol = getSymbolFromSymbolsListItemID(symbolsList->getListBox()->getSelectedItemId());
		ValueSymbolDefinition oldValue = symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].getSymbolDefinition(symbol);
		undoStack.execute(UndoableCommand(
			[this, id, symbol]() {
			symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].removeSymbol(symbol);
			if (getSymbolFromSymbolsListItemID(symbolsList->getListBox()->getSelectedItemId()) == symbol) {
				symbolsList->getListBox()->deselectItem();
			}

			// Reload symbolsList
			setSymbolTablesHierarchy(symbolTablesHierarchy);

			onValueChange.emit(this, symbolTablesHierarchy[symbolTablesHierarchy.size() - 1]);
		},
			[this, id, symbol, oldValue]() {
			symbolTablesHierarchy[symbolTablesHierarchy.size() - 1].setSymbol(symbol, oldValue.value, oldValue.redelegated);

			// Reload symbolsList
			setSymbolTablesHierarchy(symbolTablesHierarchy);

			onValueChange.emit(this, symbolTablesHierarchy[symbolTablesHierarchy.size() - 1]);
		}));
	}
}

std::string ValueSymbolTableEditor::getSymbolFromSymbolsListItemID(std::string id) {
	// See setSymbolTablesHierarchy()/manualAdd()/manualDelete() for how id is formatted
	return id.substr(id.find_first_of('|') + 1);
}

void ValueSymbolTablesChangePropagator::updateSymbolTables(std::vector<ValueSymbolTable> parentSymbolTables) {
	ValueSymbolTable mySymbolTable = getLevelPackObjectSymbolTable();
	parentSymbolTables.push_back(mySymbolTable);
	this->symbolTables = std::vector<ValueSymbolTable>(parentSymbolTables);

	propagateChangesToChildren();
}

void ValueSymbolTablesChangePropagator::onChange(ValueSymbolTable symbolTable) {
	// The last ValueSymbolTable in symbolTables is the ValueSymbolTable of the LevelPackObject 
	// being edited by this widget, so update it
	symbolTables[symbolTables.size() - 1] = symbolTable;

	propagateChangesToChildren();
}
