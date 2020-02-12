#pragma once
#include "LevelPack.h"
#include "Attack.h"
#include "EventCapturable.h"
#include "UndoStack.h"
#include "EditorUtilities.h"
#include "EditorWindow.h"
#include "EditorMovablePointPanel.h"
#include <TGUI/TGUI.hpp>
#include <memory>

class AttackEditorPanel : public tgui::Panel, public EventCapturable {
public:
	/*
	parentWindow - the EditorWindow this widget is in
	attack - the EditorAttack that is being edited
	undoStackSize - the maximum number of undos stored
	*/
	AttackEditorPanel(EditorWindow& parentWindow, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50);
	static std::shared_ptr<AttackEditorPanel> create(EditorWindow& parentWindow, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50) {
		return std::make_shared<AttackEditorPanel>(parentWindow, attack, undoStackSize);
	}

	bool handleEvent(sf::Event event) override;

private:
	UndoStack undoStack;

	std::shared_ptr<EditorAttack> attack;

	std::shared_ptr<TabsWithPanel> tabs;
};