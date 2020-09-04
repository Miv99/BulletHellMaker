#pragma once
#include <thread>

#include <Editor/Windows/EditorWindow.h>
#include <Editor/CustomWidgets/SliderWithEditBox.h>
#include <DataStructs/MovablePoint.h>
#include <Game/AudioPlayer.h>
#include <Game/Systems/MovementSystem.h>
#include <Game/Systems/DespawnSystem.h>
#include <Game/Systems/EnemySystem.h>
#include <Game/Systems/SpriteAnimationSystem.h>
#include <Game/Systems/ShadowTrailSystem.h>
#include <Game/Systems/PlayerSystem.h>
#include <Game/Systems/CollectibleSystem.h>
#include <Game/Systems/DebugRenderSystem.h>
#include <LevelPack/LevelPack.h>
#include <LevelPack/Attack.h>
#include <LevelPack/EditorMovablePoint.h>

class LevelPackObjectPreviewPanel;
class EntityCreationQueue;

/*
Window used to show a LevelPackObjectPreviewPanel.

This is necessary for LevelPackObjectPreviewPanel because SFML requires render calls
to be made on the same thread as the construction of the RenderWindow. This object also acts
as a controller between whatever uses it and the underlying LevelPackObjectPreviewPanel by
limiting resource-intensive actions when the window is closed.
*/
class LevelPackObjectPreviewWindow : public EditorWindow {
public:
	LevelPackObjectPreviewWindow(std::string windowTitle, int width, int height, std::string levelPackName, bool scaleWidgetsOnResize = false,
		bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);

	void previewNothing();
	void previewAttack(const std::shared_ptr<EditorAttack> attack);
	void previewAttackPattern(const std::shared_ptr<EditorAttackPattern> attackPattern);

	/*
	Should be called whenever an EditorAttack in the LevelPack being edited is modified.
	*/
	void onOriginalLevelPackAttackModified(const std::shared_ptr<EditorAttack> attack);
	/*
	Should be called whenever an EditorAttackPattern in the LevelPack being edited is modified.
	*/
	void onOriginalLevelPackAttackPatternModified(const std::shared_ptr<EditorAttackPattern> attackPattern);

	void deleteAttack(int id);
	void deleteAttackPattern(int id);

private:
	std::shared_ptr<LevelPackObjectPreviewPanel> previewPanel;

	std::shared_ptr<tgui::Panel> infoPanel;
	std::shared_ptr<tgui::Label> previewObjectLabel;
	std::shared_ptr<tgui::Label> delayLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> delay;
	std::shared_ptr<tgui::Label> timeMultiplierLabel;
	std::shared_ptr<SliderWithEditBox> timeMultiplier;
	std::shared_ptr<tgui::Button> resetPreview;
	std::shared_ptr<tgui::Button> resetCamera;
	std::shared_ptr<tgui::Button> setPlayerSpawn;
	std::shared_ptr<tgui::Button> setSource;
	std::shared_ptr<tgui::CheckBox> useDebugRenderSystem;
	std::shared_ptr<tgui::CheckBox> lockCurrentPreviewCheckBox;
	std::shared_ptr<tgui::CheckBox> invinciblePlayer;

	std::shared_ptr<tgui::TextBox> logs;

	std::string levelPackName;

	// While this is checked, new previews cannot start
	bool lockCurrentPreview = false;

	std::thread previewThread;

	bool ignoreSignals = false;

	/*
	Update widget positions/sizes depending on which ones are visible.
	*/
	void updateWidgetsPositionsAndSizes();

	bool handleEvent(sf::Event event) override;

	void onRenderWindowInitialization() override;

	void physicsUpdate(float deltaTime) override;
	void render(float deltaTime) override;
};