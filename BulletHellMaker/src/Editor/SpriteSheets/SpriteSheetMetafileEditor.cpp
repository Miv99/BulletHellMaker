#include <Editor/SpriteSheets/SpriteSheetMetafileEditor.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/CustomWidgets/EditBox.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <Editor/Util/TguiUtils.h>
#include <Editor/Util/EditorUtils.h>

const char SpriteSheetMetafileEditor::ANIMATABLES_LIST_SPRITE_INDICATOR = '!';
const char SpriteSheetMetafileEditor::ANIMATABLES_LIST_ANIMATION_INDICATOR = '@';
const float SpriteSheetMetafileEditor::WORLD_SELECTION_COLORS_CHANGE_INTERVAL = 0.4f;
const int SpriteSheetMetafileEditor::SELECTION_RECTANGLE_WIDTH = 2;
const float SpriteSheetMetafileEditor::WORLD_SELECTION_CURSOR_DIAMETER = 7.5f;
const float SpriteSheetMetafileEditor::WORLD_SELECTION_CURSOR_BORDER_THICKNESS = 2.5f;
const float SpriteSheetMetafileEditor::MAX_CAMERA_ZOOM_SCALAR = 25.0f;

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

	std::shared_ptr<tgui::Button> addSpriteButton = tgui::Button::create();
	addSpriteButton->setTextSize(TEXT_SIZE);
	addSpriteButton->setText("+S");
	addSpriteButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	addSpriteButton->onPress([this]() {
		this->mainEditorWindow.promptInput("Enter name of new sprite:", animatablesListView.get(), true)->sink()
			.connect<SpriteSheetMetafileEditor, &SpriteSheetMetafileEditor::onNewSpriteNameInput>(this);
	});
	add(addSpriteButton);

	std::shared_ptr<tgui::Button> addAnimationButton = tgui::Button::create();
	addAnimationButton->setTextSize(TEXT_SIZE);
	addAnimationButton->setText("+A");
	addAnimationButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	addAnimationButton->setPosition(tgui::bindRight(addSpriteButton), tgui::bindTop(addSpriteButton));
	addAnimationButton->onPress([this]() {
		this->mainEditorWindow.promptInput("Enter name of new animation:", animatablesListView.get(), true)->sink()
			.connect<SpriteSheetMetafileEditor, &SpriteSheetMetafileEditor::onNewAnimationNameInput>(this);
	});
	add(addAnimationButton);

	animatablesListView = ListView::create();
	animatablesListView->setPosition(tgui::bindLeft(addSpriteButton), tgui::bindBottom(addSpriteButton));
	animatablesListView->onDoubleClick([this](int index) {
		if (index == -1) {
			onAnimatableDeselect();
			return;
		}

		tgui::String id = animatablesListView->getSelectedItemId();

		if (id.at(0) == ANIMATABLES_LIST_SPRITE_INDICATOR) {
			std::string spriteName = static_cast<std::string>(id.substr(1));

			// Look at the sprite
			if (spriteSheet) {
				std::shared_ptr<SpriteData> spriteData = spriteSheet->getSpriteData(spriteName);
				if (spriteData) {
					sf::IntRect area = spriteData->getArea();
					viewFromViewController.setCenter(area.left + area.width / 2.0f, area.top + area.height / 2.0f);
				}
			}
		} else if (id.at(0) == ANIMATABLES_LIST_ANIMATION_INDICATOR) {
			std::string animationName = static_cast<std::string>(id.substr(1));
			onAnimationEditRequest.emit(this, animationName);
		}
	});
	animatablesListView->onItemSelect([this](int index) {
		if (index == -1) {
			onAnimatableDeselect();
			return;
		}

		tgui::String id = animatablesListView->getSelectedItemId();

		if (id.at(0) == ANIMATABLES_LIST_SPRITE_INDICATOR) {
			std::string spriteName = static_cast<std::string>(id.substr(1));
			if (spriteSheet->hasSpriteData(spriteName)) {
				onSpriteSelect(spriteSheet->getSpriteData(spriteName));
			}
		} else if (id.at(0) == ANIMATABLES_LIST_ANIMATION_INDICATOR) {
			std::string animationName = static_cast<std::string>(id.substr(1));
			if (spriteSheet->hasAnimationData(animationName)) {
				onAnimationSelect(spriteSheet->getAnimationData(animationName));
			}
		}
	});
	add(animatablesListView);


	animatablePreviewChildWindow = ChildWindow::create();
	animatablePreviewChildWindow->setTitleButtons(tgui::ChildWindow::TitleButton::Close);
	animatablePreviewChildWindow->setSize("30%", "30%");
	// For some reason, the window must be added to this widget first at least once, or else animatables
	// selected before the first time it is added will not show up in the preview
	add(animatablePreviewChildWindow);
	animatablePreviewChildWindow->close();

	animatablePreviewPicture = AnimatablePicture::create();
	animatablePreviewChildWindow->onSizeChange.connect([this]() {
		animatablePreviewPicture->setSize(animatablePreviewChildWindow->getInnerSize());
	});
	animatablePreviewChildWindow->add(animatablePreviewPicture);


	utilityWidgetsPanel = SimpleWidgetsContainerPanel::create();
	utilityWidgetsPanel->setWidth("75%");
	utilityWidgetsPanel->setPosition("25%", 0);
	add(utilityWidgetsPanel);

	std::shared_ptr<tgui::Button> showAnimatablePreviewButton = tgui::Button::create();
	showAnimatablePreviewButton->setTextSize(TEXT_SIZE);
	showAnimatablePreviewButton->setText("Show preview");
	showAnimatablePreviewButton->setSize(100, TEXT_BUTTON_HEIGHT);
	showAnimatablePreviewButton->onClick.connect([this]() {
		add(animatablePreviewChildWindow);
	});
	utilityWidgetsPanel->addExtraRowWidget(showAnimatablePreviewButton, GUI_LABEL_PADDING_Y);

	chooseSpriteRectButton = tgui::Button::create();
	chooseSpriteRectButton->setTextSize(TEXT_SIZE);
	chooseSpriteRectButton->setText("Set sprite area");
	chooseSpriteRectButton->setSize(100, TEXT_BUTTON_HEIGHT);
	chooseSpriteRectButton->onClick.connect([this]() {
		beginSelectingSpriteRectTopLeft();
	});
	chooseSpriteRectButton->setVisible(false);
	utilityWidgetsPanel->addExtraRowWidget(chooseSpriteRectButton, GUI_LABEL_PADDING_Y);

	chooseSpriteOriginButton = tgui::Button::create();
	chooseSpriteOriginButton->setTextSize(TEXT_SIZE);
	chooseSpriteOriginButton->setText("Set sprite origin");
	chooseSpriteOriginButton->setSize(100, TEXT_BUTTON_HEIGHT);
	chooseSpriteOriginButton->onClick.connect([this]() {
		beginSelectingSpriteRectOrigin();
	});
	chooseSpriteOriginButton->setVisible(false);
	utilityWidgetsPanel->addExtraColumnWidget(chooseSpriteOriginButton, GUI_PADDING_X);

	openSpriteColorPickerButton = tgui::Button::create();
	openSpriteColorPickerButton->setTextSize(TEXT_SIZE);
	openSpriteColorPickerButton->setText("Set sprite color");
	openSpriteColorPickerButton->setSize(100, TEXT_BUTTON_HEIGHT);
	openSpriteColorPickerButton->onClick.connect([this]() {
		add(spriteColorPicker);
	});
	openSpriteColorPickerButton->setVisible(false);
	utilityWidgetsPanel->addExtraColumnWidget(openSpriteColorPickerButton, GUI_PADDING_X);

	spriteColorPicker = tgui::ColorPicker::create();
	spriteColorPicker->setTitle("Sprite color");
	spriteColorPicker->setKeepInParent(true);
	spriteColorPicker->setColor(sf::Color(255, 255, 255, 255));
	spriteColorPicker->onOkPress([this](tgui::Color color) {
		if (selectedSpriteData) {
			sf::Color oldColor = selectedSpriteData->getColor();
			std::weak_ptr<SpriteData> weakCaptureOfSelectedSpriteData = std::weak_ptr(selectedSpriteData);
			undoStack.execute(UndoableCommand([this, weakCaptureOfSelectedSpriteData, color]() {
				std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
				spriteData->setColor(color);

				ignoreSignals = true;
				spriteColorPicker->setColor(color);
				ignoreSignals = false;

				onMetafileModify.emit(this, spriteSheet);

				// Reselect the sprite
				animatablesListView->deselectItem();
				animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
			}, [this, weakCaptureOfSelectedSpriteData, oldColor]() {
				std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
				spriteData->setColor(oldColor);

				ignoreSignals = true;
				spriteColorPicker->setColor(oldColor);
				ignoreSignals = false;

				onMetafileModify.emit(this, spriteSheet);

				// Reselect the sprite
				animatablesListView->deselectItem();
				animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
			}));
			selectedSpriteData->setColor(color);

			// Reselect the sprite
			std::string spriteName = selectedSpriteData->getSpriteName();
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteName));
		}
	});

	spriteOriginLabel = tgui::Label::create();
	spriteOriginLabel->setTextSize(TEXT_SIZE);
	spriteOriginLabel->setText("Sprite origin X/Y");
	spriteOriginLabel->setToolTip(createToolTip("The sprite origin relative to the sprite's top-left corner around \
which the sprite will be rotated/flipped."));
	spriteOriginLabel->setVisible(false);
	utilityWidgetsPanel->addExtraRowWidget(spriteOriginLabel, GUI_PADDING_Y);

	spriteOriginXEditBox = NumericalEditBoxWithLimits::create();
	spriteOriginXEditBox->setTextSize(TEXT_SIZE);
	spriteOriginXEditBox->setVisible(false);
	spriteOriginXEditBox->onValueChange.connect([this](float newValue) {
		if (ignoreSignals) {
			return;
		}

		sf::Vector2f oldOrigin = sf::Vector2f(selectedSpriteData->getSpriteOriginX(), selectedSpriteData->getSpriteOriginY());
		sf::Vector2f newOrigin = sf::Vector2f(newValue, selectedSpriteData->getSpriteOriginY());
		std::weak_ptr<SpriteData> weakCaptureOfSelectedSpriteData = std::weak_ptr<SpriteData>(selectedSpriteData);
		undoStack.execute(UndoableCommand([this, weakCaptureOfSelectedSpriteData, newOrigin]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteOrigin(newOrigin.x, newOrigin.y);

			ignoreSignals = true;
			spriteOriginXEditBox->setValue(newOrigin.x);
			ignoreSignals = false;

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}, [this, weakCaptureOfSelectedSpriteData, oldOrigin]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteOrigin(oldOrigin.x, oldOrigin.y);
			
			ignoreSignals = true;
			spriteOriginXEditBox->setValue(oldOrigin.x);
			ignoreSignals = false;

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}));
	});
	utilityWidgetsPanel->addExtraRowWidget(spriteOriginXEditBox, GUI_LABEL_PADDING_Y);

	spriteOriginYEditBox = NumericalEditBoxWithLimits::create();
	spriteOriginYEditBox->setTextSize(TEXT_SIZE);
	spriteOriginYEditBox->setVisible(false);
	spriteOriginYEditBox->onValueChange.connect([this](float newValue) {
		if (ignoreSignals) {
			return;
		}

		sf::Vector2f oldOrigin = sf::Vector2f(selectedSpriteData->getSpriteOriginX(), selectedSpriteData->getSpriteOriginY());
		sf::Vector2f newOrigin = sf::Vector2f(selectedSpriteData->getSpriteOriginX(), newValue);
		std::weak_ptr<SpriteData> weakCaptureOfSelectedSpriteData = std::weak_ptr<SpriteData>(selectedSpriteData);
		undoStack.execute(UndoableCommand([this, weakCaptureOfSelectedSpriteData, newOrigin]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteOrigin(newOrigin.x, newOrigin.y);

			ignoreSignals = true;
			spriteOriginYEditBox->setValue(newOrigin.y);
			ignoreSignals = false;

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}, [this, weakCaptureOfSelectedSpriteData, oldOrigin]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteOrigin(oldOrigin.x, oldOrigin.y);

			ignoreSignals = true;
			spriteOriginYEditBox->setValue(oldOrigin.y);
			ignoreSignals = false;

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}));
	});
	utilityWidgetsPanel->addExtraColumnWidget(spriteOriginYEditBox, GUI_PADDING_X);

	ingameSpriteSizeLabel = tgui::Label::create();
	ingameSpriteSizeLabel->setTextSize(TEXT_SIZE);
	ingameSpriteSizeLabel->setText("In-game sprite size");
	ingameSpriteSizeLabel->setToolTip(createToolTip(format("The size of this sprite when displayed in-game. The original sprite \
will be scaled to match this size. For reference, the map is %d by %d units large.", MAP_WIDTH, MAP_HEIGHT)));
	ingameSpriteSizeLabel->setVisible(false);
	utilityWidgetsPanel->addExtraRowWidget(ingameSpriteSizeLabel, GUI_PADDING_Y);

	ingameSpriteSizeXEditBox = NumericalEditBoxWithLimits::create();
	ingameSpriteSizeXEditBox->setMin(0);
	ingameSpriteSizeXEditBox->setTextSize(TEXT_SIZE);
	ingameSpriteSizeXEditBox->setVisible(false);
	ingameSpriteSizeXEditBox->onValueChange.connect([this](float newValue) {
		if (ignoreSignals) {
			return;
		}

		sf::Vector2f oldSize = sf::Vector2f(selectedSpriteData->getSpriteWidth(), selectedSpriteData->getSpriteHeight());
		sf::Vector2f newSize = sf::Vector2f(newValue, selectedSpriteData->getSpriteHeight());
		std::weak_ptr<SpriteData> weakCaptureOfSelectedSpriteData = std::weak_ptr<SpriteData>(selectedSpriteData);
		undoStack.execute(UndoableCommand([this, weakCaptureOfSelectedSpriteData, newSize]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteSize(newSize.x, newSize.y);

			ignoreSignals = true;
			ingameSpriteSizeXEditBox->setValue(newSize.x);
			ignoreSignals = false;

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}, [this, weakCaptureOfSelectedSpriteData, oldSize]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteSize(oldSize.x, oldSize.y);

			ignoreSignals = true;
			ingameSpriteSizeXEditBox->setValue(oldSize.x);
			ignoreSignals = false;

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}));
	});
	utilityWidgetsPanel->addExtraRowWidget(ingameSpriteSizeXEditBox, GUI_LABEL_PADDING_Y);

	ingameSpriteSizeYEditBox = NumericalEditBoxWithLimits::create();
	ingameSpriteSizeYEditBox->setMin(0);
	ingameSpriteSizeYEditBox->setTextSize(TEXT_SIZE);
	ingameSpriteSizeYEditBox->setVisible(false);
	ingameSpriteSizeYEditBox->onValueChange.connect([this](float newValue) {
		if (ignoreSignals) {
			return;
		}

		sf::Vector2f oldSize = sf::Vector2f(selectedSpriteData->getSpriteWidth(), selectedSpriteData->getSpriteHeight());
		sf::Vector2f newSize = sf::Vector2f(selectedSpriteData->getSpriteWidth(), newValue);
		std::weak_ptr<SpriteData> weakCaptureOfSelectedSpriteData = std::weak_ptr<SpriteData>(selectedSpriteData);
		undoStack.execute(UndoableCommand([this, weakCaptureOfSelectedSpriteData, newSize]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteSize(newSize.x, newSize.y);

			ignoreSignals = true;
			ingameSpriteSizeYEditBox->setValue(newSize.x);
			ignoreSignals = false;

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}, [this, weakCaptureOfSelectedSpriteData, oldSize]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteSize(oldSize.x, oldSize.y);

			ignoreSignals = true;
			ingameSpriteSizeYEditBox->setValue(oldSize.x);
			ignoreSignals = false;

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}));
	});
	utilityWidgetsPanel->addExtraColumnWidget(ingameSpriteSizeYEditBox, GUI_PADDING_X);

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
	onSizeChange.connect([this, addSpriteButton](sf::Vector2f newSize) {
		// Handle animatablesListView size change here to avoid calling updateWindowView() before animatablesListView's size is updated
		// since updateWindowView() uses animatablesListView's size
		animatablesListView->setSize(0.25f * newSize.x, newSize.y - tgui::bindBottom(addSpriteButton));

		updateWindowView();
	});

	utilityWidgetsPanel->onSizeChange.connect([this](sf::Vector2f newSize) {
		spriteOriginXEditBox->setSize(newSize.x / 2.0f - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		spriteOriginYEditBox->setSize(newSize.x / 2.0f - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		ingameSpriteSizeXEditBox->setSize(newSize.x / 2.0f - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		ingameSpriteSizeYEditBox->setSize(newSize.x / 2.0f - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	});

	updateWindowView();
}

bool SpriteSheetMetafileEditor::updateTime(tgui::Duration elapsedTime) {
	bool ret = tgui::Panel::updateTime(elapsedTime);

	timeUntilWorldSelectionColorsChange -= elapsedTime.asSeconds();
	if (timeUntilWorldSelectionColorsChange <= 0) {
		timeUntilWorldSelectionColorsChange = WORLD_SELECTION_COLORS_CHANGE_INTERVAL;

		if (selectionRectangleColor == tgui::Color::Black) {
			selectionRectangleColor = tgui::Color::White;
			worldSelectionCursorColor = tgui::Color::Black;
		} else {
			selectionRectangleColor = tgui::Color::Black;
			worldSelectionCursorColor = tgui::Color::White;
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
	// Adjust to account for the window's view (idk why this works)
	sf::View offsetView = viewFromViewController;
	offsetView.setCenter(offsetView.getCenter() + getAbsolutePosition());

	renderTarget.setView(offsetView);

	// 1 pixel on the screen is equal to this much in world coordinates
	sf::Vector2f scale = renderTarget.mapPixelToCoords(sf::Vector2i(1, 1)) - renderTarget.mapPixelToCoords(sf::Vector2i(0, 0));
	scale.x = std::abs(scale.x);
	scale.y = std::abs(scale.y);

	renderTarget.draw(transparentTextureSprite, sfmlStates);
	if (loadedTexture) {
		sfmlTarget.drawSprite(states, fullTextureAsSprite);

		// Create borders such that they're always 2 pixels in each direction
		tgui::Borders borders = { scale.x * SELECTION_RECTANGLE_WIDTH, scale.x * SELECTION_RECTANGLE_WIDTH, 
			scale.y * SELECTION_RECTANGLE_WIDTH, scale.y * SELECTION_RECTANGLE_WIDTH };

		for (auto rect : selectionRectangles) {
			states.transform.translate(rect.getPosition());
			sfmlTarget.drawBorders(states, borders, rect.getSize(), selectionRectangleColor);
			states.transform.translate(-rect.getPosition());
		}

		if (selectedSpriteData) {
			// Draw selected sprite's origin location

			sf::IntRect rect = selectedSpriteData->getArea();
			float diameter = scale.x * WORLD_SELECTION_CURSOR_DIAMETER;
			sf::Vector2f circlePos = sf::Vector2f(selectedSpriteData->getSpriteOriginX() - diameter / 2.0f, selectedSpriteData->getSpriteOriginY() - diameter / 2.0f);
			states.transform.translate(sf::Vector2f(rect.left + circlePos.x, rect.top + circlePos.y));
			// Assume scale.x == scale.y (that the view isn't stretched in some way)
			sfmlTarget.drawCircle(states, diameter, sf::Color::Transparent, scale.x * WORLD_SELECTION_CURSOR_BORDER_THICKNESS, selectionRectangleColor);
			states.transform.translate(-sf::Vector2f(rect.left + circlePos.x, rect.top + circlePos.y));
		}
	}
	if (selectingSpriteRectTopLeft || selectingSpriteRectBottomRight || selectingSpriteRectOrigin) {
		// Draw the world selection cursor (a circle) at the mouse, rounded to the nearest int in world coordinates

		sf::Vector2f mouseWorldRoundedPos = mainEditorWindow.getWindow()->mapPixelToCoords(mainEditorWindow.getMousePos(), viewFromViewController);
		if (!selectingSpriteRectOrigin) {
			// Sprite origin is a float, not an int

			mouseWorldRoundedPos.x = std::lround(mouseWorldRoundedPos.x);
			mouseWorldRoundedPos.y = std::lround(mouseWorldRoundedPos.y);
		}

		float diameter = scale.x * WORLD_SELECTION_CURSOR_DIAMETER;
		sf::Vector2f circlePos = sf::Vector2f(mouseWorldRoundedPos.x - diameter / 2.0f, mouseWorldRoundedPos.y - diameter / 2.0f);
		states.transform.translate(circlePos);
		// Assume scale.x == scale.y (that the view isn't stretched in some way)
		sfmlTarget.drawCircle(states, diameter, sf::Color::Transparent, scale.x *WORLD_SELECTION_CURSOR_BORDER_THICKNESS, worldSelectionCursorColor);
		states.transform.translate(-circlePos);

		if (selectingSpriteRectBottomRight) {
			// Draw a rectangle from the rect's top-left to cursor world pos
		
			// Same borders size as selectionRectangles
			tgui::Borders borders = { scale.x * SELECTION_RECTANGLE_WIDTH, scale.x * SELECTION_RECTANGLE_WIDTH,
				scale.y * SELECTION_RECTANGLE_WIDTH, scale.y * SELECTION_RECTANGLE_WIDTH };

			sf::RectangleShape rect;
			rect.setPosition(spriteRectTopLeftSet.x, spriteRectTopLeftSet.y);
			rect.setSize(mouseWorldRoundedPos - sf::Vector2f(spriteRectTopLeftSet));

			states.transform.translate(rect.getPosition());
			sfmlTarget.drawBorders(states, borders, rect.getSize(), selectionRectangleColor);
			states.transform.translate(-rect.getPosition());
		} else {
			// Draw lines indicating x/y components of the cursor position

			float rectangleWidth = scale.x * WORLD_SELECTION_CURSOR_BORDER_THICKNESS / 2.0f;
			sf::Vector2f visibleWorldSize = mainEditorWindow.getWindow()->mapPixelToCoords(sf::Vector2i(getSize().x, getSize().y), viewFromViewController)
				- mainEditorWindow.getWindow()->mapPixelToCoords(sf::Vector2i(0, 0), viewFromViewController);

			sf::Vector2f horizontalLinePos = sf::Vector2f(mouseWorldRoundedPos.x - visibleWorldSize.x, mouseWorldRoundedPos.y - rectangleWidth / 2.0f);
			states.transform.translate(horizontalLinePos);
			sfmlTarget.drawFilledRect(states, sf::Vector2f(visibleWorldSize.x * 2.0f, rectangleWidth), worldSelectionCursorColor);
			states.transform.translate(-horizontalLinePos);

			sf::Vector2f verticalLinePos = sf::Vector2f(mouseWorldRoundedPos.x - rectangleWidth / 2.0f, mouseWorldRoundedPos.y - visibleWorldSize.y);
			states.transform.translate(verticalLinePos);
			sfmlTarget.drawFilledRect(states, sf::Vector2f(rectangleWidth, visibleWorldSize.y * 2.0f), worldSelectionCursorColor);
			states.transform.translate(-verticalLinePos);
		}
	}
	renderTarget.setView(originalView);

	for (auto widget : getWidgets()) {
		states.transform.translate(widget->getPosition());
		widget->draw(target, states);
		states.transform.translate(-widget->getPosition());
	}
}

CopyOperationResult SpriteSheetMetafileEditor::copyFrom() {
	if (selectedSpriteData) {
		return CopyOperationResult(std::make_shared<CopiedSpriteData>(getID(), selectedSpriteData), "Copied the selected sprite");
	} else if (selectedAnimationData) {
		return CopyOperationResult(std::make_shared<CopiedAnimationData>(getID(), selectedAnimationData), "Copied the selected animation");
	}
	return CopyOperationResult(nullptr, "");
}

PasteOperationResult SpriteSheetMetafileEditor::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	if (std::static_pointer_cast<CopiedSpriteData>(pastedObject)) {
		this->mainEditorWindow.promptInput("Enter name of new pasted sprite:", 
			std::static_pointer_cast<CopiedSpriteData>(pastedObject)->getSpriteData(), animatablesListView.get(), true)->sink()
			.connect<SpriteSheetMetafileEditor, &SpriteSheetMetafileEditor::onNewPastedSpriteNameInput>(this);
		return PasteOperationResult(true, "Pasted the selected sprite");
	} else if (std::static_pointer_cast<CopiedAnimationData>(pastedObject)) {
		this->mainEditorWindow.promptInput("Enter name of new pasted animation:",
			std::static_pointer_cast<CopiedAnimationData>(pastedObject)->getAnimationData(), animatablesListView.get(), true)->sink()
			.connect<SpriteSheetMetafileEditor, &SpriteSheetMetafileEditor::onNewPastedAnimationNameInput>(this);
		return PasteOperationResult(true, "Pasted the selected animation");
	}
	return PasteOperationResult(false, "");
}

PasteOperationResult SpriteSheetMetafileEditor::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	if (std::static_pointer_cast<CopiedSpriteData>(pastedObject) && selectedSpriteData) {
		std::shared_ptr<SpriteData> pastedSprite = std::static_pointer_cast<CopiedSpriteData>(pastedObject)->getSpriteData();
		if (*selectedSpriteData == *pastedSprite) {
			return PasteOperationResult(false, "");
		}

		std::weak_ptr<SpriteData> weakCaptureOfSelectedSpriteData = std::weak_ptr<SpriteData>(selectedSpriteData);
		std::shared_ptr<SpriteData> copyOfSelectedSpriteData = std::make_shared<SpriteData>(selectedSpriteData);
		undoStack.execute(UndoableCommand([this, pastedSprite, weakCaptureOfSelectedSpriteData, copyOfSelectedSpriteData]() {
			std::shared_ptr<SpriteData> selectedSpriteData = weakCaptureOfSelectedSpriteData.lock();
			selectedSpriteData->load(pastedSprite->format());
			selectedSpriteData->setSpriteName(copyOfSelectedSpriteData->getSpriteName());

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
			// Select the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(selectedSpriteData->getSpriteName()));
		}, [this, copyOfSelectedSpriteData, weakCaptureOfSelectedSpriteData, pastedSprite]() {
			std::shared_ptr<SpriteData> selectedSpriteData = weakCaptureOfSelectedSpriteData.lock();
			selectedSpriteData->load(copyOfSelectedSpriteData->format());

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
			// Select the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(selectedSpriteData->getSpriteName()));
		}));

		return PasteOperationResult(true, "Replaced the selected sprite");
	} else if (std::static_pointer_cast<CopiedAnimationData>(pastedObject) && selectedAnimationData) {
		std::shared_ptr<AnimationData> pastedAnimation = std::static_pointer_cast<CopiedAnimationData>(pastedObject)->getAnimationData();
		if (*selectedAnimationData == *pastedAnimation) {
			return PasteOperationResult(false, "");
		}

		std::weak_ptr<AnimationData> weakCaptureOfSelectedAnimationData = std::weak_ptr<AnimationData>(selectedAnimationData);
		std::shared_ptr<AnimationData> copyOfSelectedAnimationData = std::make_shared<AnimationData>(selectedAnimationData);
		undoStack.execute(UndoableCommand([this, pastedAnimation, weakCaptureOfSelectedAnimationData, copyOfSelectedAnimationData]() {
			std::shared_ptr<AnimationData> selectedAnimationData = weakCaptureOfSelectedAnimationData.lock();
			selectedAnimationData->load(pastedAnimation->format());
			selectedAnimationData->setAnimationName(copyOfSelectedAnimationData->getAnimationName());

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
			// Select the animation
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewAnimationItemId(selectedAnimationData->getAnimationName()));
		}, [this, copyOfSelectedAnimationData, weakCaptureOfSelectedAnimationData, pastedAnimation]() {
			std::shared_ptr<AnimationData> selectedAnimationData = weakCaptureOfSelectedAnimationData.lock();
			selectedAnimationData->load(copyOfSelectedAnimationData->format());

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
			// Select the animation
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewAnimationItemId(selectedAnimationData->getAnimationName()));
		}));

		return PasteOperationResult(true, "Replaced the selected animation");
	}
	return PasteOperationResult(false, "");
}

bool SpriteSheetMetafileEditor::handleEvent(sf::Event event) {
	if (viewController->handleEvent(viewFromViewController, event)) {
		return true;
	} else if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				undoStack.undo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				undoStack.redo();
				return true;
			} else if (event.key.code == sf::Keyboard::C) {
				clipboard.copy(this);
				return true;
			} else if (event.key.code == sf::Keyboard::V) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
					clipboard.paste2(this);
				} else {
					clipboard.paste(this);
				}
				return true;
			}
		} else {
			if (event.key.code == sf::Keyboard::B) {
				add(backgroundColorPicker);
				return true;
			} else if (event.key.code == sf::Keyboard::P) {
				add(animatablePreviewChildWindow);
				return true;
			} else if (event.key.code == sf::Keyboard::G) {
				resetCamera();
				return true;
			} else if (event.key.code == sf::Keyboard::Escape) {
				stopEditingSpriteProperties();
				return true;
			} else if (event.key.code == sf::Keyboard::Delete) {
				deleteSelectedAnimatable();
				return true;
			}
		}
	} else if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Left) {
			onLeftClick(event.mouseButton.x, event.mouseButton.y);
		}
	}
	return false;
}

