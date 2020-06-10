#pragma once
#include "EventCapturable.h"
#include "EditorWindow.h"
#include "CopyPaste.h"
#include "LevelPack.h"

/*
An EventCapturable basic tgui::Panel to be used by MainEditorWindow for viewing a LevelPackObjectsListView.
This widget's main purpose is to pass events down to the child LevelPackObjectsListView widget.
*/
class LevelPackObjectsListPanel : public tgui::Panel, public EventCapturable {
public:
	/*
	mainEditorWindow - the parent MainEditorWindow
	*/
	LevelPackObjectsListPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard);
	static std::shared_ptr<LevelPackObjectsListPanel> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard) {
		return std::make_shared<LevelPackObjectsListPanel>(mainEditorWindow, clipboard);
	}

	bool handleEvent(sf::Event event) override;

	void setLevelPack(LevelPack* levelPack);

private:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	LevelPack* levelPack;
};