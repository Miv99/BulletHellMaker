#include <Editor/Windows/LevelPackObjectPreviewWindow.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>
#include <Editor/LevelPackObjectList/LevelPackObjectsListPanel.h>
#include <Editor/Previewer/LevelPackObjectPreviewPanel.h>
#include <Editor/CustomWidgets/SliderWithEditBox.h>

LevelPackObjectPreviewWindow::LevelPackObjectPreviewWindow(std::string windowTitle, int width, int height, std::string levelPackName, std::shared_ptr<SpriteLoader> spriteLoader,
	bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval)
	: EditorWindow(windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval), levelPackName(levelPackName), spriteLoader(spriteLoader) {
}

void LevelPackObjectPreviewWindow::reopenWindow() {
	if (!window || !window->isOpen()) {
		std::thread previewWindowThread = std::thread(&LevelPackObjectPreviewWindow::start, &(*this));
		previewWindowThread.detach();
	}
}

void LevelPackObjectPreviewWindow::loadLevelPack(std::string levelPackName, std::shared_ptr<SpriteLoader> spriteLoader) {
	this->levelPackName = levelPackName;
	this->spriteLoader = spriteLoader;
	if (previewPanel) {
		previewPanel->loadLevelPack(levelPackName, spriteLoader);
	}
}

void LevelPackObjectPreviewWindow::previewNothing() {
	if (window && window->isOpen()) {
		previewObjectLabel->setText("Nothing being previewed");

		if (previewThread.joinable()) {
			previewThread.join();
		}
		previewThread = std::thread(&LevelPackObjectPreviewPanel::previewNothing, &(*previewPanel));
		previewThread.detach();
	}
}

void LevelPackObjectPreviewWindow::previewAttack(const std::shared_ptr<EditorAttack> attack) {
	if (window && window->isOpen() && !lockCurrentPreview) {
		previewObjectLabel->setText("Attack ID " + std::to_string(attack->getID()));

		if (previewThread.joinable()) {
			previewThread.join();
		}
		previewThread = std::thread(&LevelPackObjectPreviewPanel::previewAttack, &(*previewPanel), attack);
		previewThread.detach();
	}
}

void LevelPackObjectPreviewWindow::previewAttackPattern(const std::shared_ptr<EditorAttackPattern> attackPattern) {
	if (window && window->isOpen() && !lockCurrentPreview) {
		previewObjectLabel->setText("Attack Pattern ID " + std::to_string(attackPattern->getID()));

		if (previewThread.joinable()) {
			previewThread.join();
		}
		previewThread = std::thread(&LevelPackObjectPreviewPanel::previewAttackPattern, &(*previewPanel), attackPattern);
		previewThread.detach();
	}
}

void LevelPackObjectPreviewWindow::resetPreview() {
	if (window && window->isOpen()) {
		previewPanel->resetPreview();
	}
}

void LevelPackObjectPreviewWindow::onOriginalLevelPackAttackModified(const std::shared_ptr<EditorAttack> attack) {
	if (window && window->isOpen()) {
		previewPanel->getLevelPack()->updateAttack(attack);
	}
}

void LevelPackObjectPreviewWindow::onOriginalLevelPackAttackPatternModified(const std::shared_ptr<EditorAttackPattern> attackPattern) {
	if (window && window->isOpen()) {
		previewPanel->getLevelPack()->updateAttackPattern(attackPattern);
	}
}

void LevelPackObjectPreviewWindow::onOriginalLevelPackSpriteSheetModified(const std::shared_ptr<SpriteSheet> spriteSheet) {
	if (window && window->isOpen()) {
		previewPanel->getLevelPack()->updateSpriteSheet(spriteSheet);
	}
}

void LevelPackObjectPreviewWindow::deleteAttack(int id) {
	if (window && window->isOpen()) {
		previewPanel->getLevelPack()->deleteAttack(id);
	}
}

void LevelPackObjectPreviewWindow::deleteAttackPattern(int id) {
	if (window && window->isOpen()) {
		previewPanel->getLevelPack()->deleteAttackPattern(id);
	}
}

void LevelPackObjectPreviewWindow::updateWidgetsPositionsAndSizes() {
	if (infoPanel->isVisible()) {
		infoPanel->setSize("30%", "100%");
		if (logs->isVisible()) {
			previewPanel->setSize("70%", "60%");
		} else {
			previewPanel->setSize("70%", "100%");
		}
		previewPanel->setPosition("30%", 0);
	} else {
		if (logs->isVisible()) {
			previewPanel->setSize("100%", "60%");
		} else {
			previewPanel->setSize("100%", "100%");
		}
		previewPanel->setPosition(0, 0);
	}
}