bool SpriteSheetMetafileEditor::loadSpriteSheet(const std::string& levelPackName, std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<SpriteSheet> spriteSheet) {
	animatablePreviewPicture->setEmptyAnimatable();
	bool success = loadImage(levelPackName, spriteLoader, spriteSheet->getName());
	if (!success) {
		return false;
	}

	repopulateAnimatablesListView();

	viewFromViewController.setCenter(loadedTexture->getSize().x / 2.0f, loadedTexture->getSize().y / 2.0f);
	updateWindowView();

	return true;
}

bool SpriteSheetMetafileEditor::loadImage(const std::string& levelPackName, std::shared_ptr<SpriteLoader> spriteLoader, std::string spriteSheetName) {
	this->spriteLoader = std::make_shared<SpriteLoader>(levelPackName);
	bool successfulLoad = this->spriteLoader->loadSpriteSheet(spriteSheetName);
	if (!successfulLoad) {
		return false;
	}
	this->spriteSheet = this->spriteLoader->getSpriteSheet(spriteSheetName);
	
	if (animatablesListView->getSelectedItemIndex() != -1) {
		tgui::String id = animatablesListView->getSelectedItemId();

		if (id.at(0) == ANIMATABLES_LIST_SPRITE_INDICATOR) {
			std::string spriteName = static_cast<std::string>(id.substr(1));
			if (spriteSheet->hasSpriteData(spriteName)) {
				onSpriteSelect(spriteSheet->getSpriteData(spriteName));
			}
		} else if (id.at(0) == ANIMATABLES_LIST_ANIMATION_INDICATOR) {
			std::string animationName = static_cast<std::string>(id.substr(1));
			if (spriteSheet->hasAnimationData(animationName)) {
				onAnimationSelect(spriteSheet->getAnimationData(animationName));
			}
		}
	}

	// Load texture from spriteLoader rather than this->spriteSheet because this->spriteSheet's 
	// texture isn't guaranteed to be loaded
	loadedTexture = spriteLoader->getSpriteSheet(spriteSheetName)->getTexture();
	fullTextureAsSprite = tgui::Sprite(*loadedTexture);

	updateWindowView();
	viewController->setMaxCameraZoom(viewFromViewController, std::min(loadedTexture->getSize().x, loadedTexture->getSize().y) / MAX_CAMERA_ZOOM_SCALAR);
	scaleTransparentTextureToCameraZoom(viewController->getZoomAmount());

	resetCamera();

	return true;
}

