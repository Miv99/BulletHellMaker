#pragma once
#include "LevelPack.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"
#include "EditorMovablePointSpawnType.h"
#include "EventCapturable.h"
#include "UndoStack.h"
#include <TGUI/TGUI.hpp>

class EditorMovablePointPanel : public tgui::Panel, public EventCapturable {
public:

	bool handleEvent(sf::Event event) override;

private:

};