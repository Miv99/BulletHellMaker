#include <Editor/CustomWidgets/EMPAAngleOffsetGroup.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/EditorWindow.h>
#include <Editor/Util/EditorUtils.h>

EMPAAngleOffsetGroup::EMPAAngleOffsetGroup(EditorWindow& parentWindow) 
	: parentWindow(parentWindow) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	changeType = tgui::Button::create();
	changeType->setText("Change type");
	changeType->setToolTip(createToolTip("Changes the type of this angle evaluator."));
	changeType->connect("Pressed", [&]() {
		parentWindow.addPopupWidget(typePopup, parentWindow.getMousePos().x, parentWindow.getMousePos().y, 200, typePopup->getSize().y);
	});
	add(changeType);

	typePopup = createMenuPopup({
		std::make_pair("No evaluation", [&]() {
			this->offset = std::make_shared<EMPAAngleOffsetZero>();
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
			updateWidgets();
		}),
		std::make_pair("Constant value", [&]() {
			this->offset = std::make_shared<EMPAAngleOffsetConstant>();
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
			updateWidgets();
		}),
		std::make_pair("Relative to player", [&]() {
			this->offset = std::make_shared<EMPAAngleOffsetToPlayer>();
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
			updateWidgets();
		}),
		std::make_pair("Absolute position", [&]() {
			this->offset = std::make_shared<EMPAAngleOffsetToGlobalPosition>();
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
			updateWidgets();
		}),
		std::make_pair("Bind to player's direction", [&]() {
			this->offset = std::make_shared<EMPAngleOffsetPlayerSpriteAngle>();
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
			updateWidgets();
		})
		});
	typePopup->setToolTip(createToolTip("No evaluation - This function will return 0.\n\
Constant value - This function will return a constant value.\n\
Relative to player - This function will return the angle from (evaluator.x, evaluator.y) to (player.x + x, player.y + y).\n\
Absolute position - This function will return the angle from (evaluator.x, evalutor.y) to (x, y).\n\
Bind to player's direction - This function will return the rotation angle of the player.\n\
Evaluator refers to the entity that is evaluating this function, and player refers to the player entity."));
	typePopup->setPosition(tgui::bindLeft(changeType), tgui::bindTop(changeType));

	offsetName = tgui::Label::create();
	xLabel = tgui::Label::create();
	yLabel = tgui::Label::create();
	offsetName->setTextSize(TEXT_SIZE);
	xLabel->setTextSize(TEXT_SIZE);
	yLabel->setTextSize(TEXT_SIZE);
	add(offsetName);
	add(xLabel);
	add(yLabel);

	x = EditBox::create();
	y = EditBox::create();
	x->connect("ValueChanged", [&](std::string value) {
		if (ignoreSignals) return;

		if (dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get()) != nullptr) {
			auto ptr = dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get());
			ptr->setXOffset(value);
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
		} else if (dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get()) != nullptr) {
			auto ptr = dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get());
			ptr->setX(value);
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
		} else if (dynamic_cast<EMPAAngleOffsetConstant*>(offset.get()) != nullptr) {
			auto ptr = dynamic_cast<EMPAAngleOffsetConstant*>(offset.get());
			ptr->setValue(value);
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
		}
	});
	y->connect("ValueChanged", [&](std::string value) {
		if (ignoreSignals) return;

		if (dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get()) != nullptr) {
			auto ptr = dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get());
			ptr->setYOffset(value);
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
		} else if (dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get()) != nullptr) {
			auto ptr = dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get());
			ptr->setY(value);
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
		}
	});
	x->setTextSize(TEXT_SIZE);
	y->setTextSize(TEXT_SIZE);
	x->setSize("100%", TEXT_BOX_HEIGHT);
	y->setSize("100%", TEXT_BOX_HEIGHT);
	add(x);
	add(y);

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		if (ignoreResizeSignal) {
			return;
		}

		ignoreResizeSignal = true;

		offsetName->setPosition(0, 0);

		changeType->setSize(100, TEXT_BUTTON_HEIGHT);
		changeType->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(offsetName) + GUI_LABEL_PADDING_Y);

		xLabel->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(changeType) + GUI_PADDING_Y);
		x->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(xLabel) + GUI_LABEL_PADDING_Y);
		yLabel->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(x) + GUI_PADDING_Y);
		y->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(yLabel) + GUI_LABEL_PADDING_Y);

		this->setSize(this->getSizeLayout().x, y->getPosition().y + y->getSize().y);
		ignoreResizeSignal = false;
	});
}