void SpriteSheetMetafileEditor::repopulateAnimatablesListView() {
	animatablesListView->removeAllItems();
	for (std::pair<std::string, std::shared_ptr<SpriteData>> item : spriteSheet->getSpriteData()) {
		animatablesListView->addItem(format("[S] %s", item.second->getSpriteName().c_str()), getAnimatablesListViewSpriteItemId(item.second->getSpriteName()));
	}
	for (std::pair<std::string, std::shared_ptr<AnimationData>> item : spriteSheet->getAnimationData()) {
		animatablesListView->addItem(format("[A] %s", item.second->getAnimationName().c_str()), getAnimatablesListViewAnimationItemId(item.second->getAnimationName()));
	}

	onAnimatableDeselect();
}

void SpriteSheetMetafileEditor::resetCamera() {
	if (loadedTexture) {
		viewFromViewController.setCenter(loadedTexture->getSize().x / 2.0f, loadedTexture->getSize().y / 2.0f);
	} else {
		viewFromViewController.setCenter(0, 0);
	}
}

void SpriteSheetMetafileEditor::scaleTransparentTextureToCameraZoom(float cameraZoomAmount) {
	float scale = 4.0f / cameraZoomAmount;

	transparentTextureSprite.setScale(scale, scale);
	if (loadedTexture) {
		transparentTextureSprite.setTextureRect(sf::IntRect(0, 0, loadedTexture->getSize().x / scale, loadedTexture->getSize().y / scale));
	}
}

