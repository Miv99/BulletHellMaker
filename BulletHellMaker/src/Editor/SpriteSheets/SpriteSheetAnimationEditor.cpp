#include <Editor/SpriteSheets/SpriteSheetAnimationEditor.h>

#include <GuiConfig.h>
#include <Editor/Windows/MainEditorWindow.h>

const float SpriteSheetAnimationEditor::MAX_ANIMATION_PREVIEW_HEIGHT = 300;
const int SpriteSheetAnimationEditor::MAX_SPRITE_ROWS_PER_PAGE = 20;

SpriteSheetAnimationEditor::SpriteSheetAnimationEditor(MainEditorWindow& mainEditorWindow, std::shared_ptr<SpriteLoader> spriteLoader,
	std::shared_ptr<AnimationData> animationData, int undoStackSize)
	: mainEditorWindow(mainEditorWindow), spriteLoader(spriteLoader), animationData(animationData), undoStack(UndoStack(undoStackSize)) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	animationPreviewPicture = AnimatablePicture::create();
	animationPreviewPicture->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	add(animationPreviewPicture);

	previousPageButton = tgui::Button::create();
	previousPageButton->setTextSize(TEXT_SIZE);
	previousPageButton->setText("<");
	previousPageButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	previousPageButton->onPress.connect([this]() {
		this->setCurrentPage(this->currentPage - 1);
	});
	add(previousPageButton);

	currentPageLabel = tgui::Label::create();
	currentPageLabel->setTextSize(TEXT_SIZE);
	currentPageLabel->setPosition(tgui::bindRight(previousPageButton) + GUI_PADDING_X, tgui::bindTop(previousPageButton));
	add(currentPageLabel);

	nextPageButton = tgui::Button::create();
	nextPageButton->setTextSize(TEXT_SIZE);
	nextPageButton->setText(">");
	nextPageButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	nextPageButton->setPosition(tgui::bindRight(currentPageLabel) + GUI_PADDING_X, tgui::bindTop(previousPageButton));
	nextPageButton->onPress.connect([this]() {
		this->setCurrentPage(this->currentPage + 1);
	});
	add(nextPageButton);

	gotoPageEditBox = NumericalEditBoxWithLimits::create();
	gotoPageEditBox->setTextSize(TEXT_SIZE);
	gotoPageEditBox->setSize(100, TEXT_BUTTON_HEIGHT);
	gotoPageEditBox->onValueChange.connect([this](float page) {
		int rounded = std::lround(page);
		setCurrentPage(rounded - 1);
	});
	add(gotoPageEditBox);

	gotoPageLabel = tgui::Label::create();
	gotoPageLabel->setTextSize(TEXT_SIZE);
	gotoPageLabel->setText("Go to page:");
	gotoPageLabel->setPosition(tgui::bindLeft(gotoPageEditBox) - tgui::bindWidth(gotoPageLabel) - GUI_PADDING_X, tgui::bindTop(nextPageButton));
	add(gotoPageLabel);

	insertSpriteInfoButton = tgui::Button::create();
	insertSpriteInfoButton->setTextSize(TEXT_SIZE);
	insertSpriteInfoButton->setText("+");
	insertSpriteInfoButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	insertSpriteInfoButton->onPress([this]() {
		undoStack.execute(UndoableCommand([this]() {
			int row = this->animationData->getNumSprites();
			this->insertSpriteInfo(row, std::make_pair(0, ""));

			viewRow(row);
		}, [this]() {
			this->animationData->removeSpriteEntry(this->animationData->getNumSprites() - 1);
			onAnimationModified();

			viewRow(this->animationData->getNumSprites() - 1);
		}));
	});
	add(insertSpriteInfoButton);

	for (int i = 0; i < MAX_SPRITE_ROWS_PER_PAGE; i++) {
		SpriteRow row;

		row.widgetGroup = tgui::Group::create();
		if (i == 0) {
			row.widgetGroup->setPosition(tgui::bindLeft(animationPreviewPicture), tgui::bindBottom(animationPreviewPicture) + GUI_PADDING_Y * 2);
		} else if (i != 0) {
			row.widgetGroup->setPosition(tgui::bindLeft(spriteRows[i - 1].widgetGroup), tgui::bindBottom(spriteRows[i - 1].widgetGroup) + GUI_PADDING_Y);
		}
		row.widgetGroup->setSize("100%", TEXT_BOX_HEIGHT);

		row.rowNumberLabel = tgui::Label::create();
		row.rowNumberLabel->setTextSize(TEXT_SIZE);
		row.widgetGroup->add(row.rowNumberLabel);

		row.spriteNameEditBox = tgui::EditBox::create();
		row.spriteNameEditBox->setTextSize(TEXT_SIZE);
		row.spriteNameEditBox->setSize("40%", TEXT_BOX_HEIGHT);
		row.spriteNameEditBox->setPosition(tgui::bindRight(row.rowNumberLabel) + GUI_PADDING_X, 0);
		row.spriteNameEditBox->onReturnOrUnfocus([this, i](tgui::String spriteName) {
			if (ignoreSignals) {
				return;
			}

			std::string oldSpriteName = this->animationData->getSpriteInfo(this->spriteRows[i].row).second;
			if (static_cast<std::string>(spriteName) == oldSpriteName) {
				return;
			}

			int row = this->spriteRows[i].row;
			undoStack.execute(UndoableCommand([this, row, spriteName]() {
				this->animationData->setSpriteName(row, static_cast<std::string>(spriteName));
				onAnimationModified();

				viewRow(row);
			}, [this, row, oldSpriteName]() {
				this->animationData->setSpriteName(row, oldSpriteName);
				onAnimationModified();

				viewRow(row);
			}));
		});
		row.widgetGroup->add(row.spriteNameEditBox);

		row.durationEditBox = NumericalEditBoxWithLimits::create();
		row.durationEditBox->setMin(0);
		row.durationEditBox->setTextSize(TEXT_SIZE);
		row.durationEditBox->setSize("25%", TEXT_BOX_HEIGHT);
		row.durationEditBox->setPosition(tgui::bindRight(row.spriteNameEditBox) + GUI_PADDING_X, 0);
		row.durationEditBox->onValueChange([this, i](float duration) {
			if (ignoreSignals) {
				return;
			}

			float oldDuration = this->animationData->getSpriteInfo(this->spriteRows[i].row).first;
			int row = this->spriteRows[i].row;
			undoStack.execute(UndoableCommand([this, row, duration]() {
				this->animationData->setSpriteDuration(row, duration);
				onAnimationModified();

				viewRow(row);
			}, [this, row, oldDuration]() {
				this->animationData->setSpriteDuration(row, oldDuration);
				onAnimationModified();

				viewRow(row);
			}));
		});
		row.widgetGroup->add(row.durationEditBox);

		row.deleteRowButton = tgui::Button::create();
		row.deleteRowButton->setTextSize(TEXT_SIZE);
		row.deleteRowButton->setText("X");
		row.deleteRowButton->setSize(TEXT_BOX_HEIGHT, TEXT_BOX_HEIGHT);
		row.deleteRowButton->setPosition(tgui::bindRight(row.durationEditBox) + GUI_PADDING_X, 0);
		row.deleteRowButton->onPress([this, i]() {
			std::pair<float, std::string> oldInfo = this->animationData->getSpriteInfo(this->spriteRows[i].row);
			int row = this->spriteRows[i].row;
			undoStack.execute(UndoableCommand([this, row]() {
				this->animationData->removeSpriteEntry(row);
				onAnimationModified();

				viewRow(row);
			}, [this, row, oldInfo]() {
				this->insertSpriteInfo(row, oldInfo);

				viewRow(row);
			}));
		});
		row.widgetGroup->add(row.deleteRowButton);

		row.addRowButton = tgui::Button::create();
		row.addRowButton->setTextSize(TEXT_SIZE);
		row.addRowButton->setText("+");
		row.addRowButton->setSize(TEXT_BOX_HEIGHT, TEXT_BOX_HEIGHT);
		row.addRowButton->setPosition(tgui::bindRight(row.deleteRowButton) + GUI_PADDING_X, 0);
		row.addRowButton->onPress([this, i]() {
			int row = this->spriteRows[i].row;
			undoStack.execute(UndoableCommand([this, row]() {
				this->insertSpriteInfo(row, std::make_pair(0, ""));

				viewRow(row);
			}, [this, row]() {
				this->animationData->removeSpriteEntry(row);
				onAnimationModified();

				viewRow(row);
			}));
		});
		row.widgetGroup->add(row.addRowButton);

		add(row.widgetGroup);
		spriteRows.push_back(row);
	}

	onSizeChange.connect([this](sf::Vector2f newSize) {
		int currentRow = currentPage * spriteRowsPerPage;

		animationPreviewPicture->setSize(newSize.x - GUI_PADDING_X * 2, tgui::bindMin(MAX_ANIMATION_PREVIEW_HEIGHT, newSize.y * 0.33f));

		previousPageButton->setPosition(GUI_PADDING_X, newSize.y - GUI_PADDING_Y - previousPageButton->getSize().y);
		gotoPageEditBox->setPosition(newSize.x - GUI_PADDING_X - tgui::bindWidth(gotoPageEditBox), tgui::bindTop(previousPageButton));

		spriteRowsPerPage = std::max(std::min((int)((newSize.y - (animationPreviewPicture->getPosition().y + animationPreviewPicture->getSize().y) - GUI_PADDING_Y * 4) / 
			(spriteRows[0].widgetGroup->getSize().y + GUI_PADDING_Y)), MAX_SPRITE_ROWS_PER_PAGE), 1);

		viewRow(currentRow);
	});
	
	// Init spriteRowsPerPage to 1 for now; once this widget is loaded, spriteRowsPerPage will be set properly in onSizeChange
	spriteRowsPerPage = 1;
	setCurrentPage(0);
}

