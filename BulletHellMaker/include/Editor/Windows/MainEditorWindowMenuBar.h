#pragma once
#include <TGUI/TGUI.hpp>

#include <Editor/Windows/EditorWindowConfirmationPromptChoice.h>

class MainEditorWindow;

class MainEditorWindowMenuBar : public tgui::MenuBar {
public:
	MainEditorWindowMenuBar(MainEditorWindow& mainEditorWindow);
	static std::shared_ptr<MainEditorWindowMenuBar> create(MainEditorWindow& mainEditorWindow) {
		return std::make_shared<MainEditorWindowMenuBar>(mainEditorWindow);
	}

private:
	MainEditorWindow& mainEditorWindow;

	void promptOpenLevelPack();

	void onOpenLevelPackWhileUnsavedChangesExistConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice);
};