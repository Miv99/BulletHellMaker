#include "EditorMovablePointActionPanel.h"

const std::string EditorMovablePointActionPanel::BEZIER_CONTROL_POINT_FORMAT = "%d (%.2f, %.2f)";

EditorMovablePointActionPanel::EditorMovablePointActionPanel(EditorWindow& parentWindow, std::shared_ptr<EMPAction> empa, int undoStackSize) : empa(empa), undoStack(UndoStack(undoStackSize)) {
	setVerticalScrollAmount(SCROLL_AMOUNT);
	setVerticalScrollAmount(SCROLL_AMOUNT);
	
	bezierControlPointsMarkerPlacer = BezierControlPointsPlacer::create(*parentWindow.getWindow());
	bezierControlPointsMarkerPlacer->setPosition(0, 0);
	bezierControlPointsMarkerPlacer->setSize("100%", "100%");
	bezierControlPointsMarkerPlacerFinishEditing = tgui::Button::create();
	bezierControlPointsMarkerPlacerFinishEditing->setSize(100, TEXT_BUTTON_HEIGHT);
	bezierControlPointsMarkerPlacerFinishEditing->setTextSize(TEXT_SIZE);
	bezierControlPointsMarkerPlacerFinishEditing->setText("Finish");
	bezierControlPointsMarkerPlacerFinishEditing->connect("Pressed", [&]() {
		finishEditingBezierControlPoints();
	});

	empaiInfo = tgui::Label::create();
	empaiChangeType = tgui::Button::create();
	empaiDurationLabel = tgui::Label::create();
	empaiDuration = NumericalEditBoxWithLimits::create();
	empaiPolarDistanceLabel = tgui::Label::create();
	empaiPolarDistance = TFVGroup::create(parentWindow);
	empaiPolarAngleLabel = tgui::Label::create();
	empaiPolarAngle = TFVGroup::create(parentWindow);
	empaiBezierControlPointsLabel = tgui::Label::create();
	empaiBezierControlPoints = ListViewScrollablePanel::create();
	empaiAngleOffsetLabel = tgui::Label::create();
	empaiAngleOffset = EMPAAngleOffsetGroup::create(parentWindow);
	empaiHomingStrengthLabel = tgui::Label::create();
	empaiHomingStrength = TFVGroup::create(parentWindow);
	empaiHomingSpeedLabel = tgui::Label::create();
	empaiHomingSpeed = TFVGroup::create(parentWindow);
	empaiEditBezierControlPoints = tgui::Button::create();

	empaiChangeType->setSize(150, TEXT_BUTTON_HEIGHT);

	empaiInfo->setTextSize(TEXT_SIZE);
	empaiChangeType->setTextSize(TEXT_SIZE);
	empaiDurationLabel->setTextSize(TEXT_SIZE);
	empaiPolarDistanceLabel->setTextSize(TEXT_SIZE);
	empaiPolarAngleLabel->setTextSize(TEXT_SIZE);
	empaiBezierControlPointsLabel->setTextSize(TEXT_SIZE);
	empaiAngleOffsetLabel->setTextSize(TEXT_SIZE);
	empaiHomingStrengthLabel->setTextSize(TEXT_SIZE);
	empaiHomingSpeedLabel->setTextSize(TEXT_SIZE);
	empaiEditBezierControlPoints->setTextSize(TEXT_SIZE);

	empaiChangeType->setText("Change action type");
	empaiDurationLabel->setText("Duration");
	empaiPolarDistanceLabel->setText("Distance as function of time");
	empaiPolarAngleLabel->setText("Angle as function of time");
	empaiBezierControlPointsLabel->setText("Bezier control points");
	empaiAngleOffsetLabel->setText("Angle offset");
	empaiHomingStrengthLabel->setText("Homing strength as function of time");
	empaiHomingSpeedLabel->setText("Speed as function of time");
	empaiEditBezierControlPoints->setText("Edit control points");

	changeTypePopup = createMenuPopup({
		std::make_pair("Detach from parent", [&]() {
			std::shared_ptr<EMPAction> oldEMPA = this->empa;
			undoStack.execute(UndoableCommand(
				[this]() {
				this->empa = std::make_shared<DetachFromParentEMPA>();
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			},
				[this, oldEMPA]() {
				this->empa = oldEMPA;
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			}));
		}),
		std::make_pair("Stay still", [&]() {
			std::shared_ptr<EMPAction> oldEMPA = this->empa;
			undoStack.execute(UndoableCommand(
				[this]() {
				this->empa = std::make_shared<StayStillAtLastPositionEMPA>(empaiDuration->getValue());
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			},
				[this, oldEMPA]() {
				this->empa = oldEMPA;
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			}));
		}),
		std::make_pair("Polar-based movement", [&]() {
			std::shared_ptr<EMPAction> oldEMPA = this->empa;
			undoStack.execute(UndoableCommand(
				[this]() {
				this->empa = std::make_shared<MoveCustomPolarEMPA>(std::make_shared<ConstantTFV>(0), std::make_shared<ConstantTFV>(0), empaiDuration->getValue(), std::make_shared<EMPAAngleOffsetZero>());
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			},
				[this, oldEMPA]() {
				this->empa = oldEMPA;
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			}));
		}),
		std::make_pair("Bezier-based movement", [&]() {
			std::shared_ptr<EMPAction> oldEMPA = this->empa;
			undoStack.execute(UndoableCommand(
				[this]() {
				std::vector<sf::Vector2f> points = { sf::Vector2f(0, 0) };
				this->empa = std::make_shared<MoveCustomBezierEMPA>(points, empaiDuration->getValue(), std::make_shared<EMPAAngleOffsetZero>());
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			},
				[this, oldEMPA]() {
				this->empa = oldEMPA;
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			}));
		}),
		std::make_pair("Homing movement", [&]() {
			std::shared_ptr<EMPAction> oldEMPA = this->empa;
			undoStack.execute(UndoableCommand(
				[this]() {
				this->empa = std::make_shared<MovePlayerHomingEMPA>(std::make_shared<ConstantTFV>(0), std::make_shared<ConstantTFV>(0), empaiDuration->getValue());
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			},
				[this, oldEMPA]() {
				this->empa = oldEMPA;
				onEMPATypeChange();
				onEMPAModify.emit(this, this->empa);
			}));
		})
	});
	empaiChangeType->connect("Pressed", [&]() {
		parentWindow.addPopupWidget(changeTypePopup, empaiChangeType->getAbsolutePosition().x, empaiChangeType->getAbsolutePosition().y, 200, changeTypePopup->getSize().y);
	});
	empaiDuration->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) {
			return;
		}

		float oldValue = this->empa->getTime();
		undoStack.execute(UndoableCommand(
			[this, value]() {
			this->empa->setTime(value);

			ignoreSignals = true;
			this->empaiDuration->setValue(value);
			bezierControlPointsMarkerPlacer->setMovementDuration(value);
			ignoreSignals = false;

			if (dynamic_cast<MoveCustomPolarEMPA*>(this->empa.get()) != nullptr) {
				auto ptr = dynamic_cast<MoveCustomPolarEMPA*>(this->empa.get());
				ptr->getDistance()->setMaxTime(value);
				ptr->getAngle()->setMaxTime(value);
			} else if (dynamic_cast<MovePlayerHomingEMPA*>(this->empa.get()) != nullptr) {
				auto ptr = dynamic_cast<MovePlayerHomingEMPA*>(this->empa.get());
				ptr->getHomingStrength()->setMaxTime(value);
				ptr->getSpeed()->setMaxTime(value);
			}
			// Add EMPAs to this if any other EMPAs also use TFVs

			onEMPAModify.emit(this, this->empa);
		},
			[this, oldValue]() {
			this->empa->setTime(oldValue);

			ignoreSignals = true;
			this->empaiDuration->setValue(oldValue);
			bezierControlPointsMarkerPlacer->setMovementDuration(oldValue);
			ignoreSignals = false;

			if (dynamic_cast<MoveCustomPolarEMPA*>(this->empa.get()) != nullptr) {
				auto ptr = dynamic_cast<MoveCustomPolarEMPA*>(this->empa.get());
				ptr->getDistance()->setMaxTime(oldValue);
				ptr->getAngle()->setMaxTime(oldValue);
			} else if (dynamic_cast<MovePlayerHomingEMPA*>(this->empa.get()) != nullptr) {
				auto ptr = dynamic_cast<MovePlayerHomingEMPA*>(this->empa.get());
				ptr->getHomingStrength()->setMaxTime(oldValue);
				ptr->getSpeed()->setMaxTime(oldValue);
			}
			// Add EMPAs to this if any other EMPAs also use TFVs

			onEMPAModify.emit(this, this->empa);
		}));
	});
	empaiAngleOffset->connect("ValueChanged", [&](std::pair<std::shared_ptr<EMPAAngleOffset>, std::shared_ptr<EMPAAngleOffset>> offsets) {
		if (ignoreSignals) {
			return;
		}
		
		std::shared_ptr<EMPAAngleOffset> oldOffset = offsets.first;
		std::shared_ptr<EMPAAngleOffset> updatedOffset = offsets.second;
		std::shared_ptr<EMPAAngleOffset> copyOfOld = oldOffset->clone();

		undoStack.execute(UndoableCommand(
			[this, &oldOffset = oldOffset, updatedOffset]() {
			*oldOffset = *updatedOffset;
			
			ignoreSignals = true;
			this->empaiAngleOffset->setEMPAAngleOffset(updatedOffset);
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		},
			[this, &oldOffset = oldOffset, copyOfOld]() {
			*oldOffset = *copyOfOld;

			ignoreSignals = true;
			this->empaiAngleOffset->setEMPAAngleOffset(copyOfOld);
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		}));
	});
	empaiEditBezierControlPoints->connect("Pressed", [&]() {
		savedWidgets = getWidgets();
		horizontalScrollPos = getHorizontalScrollbarValue();
		verticalScrollPos = getVerticalScrollbarValue();

		removeAllWidgets();
		setHorizontalScrollbarValue(0);
		setVerticalScrollbarValue(0);

		bezierControlPointsMarkerPlacer->clearUndoStack();

		auto ptr = dynamic_cast<MoveCustomBezierEMPA*>(this->empa.get());
		std::vector<sf::Vector2f> points = ptr->getUnrotatedControlPoints();
		std::vector<std::pair<sf::Vector2f, sf::Color>> pointsWithColor;
		for (sf::Vector2f point : points) {
			pointsWithColor.push_back(std::make_pair(point, sf::Color::Red));
		}
		bezierControlPointsMarkerPlacer->setMarkers(pointsWithColor);
		if (points.size() > 0) {
			bezierControlPointsMarkerPlacer->lookAt(points[0]);
		} else {
			bezierControlPointsMarkerPlacer->lookAt(sf::Vector2f(0, 0));
		}

		add(bezierControlPointsMarkerPlacer);
		add(bezierControlPointsMarkerPlacerFinishEditing);
		bezierControlPointsMarkerPlacer->setFocused(true);

		editingBezierControlPoints = true;
	});
	empaiPolarDistance->connect("ValueChanged", [&](std::pair<std::shared_ptr<TFV>, std::shared_ptr<TFV>> tfvs) {
		if (ignoreSignals) {
			return;
		}

		std::shared_ptr<TFV> oldTFV = tfvs.first;
		std::shared_ptr<TFV> updatedTFV = tfvs.second;
		std::shared_ptr<TFV> copyOfOld = oldTFV->clone();

		undoStack.execute(UndoableCommand(
			[this, &oldTFV = oldTFV, updatedTFV]() {
			*oldTFV = *updatedTFV;

			ignoreSignals = true;
			this->empaiPolarDistance->setTFV(updatedTFV, this->empa->getTime());
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		},
			[this, &oldTFV = oldTFV, copyOfOld]() {
			*oldTFV = *copyOfOld;

			ignoreSignals = true;
			this->empaiPolarDistance->setTFV(copyOfOld, this->empa->getTime());
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		}));
	});
	empaiPolarAngle->connect("ValueChanged", [&](std::pair<std::shared_ptr<TFV>, std::shared_ptr<TFV>> tfvs) {
		if (ignoreSignals) {
			return;
		}

		std::shared_ptr<TFV> oldTFV = tfvs.first;
		std::shared_ptr<TFV> updatedTFV = tfvs.second;
		std::shared_ptr<TFV> copyOfOld = oldTFV->clone();

		undoStack.execute(UndoableCommand(
			[this, &oldTFV = oldTFV, updatedTFV]() {
			*oldTFV = *updatedTFV;

			ignoreSignals = true;
			this->empaiPolarAngle->setTFV(updatedTFV, this->empa->getTime());
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		},
			[this, &oldTFV = oldTFV, copyOfOld]() {
			*oldTFV = *copyOfOld;

			ignoreSignals = true;
			this->empaiPolarAngle->setTFV(copyOfOld, this->empa->getTime());
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		}));
	});
	empaiHomingStrength->connect("ValueChanged", [&](std::pair<std::shared_ptr<TFV>, std::shared_ptr<TFV>> tfvs) {
		if (ignoreSignals) {
			return;
		}

		std::shared_ptr<TFV> oldTFV = tfvs.first;
		std::shared_ptr<TFV> updatedTFV = tfvs.second;
		std::shared_ptr<TFV> copyOfOld = oldTFV->clone();

		undoStack.execute(UndoableCommand(
			[this, oldTFV, updatedTFV]() {
			*oldTFV = *updatedTFV;

			ignoreSignals = true;
			this->empaiHomingStrength->setTFV(updatedTFV, this->empa->getTime());
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		},
			[this, oldTFV, copyOfOld]() {
			*oldTFV = *copyOfOld;

			ignoreSignals = true;
			this->empaiHomingStrength->setTFV(copyOfOld, this->empa->getTime());
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		}));
	});
	empaiHomingSpeed->connect("ValueChanged", [&](std::pair<std::shared_ptr<TFV>, std::shared_ptr<TFV>> tfvs) {
		if (ignoreSignals) {
			return;
		}

		std::shared_ptr<TFV> oldTFV = tfvs.first;
		std::shared_ptr<TFV> updatedTFV = tfvs.second;
		std::shared_ptr<TFV> copyOfOld = oldTFV->clone();

		undoStack.execute(UndoableCommand(
			[this, &oldTFV = oldTFV, updatedTFV]() {
			*oldTFV = *updatedTFV;

			ignoreSignals = true;
			this->empaiHomingSpeed->setTFV(updatedTFV, this->empa->getTime());
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		},
			[this, &oldTFV = oldTFV, copyOfOld]() {
			*oldTFV = *copyOfOld;

			ignoreSignals = true;
			this->empaiHomingSpeed->setTFV(copyOfOld, this->empa->getTime());
			ignoreSignals = false;

			onEMPAModify.emit(this, this->empa);
		}));
	});

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		tgui::Layout empaiAreaWidth = newSize.x;
		empaiDuration->setSize(empaiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		empaiPolarDistance->setSize(empaiAreaWidth - GUI_PADDING_X * 2, 0);
		empaiPolarAngle->setSize(empaiAreaWidth - GUI_PADDING_X * 2, 0);
		empaiBezierControlPoints->setSize(empaiAreaWidth - GUI_PADDING_X * 2, 200);
		empaiAngleOffset->setSize(empaiAreaWidth - GUI_PADDING_X * 2, 0);
		empaiHomingStrength->setSize(empaiAreaWidth - GUI_PADDING_X * 2, 0);
		empaiHomingSpeed->setSize(empaiAreaWidth - GUI_PADDING_X * 2, 0);
		empaiEditBezierControlPoints->setSize(empaiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);

		empaiInfo->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
		empaiChangeType->setPosition(tgui::bindLeft(empaiInfo), tgui::bindBottom(empaiInfo) + GUI_PADDING_Y);
		empaiDurationLabel->setPosition(tgui::bindLeft(empaiChangeType), tgui::bindBottom(empaiChangeType) + GUI_PADDING_Y);
		empaiDuration->setPosition(GUI_PADDING_X, empaiDurationLabel->getPosition().y + empaiDurationLabel->getSize().y + GUI_LABEL_PADDING_Y);
		empaiAngleOffsetLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiDuration) + GUI_PADDING_Y);
		empaiAngleOffset->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiAngleOffsetLabel) + GUI_LABEL_PADDING_Y);
		empaiPolarDistanceLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiAngleOffset) + GUI_PADDING_Y);
		empaiPolarDistance->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiPolarDistanceLabel) + GUI_LABEL_PADDING_Y);
		empaiPolarAngleLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiPolarDistance) + GUI_PADDING_Y);
		empaiPolarAngle->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiPolarAngleLabel) + GUI_LABEL_PADDING_Y);
		empaiBezierControlPointsLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiAngleOffset) + GUI_PADDING_Y);
		empaiBezierControlPoints->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiBezierControlPointsLabel) + GUI_LABEL_PADDING_Y);
		empaiEditBezierControlPoints->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiBezierControlPoints) + GUI_PADDING_Y);
		empaiHomingStrengthLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiDuration) + GUI_PADDING_Y);
		empaiHomingStrength->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiHomingStrengthLabel) + GUI_LABEL_PADDING_Y);
		empaiHomingSpeedLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiHomingStrength) + GUI_PADDING_Y);
		empaiHomingSpeed->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiHomingSpeedLabel) + GUI_LABEL_PADDING_Y);
	
		bezierControlPointsMarkerPlacerFinishEditing->setPosition(newSize.x - bezierControlPointsMarkerPlacerFinishEditing->getSize().x, newSize.y - bezierControlPointsMarkerPlacerFinishEditing->getSize().y);
	});

	// Init values
	empaiDuration->setValue(empa->getTime());
	bezierControlPointsMarkerPlacer->setMovementDuration(empa->getTime());

	add(empaiInfo);
	add(empaiChangeType);
	add(empaiDurationLabel);
	add(empaiDuration);
	add(empaiPolarDistanceLabel);
	add(empaiPolarDistance);
	add(empaiPolarAngleLabel);
	add(empaiPolarAngle);
	add(empaiBezierControlPointsLabel);
	add(empaiBezierControlPoints);
	add(empaiAngleOffsetLabel);
	add(empaiAngleOffset);
	add(empaiHomingStrengthLabel);
	add(empaiHomingStrength);
	add(empaiHomingSpeedLabel);
	add(empaiHomingSpeed);
	add(empaiEditBezierControlPoints);

	onEMPATypeChange();
}

