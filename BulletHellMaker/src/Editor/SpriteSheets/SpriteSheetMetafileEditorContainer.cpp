#include <Editor/SpriteSheets/SpriteSheetMetafileEditorContainer.h>

#include <Editor/Windows/MainEditorWindow.h>

const std::string SpriteSheetMetafileEditorContainer::MAIN_EDITOR_TAB_NAME = "Main Editor";
const std::string SpriteSheetMetafileEditorContainer::ANIMATION_EDITOR_TAB_PREFIX = "/";

SpriteSheetMetafileEditorContainer::SpriteSheetMetafileEditorContainer(MainEditorWindow& mainEditorWindow, Clipboard& clipboard,
	const std::string& levelPackName, std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<SpriteSheet> spriteSheet)
	: mainEditorWindow(mainEditorWindow) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	tabs = TabsWithPanel::create(mainEditorWindow);
	tabs->setSize("100%", "100%");
	add(tabs);

	metafileEditor = SpriteSheetMetafileEditor::create(mainEditorWindow, clipboard);
	bool successfulLoad = metafileEditor->loadSpriteSheet(levelPackName, spriteLoader, spriteSheet);
	if (!successfulLoad) {
		throw std::runtime_error("Failed to load sprite sheet");
	}
	// Set this->spriteSheet after having metafileEditor load a copy of spriteSheet
	this->spriteSheet = metafileEditor->getSpriteSheet();

	metafileEditor->onMetafileModify.connect([this](std::shared_ptr<SpriteSheet> spriteSheet) {
		// Propagate signal
		onMetafileModify.emit(this, spriteSheet);
	});
	metafileEditor->onAnimationEditRequest.connect([this](tgui::String animationName) {
		std::shared_ptr<AnimationData> animationData = this->spriteSheet->getAnimationData(static_cast<std::string>(animationName));
		if (animationData) {
			std::string tabName = ANIMATION_EDITOR_TAB_PREFIX + animationData->getAnimationName();
			// If tab already exists, open it
			if (tabs->hasTab(tabName)) {
				tabs->selectTab(tabName);
			} else {
				std::shared_ptr<SpriteSheetAnimationEditor> animationEditor
					= SpriteSheetAnimationEditor::create(this->mainEditorWindow, metafileEditor->getSpriteLoader(), animationData);
				animationEditor->setSpriteLoader(metafileEditor->getSpriteLoader(), this->spriteSheet->getName());

				animationEditor->onAnimationModify.connect([this]() {
					metafileEditor->reloadPreviewAnimation();
				});

				// Put some string before the animation editor tab's name to avoid potential conflict with metafileEditor's tab
				tabs->addTab(tabName, animationEditor, true, true);
			}
		}
	});
	metafileEditor->onAnimationDelete.connect([this](tgui::String animationName) {
		// Delete animation editor tab if its animation data is deleted
		std::string tabName = ANIMATION_EDITOR_TAB_PREFIX + static_cast<std::string>(animationName);
		if (tabs->hasTab(tabName)) {
			tabs->removeTab(tabName);
		}
	});
	tabs->addTab(MAIN_EDITOR_TAB_NAME, metafileEditor, true, false);
}

bool SpriteSheetMetafileEditorContainer::handleEvent(sf::Event event) {
	return tabs->handleEvent(event);
}

void SpriteSheetMetafileEditorContainer::loadImage(const std::string& levelPackName, std::shared_ptr<SpriteLoader> spriteLoader, std::string spriteSheetName) {
	bool success = metafileEditor->loadImage(levelPackName, spriteLoader, spriteSheetName);
	if (!success) {
		throw std::runtime_error("Failed to load sprite sheet image");
	}

	// Use metafileEditor's SpriteLoader to set the animation editors' SpriteLoaders
	for (auto tabName : tabs->getTabNames()) {
		std::shared_ptr<SpriteSheetAnimationEditor> animationEditor = std::dynamic_pointer_cast<SpriteSheetAnimationEditor>(tabs->getTab(tabName));
		if (animationEditor) {
			animationEditor->setSpriteLoader(metafileEditor->getSpriteLoader(), spriteSheet->getName());
		}
	}
}

void SpriteSheetMetafileEditorContainer::renameSprite(std::string oldSpriteName, std::string newSpriteName) {
	// Let everything get handled by SpriteSheetMetafileEditor, including emitting the onMetafileModify signal
	metafileEditor->renameSprite(oldSpriteName, newSpriteName);
}

void SpriteSheetMetafileEditorContainer::renameAnimation(std::string oldAnimationName, std::string newAnimationName) {
	spriteSheet->renameAnimation(oldAnimationName, newAnimationName);

	// If animation editor tab exists, rename it
	if (tabs->hasTab(ANIMATION_EDITOR_TAB_PREFIX + oldAnimationName)) {
		tabs->renameTab(ANIMATION_EDITOR_TAB_PREFIX + oldAnimationName, ANIMATION_EDITOR_TAB_PREFIX + newAnimationName);
	}

	metafileEditor->repopulateAnimatablesListView();

	onMetafileModify.emit(this, spriteSheet);
}
