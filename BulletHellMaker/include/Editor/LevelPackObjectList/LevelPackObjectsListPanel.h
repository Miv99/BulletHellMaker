#pragma once
#include <Editor/EventCapturable.h>
#include <Editor/EditorWindow.h>
#include <Editor/CopyPaste.h>
#include <LevelPack/LevelPack.h>

/*
An EventCapturable basic tgui::Panel to be used by MainEditorWindow for viewing a LevelPackObjectsListView.
This widget's main purpose is to pass events down to the child LevelPackObjectsListView widget and hold
utility buttons that can't be scrolled past.
*/
class LevelPackObjectsListPanel : public tgui::Panel, public EventCapturable {
public:
	/*
	mainEditorWindow - the parent MainEditorWindow
	childListView - the LevelPackObjectsListView that this widget will pass events to
	*/
	LevelPackObjectsListPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, LevelPackObjectsListView& childListView);
	static std::shared_ptr<LevelPackObjectsListPanel> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, 
		LevelPackObjectsListView& childListView) {
		return std::make_shared<LevelPackObjectsListPanel>(mainEditorWindow, clipboard, childListView);
	}

	bool handleEvent(sf::Event event) override;

	void setLevelPack(LevelPack* levelPack);

private:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	LevelPack* levelPack;
	LevelPackObjectsListView& childListView;
};