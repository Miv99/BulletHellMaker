#pragma once
#include <Editor/EventCapturable.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <Editor/CopyPaste.h>
#include <LevelPack/LevelPack.h>

class LevelPackObjectsListView;

/*
An EventCapturable basic tgui::Panel to be used by MainEditorWindow for viewing a LevelPackObjectsListView.
This widget's main purpose is to pass events down to the child LevelPackObjectsListView widget and hold
utility buttons that can't be scrolled past.
*/
class LevelPackObjectsListPanel : public tgui::Panel, public EventCapturable {
public:
	/*
	childListView - the LevelPackObjectsListView that this widget will pass events to
	*/
	LevelPackObjectsListPanel(LevelPackObjectsListView& childListView);
	static std::shared_ptr<LevelPackObjectsListPanel> create(LevelPackObjectsListView& childListView) {
		return std::make_shared<LevelPackObjectsListPanel>(childListView);
	}

	bool handleEvent(sf::Event event) override;

private:
	LevelPackObjectsListView& childListView;
};