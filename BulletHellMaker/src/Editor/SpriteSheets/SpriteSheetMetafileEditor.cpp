#include <Editor/SpriteSheets/SpriteSheetMetafileEditor.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <Editor/Util/TguiUtils.h>

const char SpriteSheetMetafileEditor::ANIMATABLES_LIST_SPRITE_INDICATOR = '!';
const char SpriteSheetMetafileEditor::ANIMATABLES_LIST_ANIMATION_INDICATOR = '@';

SpriteSheetMetafileEditor::SpriteSheetMetafileEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard)
	: mainEditorWindow(mainEditorWindow), clipboard(clipboard), spriteSheet(spriteSheet), CopyPasteable(SPRITE_SHEET_METAFILE_SPRITE_COPY_PASTE_ID) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	viewController = std::make_unique<ViewController>(*mainEditorWindow.getWindow());
	viewController->getOnZoomAmountChange().sink().connect<SpriteSheetMetafileEditor, &SpriteSheetMetafileEditor::scaleTransparentTextureToCameraZoom>(this);

	// Create background sprite
	{
		sf::Image backgroundImage;
		sf::Uint8 pixels[4];
		pixels[0] = 255;
		pixels[1] = 255;
		pixels[2] = 255;
		pixels[3] = 255;
		backgroundImage.create(1, 1, pixels);
		backgroundTexture.loadFromImage(backgroundImage);
		backgroundSprite = sf::Sprite(backgroundTexture);
		backgroundSprite.setPosition(sf::Vector2f(0, 0));
	}

	// Create transparent texture
	{
		sf::Image transparentTextureImage;
		sf::Uint8 pixels[16];
		pixels[0] = 255;
		pixels[1] = 255;
		pixels[2] = 255;
		pixels[3] = 255;
		pixels[4] = 191;
		pixels[5] = 191;
		pixels[6] = 191;
		pixels[7] = 255;
		pixels[8] = 191;
		pixels[9] = 191;
		pixels[10] = 191;
		pixels[11] = 255;
		pixels[12] = 255;
		pixels[13] = 255;
		pixels[14] = 255;
		pixels[15] = 255;
		transparentTextureImage.create(2, 2, pixels);
		transparentTexture.loadFromImage(transparentTextureImage);
		transparentTexture.setRepeated(true);
		transparentTextureSprite = sf::Sprite(transparentTexture);
		transparentTextureSprite.setPosition(0, 0);
	}

	animatablesListView = ListView::create();
	animatablesListView->setPosition(0, 0);
	animatablesListView->setSize("25%", "100%");
	animatablesListView->onDoubleClick([this](int index) {
		if (index == -1) {
			return;
		}

		tgui::String id = animatablesListView->getItem(index);

		if (id.at(0) == ANIMATABLES_LIST_SPRITE_INDICATOR) {
			// TODO: look at the sprite
			std::string spriteName = static_cast<std::string>(id.substr(1));
		} else if (id.at(0) == ANIMATABLES_LIST_ANIMATION_INDICATOR) {
			// TODO: open animation editor
			std::string animationName = static_cast<std::string>(id.substr(1));
		}
	});
	animatablesListView->onItemSelect([this](int index) {
		if (index == -1) {
			return;
		}

		tgui::String id = animatablesListView->getItem(index);

		if (id.at(0) == ANIMATABLES_LIST_SPRITE_INDICATOR) {
			// TODO: draw rectangle around the sprite
			std::string spriteName = static_cast<std::string>(id.substr(1));
		} else if (id.at(0) == ANIMATABLES_LIST_ANIMATION_INDICATOR) {
			// TODO: draw rectangles around the sprites used in the animation
			std::string animationName = static_cast<std::string>(id.substr(1));
		}
	});
	add(animatablesListView);

	utilityWidgetsPanel = SimpleWidgetsContainerPanel::create();
	utilityWidgetsPanel->setWidth("100%");
	utilityWidgetsPanel->setPosition(tgui::bindRight(animatablesListView), 0);
	//add(utilityWidgetsPanel);

	lastChosenBackgroundColor = tgui::Color(207, 207, 207, 255);
	backgroundSprite.setColor(lastChosenBackgroundColor);

	backgroundColorPicker = tgui::ColorPicker::create();
	backgroundColorPicker->setTitle("Background color");
	backgroundColorPicker->setKeepInParent(true);
	backgroundColorPicker->setColor(lastChosenBackgroundColor);
	backgroundColorPicker->onColorChange([this](tgui::Color color) {
		backgroundSprite.setColor(color);
	});
	backgroundColorPicker->onOkPress([this](tgui::Color color) {
		lastChosenBackgroundColor = color;
	});
	backgroundColorPicker->onClose([this]() {
		backgroundSprite.setColor(lastChosenBackgroundColor);
	});

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
	sf::RenderStates sfmlStates = toSFMLRenderStates(states);
	renderTarget.draw(backgroundSprite, sfmlStates);

	sf::View originalView = renderTarget.getView();
	renderTarget.setView(viewFromViewController);
	renderTarget.draw(transparentTextureSprite, sfmlStates);
	if (loadedTexture) {
		sfmlTarget.drawSprite(states, fullTextureAsSprite);
	}
	renderTarget.setView(originalView);

	for (auto widget : getWidgets()) {
		// Special handling required for ChildWindows
		if (widget == backgroundColorPicker) {
			states.transform.translate(widget->getPosition());
			widget->draw(target, states);
			states.transform.translate(-widget->getPosition());
		} else {
			widget->draw(target, states);
		}
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
	} else if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::B) {
			add(backgroundColorPicker);
		}
	}
	return false;
}