void SpriteSheetMetafileEditor::reselectSelectedAnimatable() {
	if (animatablesListView->getSelectedItemIndex() != -1) {
		tgui::String selectedItemId = animatablesListView->getSelectedItemId();
		animatablesListView->deselectItem();
		animatablesListView->setSelectedItemById(selectedItemId);
	}
}

void SpriteSheetMetafileEditor::reloadPreviewAnimation() {
	if (spriteLoader && selectedAnimationData) {
		animatablePreviewChildWindow->setTitle(format("%s preview", selectedAnimationData->getAnimationName().c_str()));
		animatablePreviewPicture->setAnimation(*spriteLoader, selectedAnimationData->getAnimationName(), spriteSheet->getName());
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

std::shared_ptr<SpriteLoader> SpriteSheetMetafileEditor::getSpriteLoader() const {
	return spriteLoader;
}

std::shared_ptr<SpriteSheet> SpriteSheetMetafileEditor::getSpriteSheet() const {
	return spriteSheet;
}

void SpriteSheetMetafileEditor::updateWindowView() {
	if (!loadedTexture) {
		return;
	}

	auto worldViewOffset = getWorldViewOffsetInPixels();

	auto windowSize = mainEditorWindow.getWindow()->getSize();
	auto size = getSize() - worldViewOffset;
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

	float viewportX = (getAbsolutePosition().x + worldViewOffset.x) / windowSize.x;
	float viewportY = (getAbsolutePosition().y + worldViewOffset.y) / windowSize.y;
	float viewportWidth = (getSize().x - worldViewOffset.x) / windowSize.x;
	float viewportHeight = (getSize().y - worldViewOffset.y) / windowSize.y;
	viewportFloatRect = sf::FloatRect(viewportX, viewportY, viewportWidth, viewportHeight);

	sf::Vector2f oldCenter = viewFromViewController.getCenter();
	viewController->setViewZone(viewFromViewController, viewFloatRect);
	viewFromViewController.setViewport(viewportFloatRect);
	viewFromViewController.setCenter(oldCenter);

	backgroundSprite.setScale(getSize());
}

void SpriteSheetMetafileEditor::beginSelectingSpriteRectTopLeft() {
	selectingSpriteRectTopLeft = true;
	selectingSpriteRectBottomRight = false;
	selectingSpriteRectOrigin = false;

	selectionRectangles.clear();
}

void SpriteSheetMetafileEditor::beginSelectingSpriteRectBottomRight() {
	selectingSpriteRectTopLeft = false;
	selectingSpriteRectBottomRight = true;
	selectingSpriteRectOrigin = false;
}

void SpriteSheetMetafileEditor::beginSelectingSpriteRectOrigin() {
	selectingSpriteRectTopLeft = false;
	selectingSpriteRectBottomRight = false;
	selectingSpriteRectOrigin = true;
}

void SpriteSheetMetafileEditor::stopEditingSpriteProperties() {
	selectingSpriteRectTopLeft = false;
	selectingSpriteRectBottomRight = false;
	selectingSpriteRectOrigin = false;

	// Reselect selected sprite to fully revert to not-editing state
	reselectSelectedAnimatable();
}

void SpriteSheetMetafileEditor::deleteSelectedAnimatable() {
	if (selectedSpriteData) {
		std::shared_ptr<SpriteData> copyOfDeletedSpriteData = std::make_shared<SpriteData>(selectedSpriteData);
		std::string selectedSpriteName = selectedSpriteData->getSpriteName();
		undoStack.execute(UndoableCommand([this, selectedSpriteName]() {
			spriteSheet->deleteSprite(selectedSpriteName);

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
		}, [this, copyOfDeletedSpriteData]() {
			spriteSheet->insertSprite(copyOfDeletedSpriteData->getSpriteName(), copyOfDeletedSpriteData);

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
			// Select the animation
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(copyOfDeletedSpriteData->getSpriteName()));
		}));
	} else if (selectedAnimationData) {
		std::shared_ptr<AnimationData> copyOfDeletedAnimationData = std::make_shared<AnimationData>(selectedAnimationData);
		std::string selectedAnimationName = selectedAnimationData->getAnimationName();
		undoStack.execute(UndoableCommand([this, selectedAnimationName]() {
			spriteSheet->deleteAnimation(selectedAnimationName);

			onAnimationDelete.emit(this, selectedAnimationName);
			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
		}, [this, copyOfDeletedAnimationData]() {
			spriteSheet->insertAnimation(copyOfDeletedAnimationData->getAnimationName(), copyOfDeletedAnimationData);

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
			// Select the animation
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewAnimationItemId(copyOfDeletedAnimationData->getAnimationName()));
		}));
	}
	
}