bool SpriteSheetAnimationEditor::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				undoStack.undo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				undoStack.redo();
				return true;
			}
		}
	}
	return false;
}

void SpriteSheetAnimationEditor::setSpriteLoader(std::shared_ptr<SpriteLoader> spriteLoader, const std::string& spriteSheetName) {
	this->spriteLoader = spriteLoader;
	this->spriteSheetName = spriteSheetName;
	animationPreviewPicture->setAnimation(*spriteLoader, animationData->getAnimationName(), spriteSheetName);
}

void SpriteSheetAnimationEditor::setCurrentPage(int currentPage) {
	int maxPage = std::ceill((float)animationData->getNumSprites() / spriteRowsPerPage);
	if (currentPage < 0) {
		currentPage = 0;
	}
	// Allow going to maxPage only if all rows are filled up in the last page
	if (currentPage >= maxPage) {
		if (animationData->getNumSprites() % spriteRowsPerPage == 0) {
			currentPage = maxPage;
		} else {
			currentPage = std::max(0, maxPage - 1);
		}
	}

	this->currentPage = currentPage;
	if (animationData->getNumSprites() % spriteRowsPerPage == 0) {
		currentPageLabel->setText(format("%d/%d", currentPage + 1, maxPage + 1));
	} else {
		currentPageLabel->setText(format("%d/%d", currentPage + 1, maxPage));
	}

	ignoreSignals = true;

	int maxRowDigits = std::log10((currentPage + 1) * spriteRowsPerPage + 1) + 1;
	int numRowsVisible = 0;
	for (int i = 0; i < spriteRowsPerPage; i++) {
		spriteRows[i].row = currentPage * spriteRowsPerPage + i;

		if (currentPage * spriteRowsPerPage + i < animationData->getNumSprites()) {
			spriteRows[i].widgetGroup->setVisible(true);
			numRowsVisible++;

			std::string rowString = std::to_string(currentPage * spriteRowsPerPage + i + 1);
			if (rowString.length() < maxRowDigits) {
				rowString += " ";
			}
			spriteRows[i].rowNumberLabel->setText(rowString);

			std::pair<float, std::string> spriteInfo = animationData->getSpriteInfo(currentPage * spriteRowsPerPage + i);
			spriteRows[i].durationEditBox->setValue(spriteInfo.first);
			spriteRows[i].spriteNameEditBox->setText(spriteInfo.second);
		} else {
			spriteRows[i].widgetGroup->setVisible(false);
		}
	}
	for (int i = spriteRowsPerPage; i < MAX_SPRITE_ROWS_PER_PAGE; i++) {
		spriteRows[i].widgetGroup->setVisible(false);
	}

	if (numRowsVisible == 0) {
		insertSpriteInfoButton->setPosition(tgui::bindLeft(animationPreviewPicture), tgui::bindBottom(animationPreviewPicture) + GUI_PADDING_Y * 2);
		insertSpriteInfoButton->setVisible(true);
	} else if (numRowsVisible != spriteRowsPerPage) {
		insertSpriteInfoButton->setPosition(tgui::bindLeft(spriteRows[numRowsVisible - 1].widgetGroup), 
			tgui::bindBottom(spriteRows[numRowsVisible - 1].widgetGroup) + GUI_PADDING_Y);
		insertSpriteInfoButton->setVisible(true);
	} else {
		insertSpriteInfoButton->setVisible(false);
	}
	
	ignoreSignals = false;
}

void SpriteSheetAnimationEditor::viewRow(int row) {
	setCurrentPage((int)(row / spriteRowsPerPage));
}

void SpriteSheetAnimationEditor::insertSpriteInfo(int index, std::pair<float, std::string> newInfo) {
	this->animationData->insertSpriteInfo(index, newInfo);
	onAnimationModified();

	viewRow(index);
}

void SpriteSheetAnimationEditor::onAnimationModified() {
	if (spriteLoader) {
		spriteLoader->unloadAnimation(spriteSheetName, animationData->getAnimationName());
		animationPreviewPicture->setAnimation(*spriteLoader, animationData->getAnimationName(), spriteSheetName);
	}

	onAnimationModify.emit(this);
}
