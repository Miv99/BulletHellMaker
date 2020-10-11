#include <Editor/SpriteSheets/SpriteSheetMetafileEditor.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/CustomWidgets/EditBox.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <Editor/Util/TguiUtils.h>

const char SpriteSheetMetafileEditor::ANIMATABLES_LIST_SPRITE_INDICATOR = '!';
const char SpriteSheetMetafileEditor::ANIMATABLES_LIST_ANIMATION_INDICATOR = '@';
const float SpriteSheetMetafileEditor::SELECTION_RECTANGLE_COLOR_CHANGE_INTERVAL = 0.4f;
const int SpriteSheetMetafileEditor::SELECTION_RECTANGLE_WIDTH = 2;

SpriteSheetMetafileEditor::SpriteSheetMetafileEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize)
	: mainEditorWindow(mainEditorWindow), clipboard(clipboard), spriteSheet(spriteSheet),
	CopyPasteable(SPRITE_SHEET_METAFILE_SPRITE_COPY_PASTE_ID), undoStack(UndoStack(undoStackSize)) {

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
			deselectRectangles();
			return;
		}

		tgui::String id = animatablesListView->getSelectedItemId();

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
			deselectRectangles();
			return;
		}

		tgui::String id = animatablesListView->getSelectedItemId();

		if (id.at(0) == ANIMATABLES_LIST_SPRITE_INDICATOR) {
			std::string spriteName = static_cast<std::string>(id.substr(1));
			if (spriteSheet->hasSpriteData(spriteName)) {
				selectSpriteRectangles(spriteSheet->getSpriteData(spriteName));
			}
		} else if (id.at(0) == ANIMATABLES_LIST_ANIMATION_INDICATOR) {
			std::string animationName = static_cast<std::string>(id.substr(1));
			if (spriteSheet->hasAnimationData(animationName)) {
				selectAnimationRectangles(spriteSheet->getAnimationData(animationName));
			}
		}
	});
	add(animatablesListView);



	utilityWidgetsPanel = SimpleWidgetsContainerPanel::create();
	utilityWidgetsPanel->setWidth("75%");
	utilityWidgetsPanel->setPosition("25%", 0);
	add(utilityWidgetsPanel);



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

	timeUntilSelectionRectangleColorChange -= elapsedTime.asSeconds();
	if (timeUntilSelectionRectangleColorChange <= 0) {
		timeUntilSelectionRectangleColorChange = SELECTION_RECTANGLE_COLOR_CHANGE_INTERVAL;

		if (selectionRectangleColor == tgui::Color::Black) {
			selectionRectangleColor = tgui::Color::White;
		} else {
			selectionRectangleColor = tgui::Color::Black;
		}
	}

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

		// 1 pixel on the screen is equal to this much in world coordinates
		sf::Vector2f scale = renderTarget.mapPixelToCoords(sf::Vector2i(1, 1)) - renderTarget.mapPixelToCoords(sf::Vector2i(0, 0));
		scale.x = std::abs(scale.x);
		scale.y = std::abs(scale.y);
		// Create borders such that they're always 2 pixels in each direction
		tgui::Borders borders = { scale.x * SELECTION_RECTANGLE_WIDTH, scale.x * SELECTION_RECTANGLE_WIDTH, 
			scale.y * SELECTION_RECTANGLE_WIDTH, scale.y * SELECTION_RECTANGLE_WIDTH };

		for (auto rect : selectionRectangles) {
			states.transform.translate(rect.getPosition());
			sfmlTarget.drawBorders(states, borders, rect.getSize(), selectionRectangleColor);
			states.transform.translate(-rect.getPosition());
		}
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

	repopulateAnimatablesListView();

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

void SpriteSheetMetafileEditor::repopulateAnimatablesListView() {
	animatablesListView->removeAllItems();
	for (std::pair<std::string, std::shared_ptr<SpriteData>> item : spriteSheet->getSpriteData()) {
		animatablesListView->addItem(format("[S] %s", item.second->getSpriteName().c_str()), format("%c%s", ANIMATABLES_LIST_SPRITE_INDICATOR, item.second->getSpriteName().c_str()));
	}
	for (std::pair<std::string, std::shared_ptr<AnimationData>> item : spriteSheet->getAnimationData()) {
		animatablesListView->addItem(format("[A] %s", item.second->getAnimationName().c_str()), format("%c%s", ANIMATABLES_LIST_ANIMATION_INDICATOR, item.second->getAnimationName().c_str()));
	}
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

void SpriteSheetMetafileEditor::renameSprite(std::string oldSpriteName, std::string newSpriteName) {
	spriteSheet->renameSprite(oldSpriteName, newSpriteName);

	repopulateAnimatablesListView();

	onMetafileModify.emit(this, spriteSheet);
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

void SpriteSheetMetafileEditor::deselectRectangles() {
	selectionRectangles.clear();
}

void SpriteSheetMetafileEditor::selectSpriteRectangles(std::shared_ptr<SpriteData> spriteData) {
	selectionRectangles.clear();

	sf::RectangleShape shape;
	ComparableIntRect spriteRect = spriteData->getArea();
	shape.setSize(sf::Vector2f(spriteRect.width, spriteRect.height));
	shape.setPosition(spriteRect.left, spriteRect.top);
	selectionRectangles.push_back(shape);
}

void SpriteSheetMetafileEditor::selectAnimationRectangles(std::shared_ptr<AnimationData> animationData) {
	selectionRectangles.clear();
	for (std::pair<float, std::string> data : animationData->getSpriteNames()) {
		if (spriteSheet->hasSpriteData(data.second)) {
			sf::RectangleShape shape;
			ComparableIntRect spriteRect = spriteSheet->getSpriteData(data.second)->getArea();
			shape.setSize(sf::Vector2f(spriteRect.width, spriteRect.height));
			shape.setPosition(spriteRect.left, spriteRect.top);
			selectionRectangles.push_back(shape);
		}
	}
}