void SpriteSheetMetafileEditor::onAnimatableDeselect() {
	selectedSpriteData = nullptr;
	selectedAnimationData = nullptr;

	selectionRectangles.clear();
	animatablePreviewPicture->setEmptyAnimatable();
	animatablePreviewChildWindow->setTitle("Empty preview");
	remove(spriteColorPicker);

	chooseSpriteRectButton->setVisible(false);
	chooseSpriteOriginButton->setVisible(false);
	openSpriteColorPickerButton->setVisible(false);
	spriteOriginLabel->setVisible(false);
	spriteOriginXEditBox->setVisible(false);
	spriteOriginYEditBox->setVisible(false);
	ingameSpriteSizeLabel->setVisible(false);
	ingameSpriteSizeXEditBox->setVisible(false);
	ingameSpriteSizeYEditBox->setVisible(false);
}

void SpriteSheetMetafileEditor::onSpriteSelect(std::shared_ptr<SpriteData> spriteData) {
	selectedSpriteData = spriteData;
	selectedAnimationData = nullptr;

	selectionRectangles.clear();
	sf::RectangleShape shape;
	ComparableIntRect spriteRect = spriteData->getArea();
	shape.setSize(sf::Vector2f(spriteRect.width, spriteRect.height));
	shape.setPosition(spriteRect.left, spriteRect.top);
	selectionRectangles.push_back(shape);

	if (spriteLoader) {
		animatablePreviewChildWindow->setTitle(format("%s preview", spriteData->getSpriteName().c_str()));
		animatablePreviewPicture->setSprite(*spriteLoader, spriteData->getSpriteName(), spriteSheet->getName());
	}

	ignoreSignals = true;
	spriteOriginXEditBox->setValue(spriteData->getSpriteOriginX());
	spriteOriginYEditBox->setValue(spriteData->getSpriteOriginY());
	ingameSpriteSizeXEditBox->setValue(spriteData->getSpriteWidth());
	ingameSpriteSizeYEditBox->setValue(spriteData->getSpriteHeight());
	spriteColorPicker->setColor(spriteData->getColor());
	ignoreSignals = false;

	chooseSpriteRectButton->setVisible(true);
	chooseSpriteOriginButton->setVisible(true);
	openSpriteColorPickerButton->setVisible(true);
	spriteOriginLabel->setVisible(true);
	spriteOriginXEditBox->setVisible(true);
	spriteOriginYEditBox->setVisible(true);
	ingameSpriteSizeLabel->setVisible(true);
	ingameSpriteSizeXEditBox->setVisible(true);
	ingameSpriteSizeYEditBox->setVisible(true);
}