void EMPAAngleOffsetGroup::setEMPAAngleOffset(std::shared_ptr<EMPAAngleOffset> offset) {
	oldOffset = offset;
	this->offset = std::dynamic_pointer_cast<EMPAAngleOffset>(offset->clone());

	updateWidgets();
}

tgui::Signal& EMPAAngleOffsetGroup::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return HideableGroup::getSignal(signalName);
}

void EMPAAngleOffsetGroup::updateWidgets() {
	ignoreSignals = true;

	offsetName->setText(offset->getName());

	if (dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get()) != nullptr) {
		auto ptr = dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get());
		xLabel->setText("X offset");
		yLabel->setText("Y offset");
		x->setText(ptr->getRawXOffset());
		y->setText(ptr->getRawYOffset());
		// Not sure why this is required but it is
		x->setCaretPosition(0);
		y->setCaretPosition(0);

		xLabel->setToolTip(createToolTip("The value of x in this function's evaluation of the angle from (evaluator.x, evaluator.y) to (player.x + x, player.y + y)."));
		yLabel->setToolTip(createToolTip("The value of y in this function's evaluation of the angle from (evaluator.x, evaluator.y) to (player.x + x, player.y + y)."));

		xLabel->setVisible(true);
		yLabel->setVisible(true);
		x->setVisible(true);
		y->setVisible(true);
	} else if (dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get()) != nullptr) {
		auto ptr = dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get());
		xLabel->setText("X");
		yLabel->setText("Y");
		x->setText(ptr->getRawX());
		y->setText(ptr->getRawY());
		// Not sure why this is required but it is
		x->setCaretPosition(0);
		y->setCaretPosition(0);

		xLabel->setToolTip(createToolTip("The value of x in this function's evaluation of the angle from (evaluator.x, evaluator.y) to (x, y)."));
		yLabel->setToolTip(createToolTip("The value of y in this function's evaluation of the angle from (evaluator.x, evaluator.y) to (x, y)."));

		xLabel->setVisible(true);
		yLabel->setVisible(true);
		x->setVisible(true);
		y->setVisible(true);
	} else if (dynamic_cast<EMPAAngleOffsetZero*>(offset.get()) != nullptr) {
		xLabel->setVisible(false);
		yLabel->setVisible(false);
		x->setVisible(false);
		y->setVisible(false);
	} else if (dynamic_cast<EMPAAngleOffsetConstant*>(offset.get()) != nullptr) {
		auto ptr = dynamic_cast<EMPAAngleOffsetConstant*>(offset.get());
		xLabel->setText("Degrees");
		x->setText(ptr->getRawValue());
		// Not sure why this is required but it is
		x->setCaretPosition(0);

		xLabel->setToolTip(createToolTip("The constant value in degrees to be returned by this function."));

		xLabel->setVisible(true);
		yLabel->setVisible(false);
		x->setVisible(true);
		y->setVisible(false);
	} else if (dynamic_cast<EMPAngleOffsetPlayerSpriteAngle*>(offset.get()) != nullptr) {
		xLabel->setVisible(false);
		yLabel->setVisible(false);
		x->setVisible(false);
		y->setVisible(false);
	} else {
		// You forgot a case
		assert(false);
	}

	ignoreSignals = false;
}