#include <Editor/SpriteSheets/SpriteSheetMetafileEditor.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/Windows/MainEditorWindow.h>

SpriteSheetMetafileEditor::SpriteSheetMetafileEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<SpriteSheet> spriteSheet)
	: mainEditorWindow(mainEditorWindow), clipboard(clipboard), spriteSheet(spriteSheet), CopyPasteable(SPRITE_SHEET_METAFILE_SPRITE_COPY_PASTE_ID) {

	viewController = std::make_unique<ViewController>(*mainEditorWindow.getWindow());
	// Create default missing sprite
	sf::Image backgroundImage;
	sf::Uint8 pixels[16];
	pixels[0] = 255;
	pixels[1] = 0;
	pixels[2] = 0;
	pixels[3] = 255;
	backgroundImage.create(1, 1, pixels);
	backgroundTexture.loadFromImage(backgroundImage);
	backgroundSprite = tgui::Sprite(backgroundTexture);

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	utilityWidgetsPanel = SimpleWidgetsContainerPanel::create();
	utilityWidgetsPanel->setWidth("100%");
	add(utilityWidgetsPanel);



	onPositionChange.connect([this]() {
		updateWindowView();
	});
	onSizeChange.connect([this]() {
		updateWindowView();
	});
}

bool SpriteSheetMetafileEditor::updateTime(tgui::Duration elapsedTime) {
	bool ret = tgui::Panel::updateTime(elapsedTime);

	return viewController->update(viewFromViewController, elapsedTime.asSeconds()) || ret;
}

void SpriteSheetMetafileEditor::draw(tgui::BackendRenderTargetBase& target, tgui::RenderStates states) const {
	tgui::BackendRenderTargetSFML& sfmlTarget = dynamic_cast<tgui::BackendRenderTargetSFML&>(target);
	sf::RenderTarget& renderTarget = *sfmlTarget.getTarget();
	sfmlTarget.drawSprite(states, backgroundSprite);

	sf::View originalView = renderTarget.getView();
	renderTarget.setView(viewFromViewController);
	if (loadedTexture) {
		sfmlTarget.drawSprite(states, fullTextureAsSprite);
	}
	renderTarget.setView(originalView);

	for (auto widget : getWidgets()) {
		widget->draw(target, states);
	}
}

CopyOperationResult SpriteSheetMetafileEditor::copyFrom() {
	// TODO
	return CopyOperationResult(nullptr, "");
}

PasteOperationResult SpriteSheetMetafileEditor::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// TODO
	return PasteOperationResult(false, "");
}

PasteOperationResult SpriteSheetMetafileEditor::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	// TODO
	return PasteOperationResult(false, "");
}

bool SpriteSheetMetafileEditor::handleEvent(sf::Event event) {
	if (viewController->handleEvent(viewFromViewController, event)) {
		return true;
	}
	return false;
}

void SpriteSheetMetafileEditor::loadImage(std::shared_ptr<SpriteLoader> spriteLoader, std::string spriteSheetName) {
	loadedTexture = spriteLoader->getSpriteSheet(spriteSheetName)->getTexture();
	fullTextureAsSprite = tgui::Sprite(*loadedTexture);

	viewFromViewController.setCenter(loadedTexture->getSize().x / 2.0f, loadedTexture->getSize().y / 2.0f);
	updateWindowView();
}

void SpriteSheetMetafileEditor::resetCamera() {
	// TODO
}

tgui::Signal& SpriteSheetMetafileEditor::getSignal(tgui::String signalName) {
	if (signalName == onMetafileModify.getName().toLower()) {
		return onMetafileModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void SpriteSheetMetafileEditor::updateWindowView() {
	if (!loadedTexture) {
		return;
	}

	auto windowSize = mainEditorWindow.getWindow()->getSize();
	auto size = getSize();
	float sizeRatio = size.x / (float)size.y;
	sf::Vector2u resolution = loadedTexture->getSize();
	float playAreaViewRatio = resolution.x / (float)resolution.y;

	float viewWidth, viewHeight;
	if (sizeRatio > playAreaViewRatio) {
		viewHeight = resolution.y;
		viewWidth = resolution.y * size.x / (float)size.y;
		float viewX = -(viewWidth - resolution.x) / 2.0f;
		float viewY = 0;
		viewFloatRect = sf::FloatRect(0, viewY, viewWidth, viewHeight);
	} else {
		viewWidth = resolution.x;
		viewHeight = resolution.x * size.y / (float)size.x;
		float viewX = 0;
		float viewY = -(viewHeight - resolution.y) / 2.0f;
		viewFloatRect = sf::FloatRect(0, viewY, viewWidth, viewHeight);
	}

	viewController->setOriginalViewSize(viewWidth, viewHeight);

	float viewportX = getAbsolutePosition().x / windowSize.x;
	float viewportY = getAbsolutePosition().y / windowSize.y;
	float viewportWidth = getSize().x / windowSize.x;
	float viewportHeight = getSize().y / windowSize.y;
	viewportFloatRect = sf::FloatRect(viewportX, viewportY, viewportWidth, viewportHeight);

	sf::Vector2f oldCenter = viewFromViewController.getCenter();
	viewController->setViewZone(viewFromViewController, viewFloatRect);
	viewFromViewController.setViewport(viewportFloatRect);
	viewFromViewController.setCenter(oldCenter);

	backgroundSprite.setSize(size);
	backgroundSprite.setPosition(getPosition());
}