bool LevelPackObjectPreviewWindow::handleEvent(sf::Event event) {
	if (EditorWindow::handleEvent(event)) {
		return true;
	} else if (previewPanel->handleEvent(event)) {
		return true;
	} else if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::I) {
			infoPanel->setVisible(!infoPanel->isVisible());
			updateWidgetsPositionsAndSizes();
			return true;
		} else if (event.key.code == sf::Keyboard::L) {
			logs->setVisible(!logs->isVisible());
			updateWidgetsPositionsAndSizes();
			return true;
		} else if (event.key.code == sf::Keyboard::T) {
			previewPanel->setUseDebugRenderSystem(!useDebugRenderSystem);
			ignoreSignals = true;
			useDebugRenderSystem->setChecked(previewPanel->getUseDebugRenderSystem());
			ignoreSignals = false;

			return true;
		} else if (event.key.code == sf::Keyboard::G) {
			previewPanel->resetCamera();
			return true;
		} else if (event.key.code == sf::Keyboard::R) {
			previewPanel->resetPreview();
			return true;
		} else if (event.key.code == sf::Keyboard::Escape) {
			previewNothing();
			return true;
		}
	}
	return previewPanel->handleEvent(event);
}

void LevelPackObjectPreviewWindow::onRenderWindowInitialization() {
	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	previewPanel = LevelPackObjectPreviewPanel::create(*this, levelPackName, spriteLoader);
	previewPanel->setSize("100%", "100%");
	gui->add(previewPanel);

	infoPanel = tgui::Panel::create();
	infoPanel->setVisible(true);
	gui->add(infoPanel);

	previewObjectLabel = tgui::Label::create();
	delayLabel = tgui::Label::create();
	delay = NumericalEditBoxWithLimits::create();
	timeMultiplierLabel = tgui::Label::create();
	timeMultiplier = SliderWithEditBox::create(false);
	resetPreviewButton = tgui::Button::create();
	resetCameraButton = tgui::Button::create();
	setPlayerSpawnButton = tgui::Button::create();
	setSourceButton = tgui::Button::create();
	useDebugRenderSystem = tgui::CheckBox::create();
	lockCurrentPreviewCheckBox = tgui::CheckBox::create();
	invinciblePlayer = tgui::CheckBox::create();

	delay->setMin(0);
	delay->setValue(previewPanel->getAttackLoopDelay());
	timeMultiplier->setMin(0, false);
	timeMultiplier->setMax(1, false);
	timeMultiplier->setStep(0.01f);
	timeMultiplier->setValue(previewPanel->getTimeMultiplier());
	useDebugRenderSystem->setChecked(previewPanel->getUseDebugRenderSystem());
	lockCurrentPreviewCheckBox->setChecked(lockCurrentPreview);

	delay->onValueChange.connect([this](float value) {
		if (ignoreSignals) {
			return;
		}

		previewPanel->setAttackLoopDelay(value);
	});
	timeMultiplier->onValueChange.connect([this](float value) {
		if (ignoreSignals) {
			return;
		}

		previewPanel->setTimeMultiplier(value);
	});
	resetPreviewButton->onPress.connect([this]() {
		previewPanel->resetPreview();
	});
	resetCameraButton->onPress.connect([this]() {
		previewPanel->resetCamera();
	});
	setPlayerSpawnButton->onPress.connect([this]() {
		previewPanel->setSettingPlayerSpawn(true);
	});
	setSourceButton->onPress.connect([this]() {
		previewPanel->setSettingSource(true);
	});
	useDebugRenderSystem->onChange.connect([this](bool checked) {
		if (ignoreSignals) {
			return;
		}

		previewPanel->setUseDebugRenderSystem(checked);
	});
	lockCurrentPreviewCheckBox->onChange.connect([this](bool checked) {
		if (ignoreSignals) {
			return;
		}

		this->lockCurrentPreview = checked;
	});
	invinciblePlayer->onChange.connect([this](bool checked) {
		if (ignoreSignals) {
			return;
		}

		previewPanel->setInvinciblePlayer(checked);
	});

	useDebugRenderSystem->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	lockCurrentPreviewCheckBox->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	invinciblePlayer->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);

	previewObjectLabel->setTextSize(TEXT_SIZE);
	delayLabel->setTextSize(TEXT_SIZE);
	delay->setTextSize(TEXT_SIZE);
	timeMultiplierLabel->setTextSize(TEXT_SIZE);
	timeMultiplier->setTextSize(TEXT_SIZE);
	resetPreviewButton->setTextSize(TEXT_SIZE);
	resetCameraButton->setTextSize(TEXT_SIZE);
	setPlayerSpawnButton->setTextSize(TEXT_SIZE);
	setSourceButton->setTextSize(TEXT_SIZE);
	useDebugRenderSystem->setTextSize(TEXT_SIZE);
	lockCurrentPreviewCheckBox->setTextSize(TEXT_SIZE);
	invinciblePlayer->setTextSize(TEXT_SIZE);

	delayLabel->setText("Attack/Attack pattern repeat interval");
	timeMultiplierLabel->setText("Time multiplier");
	resetPreviewButton->setText("Reset preview");
	resetCameraButton->setText("Reset camera");
	setPlayerSpawnButton->setText("Set player spawn");
	setSourceButton->setText("Set preview source");
	useDebugRenderSystem->setText("Hitboxes visible");
	lockCurrentPreviewCheckBox->setText("Lock current preview");
	invinciblePlayer->setText("Invincible player");

	delayLabel->setToolTip(createToolTip("Seconds before the attack or attack pattern being previewed is executed again."));
	useDebugRenderSystem->setToolTip(createToolTip("If this is checked, all hitboxes will be visible and sprites will be drawn outside the map boundaries \
but shader effects (such as piercing bullets flashing after hitting a player) will be unavailable due to technical limitations. Note that bullets with collision action \
\"Destroy self only\" only become invisible after hitting a player, so their hitboxes will still be visible even though they cannot interact with the player again."));
	lockCurrentPreviewCheckBox->setToolTip(createToolTip("While this is checked, new objects will not be previewed."));
	setPlayerSpawnButton->setToolTip(createToolTip("Sets the player's spawn position for future previews to be the next clicked position. The spawn position must be within the map boundaries."));
	setSourceButton->setToolTip(createToolTip("Sets the preview source position for future previews to be the next clicked position. The spawn position can be outside the map boundaries."));
	invinciblePlayer->setToolTip(createToolTip("While this is checked, the player cannot take damage."));

	previewObjectLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	delayLabel->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(previewObjectLabel) + GUI_PADDING_Y);
	delay->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(delayLabel) + GUI_LABEL_PADDING_Y);
	timeMultiplierLabel->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(delay) + GUI_PADDING_Y);
	timeMultiplier->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(timeMultiplierLabel) + GUI_LABEL_PADDING_Y);
	resetPreviewButton->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(timeMultiplier) + GUI_PADDING_Y);
	resetCameraButton->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(resetPreviewButton) + GUI_PADDING_Y);
	setPlayerSpawnButton->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(resetCameraButton) + GUI_PADDING_Y);
	setSourceButton->setPosition(tgui::bindRight(setPlayerSpawnButton) + GUI_PADDING_X, tgui::bindTop(setPlayerSpawnButton));
	useDebugRenderSystem->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(setSourceButton) + GUI_PADDING_Y);
	lockCurrentPreviewCheckBox->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(useDebugRenderSystem) + GUI_PADDING_Y);
	invinciblePlayer->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(lockCurrentPreviewCheckBox) + GUI_PADDING_Y);

	infoPanel->onSizeChange.connect([this](sf::Vector2f newSize) {
		delay->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		timeMultiplier->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		resetPreviewButton->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BUTTON_HEIGHT);
		resetCameraButton->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BUTTON_HEIGHT);
		setPlayerSpawnButton->setSize((newSize.x - GUI_PADDING_X * 3) / 2.0f, TEXT_BUTTON_HEIGHT);
		setSourceButton->setSize((newSize.x - GUI_PADDING_X * 3) / 2.0f, TEXT_BUTTON_HEIGHT);
	});
	infoPanel->setSize("30%", "100%");

	infoPanel->add(previewObjectLabel);
	infoPanel->add(delayLabel);
	infoPanel->add(delay);
	infoPanel->add(timeMultiplierLabel);
	infoPanel->add(timeMultiplier);
	infoPanel->add(resetPreviewButton);
	infoPanel->add(resetCameraButton);
	infoPanel->add(setPlayerSpawnButton);
	infoPanel->add(setSourceButton);
	infoPanel->add(useDebugRenderSystem);
	infoPanel->add(lockCurrentPreviewCheckBox);
	infoPanel->add(invinciblePlayer);

	logs = tgui::TextArea::create();
	logs->setVisible(false);
	logs->setSize(tgui::bindWidth(previewPanel), "40%");
	logs->setEnabled(false);
	logs->setPosition(tgui::bindLeft(previewPanel), tgui::bindBottom(previewPanel));
	gui->add(logs);

	previewPanel->onPreview.connect([this](LevelPackObject::LEGAL_STATUS status, std::vector<std::string> messages) {
		// The first string in messages is a description of the attempted action

		if (status == LevelPackObject::LEGAL_STATUS::LEGAL) {
			logs->setText(messages[0]);
		} else if (status == LevelPackObject::LEGAL_STATUS::WARNING) {
			std::string fullString = messages[0] + "\nWarning(s):";
			for (int i = 1; i < messages.size(); i++) {
				fullString += "\n" + messages[i];
			}
			logs->setText(fullString);
		} else {
			std::string fullString = messages[0] + "\nError(s):";
			for (int i = 1; i < messages.size(); i++) {
				fullString += "\n" + messages[i];
			}
			logs->setText(fullString);
		}
	});

	previewNothing();
	updateWidgetsPositionsAndSizes();
}

void LevelPackObjectPreviewWindow::physicsUpdate(float deltaTime) {
	EditorWindow::physicsUpdate(deltaTime);
	previewPanel->physicsUpdate(deltaTime);
}

void LevelPackObjectPreviewWindow::render(float deltaTime) {
	previewPanel->renderUpdate(deltaTime);
	EditorWindow::render(deltaTime);
}