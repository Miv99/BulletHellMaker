#pragma once
#include "LevelPack.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"
#include "EditorMovablePointSpawnType.h"
#include "EventCapturable.h"
#include "EditorWindow.h"
#include "UndoStack.h"
#include "ExtraSignals.h"
#include <TGUI/TGUI.hpp>

class EditorMovablePointPanel : public tgui::Panel, public EventCapturable {
public:
	/*
	parentWindow - the top level EditorWindow this widget belongs to
	levelPack - the LevelPack that emp is in
	emp - the EditorMovablePoint being edited
	*/
	EditorMovablePointPanel(EditorWindow& parentWindow, LevelPack& levelPack, std::shared_ptr<EditorMovablePoint> emp);
	inline static std::shared_ptr<EditorMovablePointPanel> create(EditorWindow& parentWindow, LevelPack& levelPack, std::shared_ptr<EditorMovablePoint> emp) {
		return std::make_shared<EditorMovablePointPanel>(parentWindow, levelPack, emp);
	}

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

private:
	EditorWindow& parentWindow;
	LevelPack& levelPack;
	std::shared_ptr<EditorMovablePoint> emp;

	std::shared_ptr<TabsWithPanel> tabs;

	/*
	Signal emitted when the EMP being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorMovablePoint
	*/
	tgui::SignalEditorMovablePoint onEMPModify = { "EMPModified" };
};