void SpriteSheetMetafileEditor::onAnimationSelect(std::shared_ptr<AnimationData> animationData) {
	selectedSpriteData = nullptr;
	selectedAnimationData = animationData;

	selectionRectangles.clear();
	for (std::pair<float, std::string> data : animationData->getSpriteInfo()) {
		if (spriteSheet->hasSpriteData(data.second)) {
			sf::RectangleShape shape;
			ComparableIntRect spriteRect = spriteSheet->getSpriteData(data.second)->getArea();
			shape.setSize(sf::Vector2f(spriteRect.width, spriteRect.height));
			shape.setPosition(spriteRect.left, spriteRect.top);
			selectionRectangles.push_back(shape);
		}
	}

	if (spriteLoader) {
		animatablePreviewChildWindow->setTitle(format("%s preview", animationData->getAnimationName().c_str()));
		animatablePreviewPicture->setAnimation(*spriteLoader, animationData->getAnimationName(), spriteSheet->getName());
	}

	chooseSpriteRectButton->setVisible(false);
	chooseSpriteOriginButton->setVisible(false);
	openSpriteColorPickerButton->setVisible(false);
	spriteOriginLabel->setVisible(false);
	spriteOriginXEditBox->setVisible(false);
	spriteOriginYEditBox->setVisible(false);
	ingameSpriteSizeLabel->setVisible(false);
	ingameSpriteSizeXEditBox->setVisible(false);
	ingameSpriteSizeYEditBox->setVisible(false);
}

