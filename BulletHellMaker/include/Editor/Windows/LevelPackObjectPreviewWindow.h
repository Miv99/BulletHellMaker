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
	LevelPackObjectPreviewWindow(std::string windowTitle, int width, int height, std::string levelPackName, std::shared_ptr<SpriteLoader> spriteLoader,
		bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);

	/*
	Reopens the window if it was closed.
	*/
	void reopenWindow();

	/*
	Loads a level pack by name.

	spriteLoader - if not nullptr, this sprite loader will be used in the newly loaded level pack
		so that textures don't have to be loaded twice
	*/
	void loadLevelPack(std::string levelPackName, std::shared_ptr<SpriteLoader> spriteLoader);

	void previewNothing();
	void previewAttack(const std::shared_ptr<EditorAttack> attack);
	void previewAttackPattern(const std::shared_ptr<EditorAttackPattern> attackPattern);

	void resetPreview();

	/*
	Should be called whenever an EditorAttack in the LevelPack being edited is modified.
	*/
	void onOriginalLevelPackAttackModified(const std::shared_ptr<EditorAttack> attack);
	/*
	Should be called whenever an EditorAttackPattern in the LevelPack being edited is modified.
	*/
	void onOriginalLevelPackAttackPatternModified(const std::shared_ptr<EditorAttackPattern> attackPattern);
	/*
	Should be called whenever a SpriteSheet in the LevelPack being edited is modified.
	*/
	void onOriginalLevelPackSpriteSheetModified(const std::shared_ptr<SpriteSheet> spriteSheet);

	void deleteAttack(int id);
	void deleteAttackPattern(int id);

private:
	std::shared_ptr<LevelPackObjectPreviewPanel> previewPanel;

	// previewPanel's level pack's overriden sprite loader
	std::shared_ptr<SpriteLoader> spriteLoader;

	std::shared_ptr<tgui::Panel> infoPanel;
	std::shared_ptr<tgui::Label> previewObjectLabel;
	std::shared_ptr<tgui::Label> delayLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> delay;
	std::shared_ptr<tgui::Label> timeMultiplierLabel;
	std::shared_ptr<SliderWithEditBox> timeMultiplier;
	std::shared_ptr<tgui::Button> resetPreviewButton;
	std::shared_ptr<tgui::Button> resetCameraButton;
	std::shared_ptr<tgui::Button> setPlayerSpawnButton;
	std::shared_ptr<tgui::Button> setSourceButton;
	std::shared_ptr<tgui::CheckBox> useDebugRenderSystem;
	std::shared_ptr<tgui::CheckBox> lockCurrentPreviewCheckBox;
	std::shared_ptr<tgui::CheckBox> invinciblePlayer;

	std::shared_ptr<tgui::TextArea> logs;

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