#pragma once
#include <Editor/CopyPaste.h>
#include <Editor/LevelPackObjectUseRelationship/LevelPackObjectUseRelationshipEditor.h>
#include <Editor/CustomWidgets/EditBox.h>
#include <Editor/CustomWidgets/NumericalEditBoxWithLimits.h>
#include <Editor/CustomWidgets/SymbolTableEditor.h>
#include <Editor/Windows/EditorWindowConfirmationPromptChoice.h>
#include <DataStructs/SymbolTable.h>

class AttackPatternToAttackUseRelationship : public LevelPackObjectUseRelationship {
public:
	AttackPatternToAttackUseRelationship(std::tuple<std::string, int, ExprSymbolTable> data);

	void setData(std::tuple<std::string, int, ExprSymbolTable> data);

	std::tuple<std::string, int, ExprSymbolTable> getData();

	static std::vector<std::tuple<std::string, int, ExprSymbolTable>> convertRelationshipVectorToDataVector(std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> relationships);
	static std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> convertDataVectorToRelationshipVector(std::vector<std::tuple<std::string, int, ExprSymbolTable>> data);

private:
	std::tuple<std::string, int, ExprSymbolTable> data;
};

class AttackPatternToAttackUseRelationshipListView : public LevelPackObjectUseRelationshipListView {
public:
	AttackPatternToAttackUseRelationshipListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, UndoStack& undoStack,
		LevelPackObjectUseRelationshipEditor& parentRelationshipEditor);
	static std::shared_ptr<AttackPatternToAttackUseRelationshipListView> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, UndoStack& undoStack,
		LevelPackObjectUseRelationshipEditor& parentRelationshipEditor) {
		return std::make_shared<AttackPatternToAttackUseRelationshipListView>(mainEditorWindow, clipboard, undoStack, parentRelationshipEditor);
	}

	CopyOperationResult copyFrom() override;
	PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

private:
	std::string getRelationshipListViewText(std::shared_ptr<LevelPackObjectUseRelationship> relationship) override;

	void onPasteIntoConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, 
		std::vector<std::tuple<std::string, int, ExprSymbolTable>> newRelationshipsData);
};

/*
Widget used to edit an EditorAttackPattern's usage of EditorAttacks.

Signals:
	RelationshipsModified - emitted whenever the relationships being edited is modified.
		Optional parameter: the new relationships
*/
class AttackPatternToAttackUseRelationshipEditor : public LevelPackObjectUseRelationshipEditor {
public:
	tgui::SignalAttackPatternToAttackUseRelationship onRelationshipsModify = { "RelationshipsModified" };

	AttackPatternToAttackUseRelationshipEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, UndoStack& undoStack,
		std::vector<std::tuple<std::string, int, ExprSymbolTable>> initialRelationshipsData);
	static std::shared_ptr<AttackPatternToAttackUseRelationshipEditor> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, UndoStack& undoStack,
		std::vector<std::tuple<std::string, int, ExprSymbolTable>> initialRelationshipsData) {
		return std::make_shared<AttackPatternToAttackUseRelationshipEditor>(mainEditorWindow, clipboard, undoStack, initialRelationshipsData);
	}

	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	tgui::Signal& getSignal(tgui::String signalName) override;

	void setSymbolTablesHierarchy(std::vector<ValueSymbolTable> symbolTablesHierarchy);

private:
	std::shared_ptr<tgui::Label> timeLabel;
	std::shared_ptr<EditBox> timeEditBox;
	std::shared_ptr<tgui::Label> idLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> idEditBox;
	std::shared_ptr<tgui::Label> symbolTableEditorLabel;
	std::shared_ptr<ExprSymbolTableEditor> symbolTableEditor;

	void initializeRelationshipEditorPanelWidgetsData(std::shared_ptr<LevelPackObjectUseRelationship> relationship, int relationshipIndex) override;
	void onRelationshipsChange(std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> newRelationships) override;
	void instantiateRelationshipListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard) override;
	std::shared_ptr<LevelPackObjectUseRelationship> instantiateDefaultRelationship() override;

	void onPasteIntoConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, 
		std::vector<std::tuple<std::string, int, ExprSymbolTable>> newRelationshipData);
};