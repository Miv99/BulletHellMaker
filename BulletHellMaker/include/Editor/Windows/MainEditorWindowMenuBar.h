#pragma once
#include <TGUI/TGUI.hpp>

class MainEditorWindow;

class MainEditorWindowMenuBar : public tgui::MenuBar {
public:
	MainEditorWindowMenuBar(MainEditorWindow& mainEditorWindow);
	static std::shared_ptr<MainEditorWindowMenuBar> create(MainEditorWindow& mainEditorWindow) {
		return std::make_shared<MainEditorWindowMenuBar>(mainEditorWindow);
	}

private:
	MainEditorWindow& mainEditorWindow;
};