void SpriteSheetMetafileEditor::onLeftClick(int mouseX, int mouseY) {
	if (selectingSpriteRectTopLeft) {
		sf::Vector2f mouseWorldPos = mainEditorWindow.getWindow()->mapPixelToCoords(mainEditorWindow.getMousePos(), viewFromViewController);
		int x = std::lround(mouseWorldPos.x);
		int y = std::lround(mouseWorldPos.y);

		if (x < 0 || y < 0 || x > loadedTexture->getSize().x || y > loadedTexture->getSize().y) {
			return;
		}

		spriteRectTopLeftSet = sf::Vector2i(x, y);
		beginSelectingSpriteRectBottomRight();
	} else if (selectingSpriteRectBottomRight) {
		sf::Vector2f mouseWorldPos = mainEditorWindow.getWindow()->mapPixelToCoords(mainEditorWindow.getMousePos(), viewFromViewController);
		sf::Vector2i spriteRectBottomRightSet(std::lround(mouseWorldPos.x), std::lround(mouseWorldPos.y));

		// In case spriteRectTopLeftSet isn't really the top-left of the rect or spriteRectBottomRightSet isn't really the bottom-right of the rect
		sf::Vector2i trueTopLeft(std::min(spriteRectTopLeftSet.x, spriteRectBottomRightSet.x), std::min(spriteRectTopLeftSet.y, spriteRectBottomRightSet.y));
		sf::Vector2i trueBottomRight(std::max(spriteRectTopLeftSet.x, spriteRectBottomRightSet.x), std::max(spriteRectTopLeftSet.y, spriteRectBottomRightSet.y));

		sf::IntRect oldRect = selectedSpriteData->getArea();
		sf::IntRect newRect(trueTopLeft.x, trueTopLeft.y, trueBottomRight.x - trueTopLeft.x, trueBottomRight.y - trueTopLeft.y);
		std::weak_ptr<SpriteData> weakCaptureOfSelectedSpriteData = std::weak_ptr<SpriteData>(selectedSpriteData);
		undoStack.execute(UndoableCommand([this, weakCaptureOfSelectedSpriteData, newRect]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setTextureArea(newRect);

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}, [this, weakCaptureOfSelectedSpriteData, oldRect]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setTextureArea(oldRect);

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}));

		stopEditingSpriteProperties();
	} else if (selectingSpriteRectOrigin) {
		sf::Vector2f newOrigin = mainEditorWindow.getWindow()->mapPixelToCoords(mainEditorWindow.getMousePos(), viewFromViewController) 
			- sf::Vector2f(selectedSpriteData->getArea().left, selectedSpriteData->getArea().top);
		sf::Vector2f oldOrigin = sf::Vector2f(selectedSpriteData->getSpriteOriginX(), selectedSpriteData->getSpriteOriginY());
		
		std::weak_ptr<SpriteData> weakCaptureOfSelectedSpriteData = std::weak_ptr<SpriteData>(selectedSpriteData);
		undoStack.execute(UndoableCommand([this, weakCaptureOfSelectedSpriteData, newOrigin]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteOrigin(newOrigin.x, newOrigin.y);

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}, [this, weakCaptureOfSelectedSpriteData, oldOrigin]() {
			std::shared_ptr<SpriteData> spriteData = weakCaptureOfSelectedSpriteData.lock();
			spriteData->setSpriteOrigin(oldOrigin.x, oldOrigin.y);

			onMetafileModify.emit(this, spriteSheet);

			// Reselect the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteData->getSpriteName()));
		}));

		stopEditingSpriteProperties();
	}
}