bool EditorMovablePointActionPanel::handleEvent(sf::Event event) {
	if (editingBezierControlPoints) {
		if (bezierControlPointsMarkerPlacer->handleEvent(event)) {
			return true;
		}

		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			finishEditingBezierControlPoints();
		}
	} else if (event.type == sf::Event::KeyPressed) {
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

tgui::Signal & EditorMovablePointActionPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onEMPAModify.getName())) {
		return onEMPAModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void EditorMovablePointActionPanel::onEMPATypeChange() {
	if (dynamic_cast<MoveCustomPolarEMPA*>(empa.get())) {
		MoveCustomPolarEMPA* concreteEMPA = dynamic_cast<MoveCustomPolarEMPA*>(empa.get());
		empaiDuration->setVisible(true);
		empaiDuration->setValue(empa->getTime());
		empaiPolarDistance->setVisible(true);
		empaiPolarDistance->setTFV(concreteEMPA->getDistance(), empa->getTime());
		empaiPolarAngle->setVisible(true);
		empaiPolarAngle->setTFV(concreteEMPA->getAngle(), empa->getTime());
		empaiBezierControlPoints->setVisible(false);
		empaiEditBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(true);
		empaiAngleOffset->setEMPAAngleOffset(concreteEMPA->getAngleOffset());
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Polar coordinates movement action");

		empaiAngleOffsetLabel->setVisible(true);
		empaiAngleOffsetLabel->setText("Angle offset");
		empaiAngleOffsetLabel->setToolTip(createToolTip("The angle offset that will be added to the angle component of this polar movement action. \n\
The angle offset is evaluated only once, at the start of this movement action, and the evaluated value will be added to \
all evaluations of the angle component of this polar movement action."));
	} else if (dynamic_cast<MoveCustomBezierEMPA*>(empa.get())) {
		MoveCustomBezierEMPA* concreteEMPA = dynamic_cast<MoveCustomBezierEMPA*>(empa.get());
		empaiDuration->setVisible(true);
		empaiDuration->setValue(empa->getTime());
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(true);
		empaiEditBezierControlPoints->setVisible(true);
		empaiAngleOffset->setVisible(true);
		empaiAngleOffset->setEMPAAngleOffset(concreteEMPA->getRotationAngle());
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Bezier movement action");
		updateEmpaiBezierControlPoints();

		bezierControlPointsMarkerPlacer->setMovementDuration(concreteEMPA->getTime());

		empaiAngleOffsetLabel->setVisible(true);
		empaiAngleOffsetLabel->setText("Rotation angle");
		empaiAngleOffsetLabel->setToolTip(createToolTip("The angle of rotation of the bezier control points.\n\
The angle offset is evaluated only once, at the start of this movement action, and all control points in this bezier movement action \
will then be rotated about the first control point (0, 0) when this movement action is executed."));
	} else if (dynamic_cast<MovePlayerHomingEMPA*>(empa.get())) {
		MovePlayerHomingEMPA* concreteEMPA = dynamic_cast<MovePlayerHomingEMPA*>(empa.get());
		empaiDuration->setVisible(true);
		empaiDuration->setValue(empa->getTime());
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(false);
		empaiEditBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(false);
		empaiHomingStrength->setVisible(true);
		empaiHomingStrength->setTFV(concreteEMPA->getHomingStrength(), empa->getTime());
		empaiHomingSpeed->setVisible(true);
		empaiHomingSpeed->setTFV(concreteEMPA->getSpeed(), empa->getTime());
		empaiInfo->setText("Homing movement action");

		empaiAngleOffsetLabel->setVisible(false);
	} else if (dynamic_cast<StayStillAtLastPositionEMPA*>(empa.get())) {
		empaiDuration->setVisible(true);
		empaiDuration->setValue(empa->getTime());
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(false);
		empaiEditBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(false);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Stay still action");

		empaiAngleOffsetLabel->setVisible(false);
	} else if (dynamic_cast<DetachFromParentEMPA*>(empa.get())) {
		empaiDuration->setVisible(false);
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(false);
		empaiEditBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(false);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Detach action");

		empaiAngleOffsetLabel->setVisible(false);
	} else {
		// This means you forgot to add a case
		assert(false);
	}
	// Add EMPAs to this if any other EMPAs also use TFVs
	empaiDurationLabel->setVisible(empaiDuration->isVisible());
	empaiPolarDistanceLabel->setVisible(empaiPolarDistance->isVisible());
	empaiPolarAngleLabel->setVisible(empaiPolarAngle->isVisible());
	empaiBezierControlPointsLabel->setVisible(empaiBezierControlPoints->isVisible());
	empaiAngleOffsetLabel->setVisible(empaiAngleOffset->isVisible());
	empaiHomingStrengthLabel->setVisible(empaiHomingStrength->isVisible());
	empaiHomingSpeedLabel->setVisible(empaiHomingSpeed->isVisible());
}

void EditorMovablePointActionPanel::updateEmpaiBezierControlPoints() {
	MoveCustomBezierEMPA* ptr = dynamic_cast<MoveCustomBezierEMPA*>(empa.get());
	empaiBezierControlPoints->getListView()->removeAllItems();
	auto cps = ptr->getUnrotatedControlPoints();
	for (int i = 0; i < cps.size(); i++) {
		empaiBezierControlPoints->getListView()->addItem(format(BEZIER_CONTROL_POINT_FORMAT, i + 1, cps[i].x, cps[i].y));
	}
}

void EditorMovablePointActionPanel::finishEditingBezierControlPoints() {
	removeAllWidgets();
	for (auto widget : savedWidgets) {
		add(widget);
	}
	savedWidgets.clear();
	setHorizontalScrollbarValue(horizontalScrollPos);
	setVerticalScrollbarValue(verticalScrollPos);

	auto oldControlPoints = dynamic_cast<MoveCustomBezierEMPA*>(empa.get())->getUnrotatedControlPoints();
	auto newControlPoints = bezierControlPointsMarkerPlacer->getMarkerPositions();
	undoStack.execute(UndoableCommand(
		[this, newControlPoints]() {
		MoveCustomBezierEMPA* ptr = dynamic_cast<MoveCustomBezierEMPA*>(empa.get());
		ptr->setUnrotatedControlPoints(newControlPoints);
		updateEmpaiBezierControlPoints();
	},
		[this, oldControlPoints]() {
		MoveCustomBezierEMPA* ptr = dynamic_cast<MoveCustomBezierEMPA*>(empa.get());
		ptr->setUnrotatedControlPoints(oldControlPoints);
		updateEmpaiBezierControlPoints();
	}));

	editingBezierControlPoints = false;
}