void SpriteSheetMetafileEditor::loadSpriteSheet(std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<SpriteSheet> spriteSheet) {
	this->spriteSheet = spriteSheet;

	loadImage(spriteLoader, spriteSheet->getName());

	animatablesListView->removeAllItems();
	for (std::pair<std::string, std::shared_ptr<SpriteData>> item : spriteSheet->getSpriteData()) {
		animatablesListView->addItem(format("[S] %s", item.second->getSpriteName().c_str()), format("%c%s", ANIMATABLES_LIST_SPRITE_INDICATOR, item.second->getSpriteName().c_str()));
	}
	for (std::pair<std::string, std::shared_ptr<AnimationData>> item : spriteSheet->getAnimationData()) {
		animatablesListView->addItem(format("[A] %s", item.second->getAnimationName().c_str()), format("%c%s", ANIMATABLES_LIST_ANIMATION_INDICATOR, item.second->getAnimationName().c_str()));
	}

	viewFromViewController.setCenter(loadedTexture->getSize().x / 2.0f, loadedTexture->getSize().y / 2.0f);
	updateWindowView();
}

void SpriteSheetMetafileEditor::loadImage(std::shared_ptr<SpriteLoader> spriteLoader, std::string spriteSheetName) {
	// Load texture from spriteLoader rather than this->spriteSheet because this->spriteSheet's 
	// texture isn't guaranteed to be loaded
	loadedTexture = spriteLoader->getSpriteSheet(spriteSheetName)->getTexture();
	fullTextureAsSprite = tgui::Sprite(*loadedTexture);

	scaleTransparentTextureToCameraZoom(viewController->getZoomAmount());
}

void SpriteSheetMetafileEditor::resetCamera() {
	// TODO
}

void SpriteSheetMetafileEditor::scaleTransparentTextureToCameraZoom(float cameraZoomAmount) {
	float scale = 4.0f / cameraZoomAmount;

	transparentTextureSprite.setScale(scale, scale);
	if (loadedTexture) {
		transparentTextureSprite.setTextureRect(sf::IntRect(0, 0, loadedTexture->getSize().x / scale, loadedTexture->getSize().y / scale));
	}
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

	backgroundSprite.setScale(size);
	backgroundSprite.setPosition(getPosition());
}