void SpriteSheetMetafileEditor::onNewSpriteNameInput(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, std::string spriteName) {
	if (choice == EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES) {
		// Sprite name can't be empty
		if (spriteName.empty()) {
			mainEditorWindow.showPopupMessageWindow("Sprite name cannot be empty.", animatablesListView.get());
			return;
		}

		// Check if sprite name already exists
		if (spriteSheet->hasSpriteData(spriteName)) {
			mainEditorWindow.showPopupMessageWindow("Sprite name already exists.", animatablesListView.get());
			return;
		}

		undoStack.execute(UndoableCommand([this, spriteName]() {
			std::shared_ptr<SpriteData> newSprite = std::make_shared<SpriteData>(spriteName, 
				sf::IntRect{0, 0, 0, 0}, 0, 0, 0, 0, sf::Color(255, 255, 255, 255));
			spriteSheet->insertSprite(spriteName, newSprite);

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
			// Select the sprite
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(spriteName));
		}, [this, spriteName]() {
			spriteSheet->deleteSprite(spriteName);

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
		}));
	}
}

void SpriteSheetMetafileEditor::onNewAnimationNameInput(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, std::string animationName) {
	if (choice == EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES) {
		// Animation name can't be empty
		if (animationName.empty()) {
			mainEditorWindow.showPopupMessageWindow("Animation name cannot be empty.", animatablesListView.get());
			return;
		}

		// Check if sprite name already exists
		if (spriteSheet->hasAnimationData(animationName)) {
			mainEditorWindow.showPopupMessageWindow("Animation name already exists.", animatablesListView.get());
			return;
		}

		undoStack.execute(UndoableCommand([this, animationName]() {
			std::shared_ptr<AnimationData> newAnimation = std::make_shared<AnimationData>(animationName, 
				std::vector<std::pair<float, std::string>>());
			spriteSheet->insertAnimation(animationName, newAnimation);

			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
			// Select the animation
			animatablesListView->deselectItem();
			animatablesListView->setSelectedItemById(getAnimatablesListViewAnimationItemId(animationName));
		}, [this, animationName]() {
			spriteSheet->deleteAnimation(animationName);

			onAnimationDelete.emit(this, animationName);
			onMetafileModify.emit(this, spriteSheet);

			repopulateAnimatablesListView();
		}));
	}
}

void SpriteSheetMetafileEditor::onNewPastedSpriteNameInput(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice,
	std::string spriteName, std::shared_ptr<SpriteData> pastedSprite) {

	if (choice != EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES) {
		return;
	}

	// Sprite name can't be empty
	if (spriteName.empty()) {
		mainEditorWindow.showPopupMessageWindow("Sprite name cannot be empty.", animatablesListView.get());
		return;
	}

	// Check if sprite name already exists
	if (spriteSheet->hasSpriteData(spriteName)) {
		mainEditorWindow.showPopupMessageWindow("Sprite name already exists.", animatablesListView.get());
		return;
	}

	pastedSprite->setSpriteName(spriteName);
	undoStack.execute(UndoableCommand([this, pastedSprite]() {
		spriteSheet->insertSprite(pastedSprite->getSpriteName(), pastedSprite);

		onMetafileModify.emit(this, spriteSheet);

		repopulateAnimatablesListView();
		// Select the sprite
		animatablesListView->deselectItem();
		animatablesListView->setSelectedItemById(getAnimatablesListViewSpriteItemId(pastedSprite->getSpriteName()));
	}, [this, pastedSprite]() {
		spriteSheet->deleteSprite(pastedSprite->getSpriteName());

		onMetafileModify.emit(this, spriteSheet);

		repopulateAnimatablesListView();
	}));
}

void SpriteSheetMetafileEditor::onNewPastedAnimationNameInput(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, 
	std::string animationName, std::shared_ptr<AnimationData> pastedAnimation) {

	if (choice != EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES) {
		return;
	}

	// Animation name can't be empty
	if (animationName.empty()) {
		mainEditorWindow.showPopupMessageWindow("Animation name cannot be empty.", animatablesListView.get());
		return;
	}

	// Check if sprite name already exists
	if (spriteSheet->hasAnimationData(animationName)) {
		mainEditorWindow.showPopupMessageWindow("Animation name already exists.", animatablesListView.get());
		return;
	}

	pastedAnimation->setAnimationName(animationName);
	undoStack.execute(UndoableCommand([this, pastedAnimation]() {
		spriteSheet->insertAnimation(pastedAnimation->getAnimationName(), pastedAnimation);

		onMetafileModify.emit(this, spriteSheet);

		repopulateAnimatablesListView();
		// Select the animation
		animatablesListView->deselectItem();
		animatablesListView->setSelectedItemById(getAnimatablesListViewAnimationItemId(pastedAnimation->getAnimationName()));
	}, [this, pastedAnimation]() {
		spriteSheet->deleteAnimation(pastedAnimation->getAnimationName());

		onAnimationDelete.emit(this, pastedAnimation->getAnimationName());
		onMetafileModify.emit(this, spriteSheet);

		repopulateAnimatablesListView();
	}));
}

sf::Vector2f SpriteSheetMetafileEditor::getWorldViewOffsetInPixels() const {
	return sf::Vector2f(animatablesListView->getPosition().x + animatablesListView->getSize().x,
		utilityWidgetsPanel->getPosition().y + utilityWidgetsPanel->getSize().y);
}

std::string SpriteSheetMetafileEditor::getAnimatablesListViewSpriteItemId(std::string spriteName) const {
	return format("%c%s", ANIMATABLES_LIST_SPRITE_INDICATOR, spriteName.c_str());
}

std::string SpriteSheetMetafileEditor::getAnimatablesListViewAnimationItemId(std::string animationName) const {
	return format("%c%s", ANIMATABLES_LIST_ANIMATION_INDICATOR, animationName.c_str());
}
