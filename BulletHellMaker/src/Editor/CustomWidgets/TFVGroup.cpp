#include <Editor/CustomWidgets/TFVGroup.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <matplotlibcpp.h>
#include <Util/StringUtils.h>
#include <Editor/EditorWindow.h>
#include <Editor/Util/EditorUtils.h>

const float TFVGroup::TFV_TIME_RESOLUTION = MAX_PHYSICS_DELTA_TIME;

TFVGroup::TFVGroup(EditorWindow& parentWindow, Clipboard& clipboard) 
	: CopyPasteable("PiecewiseTFVSegment"), parentWindow(parentWindow), clipboard(clipboard) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	showGraph = tgui::Button::create();
	showGraph->setText("Show graph");
	showGraph->setToolTip(createToolTip("Plots the graph of this value with the x-axis representing time in seconds and the y-axis representing the result of this value's evaluation. \
This might take a while to load the first time."));
	showGraph->connect("Pressed", [&]() {
		auto points = generateMPPoints(tfv, tfvLifespan, TFV_TIME_RESOLUTION);
		matplotlibcpp::clf();
		matplotlibcpp::close();
		for (int i = 0; i < points.size(); i++) {
			if (i % 2 == 0) {
				matplotlibcpp::plot(points[i].first, points[i].second, "r");
			} else {
				matplotlibcpp::plot(points[i].first, points[i].second, "b");
			}
		}
		matplotlibcpp::show();
	});
	add(showGraph);

	addSegment = tgui::Button::create();
	addSegment->setText("Add");
	addSegment->setToolTip(createToolTip("Adds a new segment to this value."));
	addSegment->connect("Pressed", [&]() {
		if (ignoreSignals) return;

		std::lock_guard<std::recursive_mutex> lock(tfvMutex);

		float time = 0;
		if (selectedSegment) {
			time = tfv->getSegment(selectedSegmentIndex).first;
		}
		tfv->insertSegment(std::make_pair(time, std::make_shared<ConstantTFV>(0)), tfvLifespan);
		onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		if (selectedSegment) {
			selectSegment(selectedSegmentIndex + 1);
		}
		populateSegmentList();
	});
	add(addSegment);

	deleteSegment = tgui::Button::create();
	deleteSegment->setText("Delete");
	deleteSegment->setToolTip(createToolTip("Deletes the selected segment."));
	deleteSegment->connect("Pressed", [&]() {
		std::lock_guard<std::recursive_mutex> lock(tfvMutex);

		tfv->removeSegment(selectedSegmentIndex, tfvLifespan);
		populateSegmentList();
		if (tfv->getSegmentsCount() == 1) {
			selectSegment(0);
		} else if (selectedSegmentIndex < tfv->getSegmentsCount()) {
			selectSegment(selectedSegmentIndex);
		} else {
			selectSegment(selectedSegmentIndex - 1);
		}
	});
	add(deleteSegment);

	changeSegmentType = tgui::Button::create();
	changeSegmentType->setText("Change type");
	changeSegmentType->setToolTip(createToolTip("Changes the selected segment's type."));
	changeSegmentType->connect("Pressed", [&]() {
		parentWindow.addPopupWidget(segmentTypePopup, parentWindow.getMousePos().x, parentWindow.getMousePos().y, 200, segmentTypePopup->getSize().y);
	});
	add(changeSegmentType);

	segmentTypePopup = createMenuPopup({
		std::make_pair("Linear", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<LinearTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			// Reselect to reload widgets
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Constant", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<ConstantTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Sine wave", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<SineWaveTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Distance from acceleration", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<ConstantAccelerationDistanceTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Dampened start", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<DampenedStartTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Dampened end", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<DampenedEndTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Double dampened", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<DoubleDampenedTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		})
		});
	segmentTypePopup->setToolTip(createToolTip("Linear - Straight line, of form y = A + (B - A)(x / T).\n\
Constant - Straight line, of form y = C.\n\
Sine wave - Sine function, of form y = A*sin(x*2*pi/T + P) + C.\n\
Distance from acceleration - Standard distance formula, of form y = C + V*x + 0.5*A*(T^2).\n\
Dampened start - A curve whose value changes slowly at the beginning, of form y = (B - A) / T^(0.08*D + 1) * x^(0.08f*D + 1) + A.\n\
Dampened end - A curve whose value changes slowly at the end, of form y = (B - A) / T^(0.08*D + 1) * (T - x)^(0.08f*D + 1) + B.\n\
Double dampened - An S-shaped curve, of form y = DampenedStart(x, A, B, D, T/2) if x <= T/2; y = DampenedEnd(x, A, B, D, T/2) if x > T/2.\
The evaluation of this value will return the currently active segment's evaluation at time t-T0, where T0 is the start time of the segment. In every equation above, T is the lifespan in seconds of the segment."));
	segmentTypePopup->setPosition(tgui::bindLeft(changeSegmentType), tgui::bindTop(changeSegmentType));

	segmentList = std::make_shared<ListBoxScrollablePanel>();
	segmentList->setTextSize(TEXT_SIZE);
	segmentList->setToolTip(createToolTip("The list of segments in this value. \"(t=A to t=B)\" denotes the time range in seconds in which the segment is active. \
The evaluation of this value will return the currently active segment's evaluation at time t-A, where A is the start time of the segment. The first segment must have \
a start time of t=0."));
	segmentList->getListBox()->connect("ItemSelected", [&](std::string item, std::string id) {
		if (ignoreSignals) return;
		if (id != "") {
			selectSegment(std::stoi(id));
		} else {
			deselectSegment();
		}
	});
	add(segmentList);

	tfvFloat1Label = tgui::Label::create();
	tfvFloat2Label = tgui::Label::create();
	tfvFloat3Label = tgui::Label::create();
	tfvFloat4Label = tgui::Label::create();
	tfvInt1Label = tgui::Label::create();
	startTimeLabel = tgui::Label::create();
	startTimeLabel->setText("Start time");
	startTimeLabel->setTextSize(TEXT_SIZE);
	tfvFloat1Label->setTextSize(TEXT_SIZE);
	tfvFloat2Label->setTextSize(TEXT_SIZE);
	tfvFloat3Label->setTextSize(TEXT_SIZE);
	tfvFloat4Label->setTextSize(TEXT_SIZE);
	tfvInt1Label->setTextSize(TEXT_SIZE);
	add(tfvFloat1Label);
	add(tfvFloat2Label);
	add(tfvFloat3Label);
	add(tfvFloat4Label);
	add(tfvInt1Label);
	add(startTimeLabel);

	tfvFloat1EditBox = NumericalEditBoxWithLimits::create();
	tfvFloat1EditBox->setIntegerMode(false);
	tfvFloat2EditBox = NumericalEditBoxWithLimits::create();
	tfvFloat2EditBox->setIntegerMode(false);
	tfvFloat3EditBox = NumericalEditBoxWithLimits::create();
	tfvFloat3EditBox->setIntegerMode(false);
	tfvFloat4EditBox = NumericalEditBoxWithLimits::create();
	tfvFloat4EditBox->setIntegerMode(false);
	tfvInt1EditBox = NumericalEditBoxWithLimits::create();
	tfvInt1EditBox->setIntegerMode(true);
	startTime = SliderWithEditBox::create();
	startTime->setIntegerMode(false);
	startTime->setStep(MAX_PHYSICS_DELTA_TIME);
	tfvFloat1EditBox->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<LinearTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<LinearTFV*>(selectedSegment.get());
			ptr->setStartValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<ConstantTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<ConstantTFV*>(selectedSegment.get());
			ptr->setValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
			ptr->setPeriod(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get());
			ptr->setInitialDistance(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedStartTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedStartTFV*>(selectedSegment.get());
			ptr->setStartValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedEndTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedEndTFV*>(selectedSegment.get());
			ptr->setStartValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get());
			ptr->setStartValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	tfvFloat2EditBox->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<LinearTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<LinearTFV*>(selectedSegment.get());
			ptr->setEndValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
			ptr->setAmplitude(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get());
			ptr->setInitialVelocity(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedStartTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedStartTFV*>(selectedSegment.get());
			ptr->setEndValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedEndTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedEndTFV*>(selectedSegment.get());
			ptr->setEndValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get());
			ptr->setEndValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	tfvFloat3EditBox->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
			ptr->setValueShift(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get());
			ptr->setAcceleration(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	tfvFloat4EditBox->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
			ptr->setPhaseShift(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	tfvInt1EditBox->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<DampenedStartTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedStartTFV*>(selectedSegment.get());
			ptr->setDampeningFactor(std::round(value));
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedEndTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedEndTFV*>(selectedSegment.get());
			ptr->setDampeningFactor(std::round(value));
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get());
			ptr->setDampeningFactor(std::round(value));
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	startTime->connect("ValueChanged", [&, &selectedSegmentIndex = this->selectedSegmentIndex](float value) {
		if (ignoreSignals) return;

		std::lock_guard<std::recursive_mutex> lock(tfvMutex);
		if (selectedSegmentIndex != -1) {
			selectSegment(tfv->changeSegmentStartTime(selectedSegmentIndex, value, tfvLifespan));
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
		populateSegmentList();
	});
	startTime->setToolTip(createToolTip("The time in seconds at which this segment becomes active."));
	tfvFloat1EditBox->setTextSize(TEXT_SIZE);
	tfvFloat2EditBox->setTextSize(TEXT_SIZE);
	tfvFloat3EditBox->setTextSize(TEXT_SIZE);
	tfvFloat4EditBox->setTextSize(TEXT_SIZE);
	tfvInt1EditBox->setTextSize(TEXT_SIZE);
	startTime->setTextSize(TEXT_SIZE);
	add(tfvFloat1EditBox);
	add(tfvFloat2EditBox);
	add(tfvFloat3EditBox);
	add(tfvFloat4EditBox);
	add(tfvInt1EditBox);
	add(startTime);

	showGraph->setSize(100, TEXT_BUTTON_HEIGHT);
	showGraph->setPosition(0, 0);

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		if (ignoreResizeSignal) {
			return;
		}

		ignoreResizeSignal = true;

		segmentList->setSize("40%", 300);
		segmentList->setPosition(tgui::bindLeft(showGraph), tgui::bindBottom(showGraph) + GUI_PADDING_Y);

		int segmentListRightBoundary = segmentList->getPosition().x + segmentList->getSize().x + GUI_PADDING_X;

		startTime->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvFloat1EditBox->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvFloat2EditBox->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvFloat3EditBox->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvFloat4EditBox->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvInt1EditBox->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);

		startTimeLabel->setPosition(segmentListRightBoundary, tgui::bindTop(segmentList));
		startTime->setPosition(segmentListRightBoundary, startTimeLabel->getPosition().y + startTimeLabel->getSize().y + GUI_LABEL_PADDING_Y);

		tfvFloat1Label->setPosition(segmentListRightBoundary, tgui::bindBottom(startTime) + GUI_PADDING_Y);
		tfvFloat1EditBox->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat1Label) + GUI_LABEL_PADDING_Y);

		tfvFloat2Label->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat1EditBox) + GUI_PADDING_Y);
		tfvFloat2EditBox->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat2Label) + GUI_LABEL_PADDING_Y);

		tfvFloat3Label->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat2EditBox) + GUI_PADDING_Y);
		tfvFloat3EditBox->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat3Label) + GUI_LABEL_PADDING_Y);

		tfvFloat4Label->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat3EditBox) + GUI_PADDING_Y);
		tfvFloat4EditBox->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat4Label) + GUI_LABEL_PADDING_Y);

		tfvInt1Label->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat2EditBox) + GUI_PADDING_Y);
		tfvInt1EditBox->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvInt1Label) + GUI_LABEL_PADDING_Y);

		segmentList->setSize(segmentList->getSize().x, tgui::bindBottom(this->tfvInt1EditBox));

		float buttonWidth = (segmentList->getSize().x - GUI_PADDING_X * 2) / 3.0f;
		addSegment->setSize(buttonWidth, TEXT_BUTTON_HEIGHT);
		addSegment->setPosition(tgui::bindLeft(segmentList), tgui::bindBottom(segmentList) + GUI_PADDING_Y);
		deleteSegment->setSize(buttonWidth, TEXT_BUTTON_HEIGHT);
		deleteSegment->setPosition(tgui::bindRight(addSegment) + GUI_PADDING_X, tgui::bindTop(addSegment));
		changeSegmentType->setSize(buttonWidth, TEXT_BUTTON_HEIGHT);
		changeSegmentType->setPosition(tgui::bindRight(deleteSegment) + GUI_PADDING_X, tgui::bindTop(deleteSegment));

		this->setSize(this->getSizeLayout().x, changeSegmentType->getPosition().y + changeSegmentType->getSize().y);
		ignoreResizeSignal = false;
	});

	deselectSegment();
}

std::pair<std::shared_ptr<CopiedObject>, std::string> TFVGroup::copyFrom() {
	if (selectedSegment) {
		float startTime = tfv->getSegment(selectedSegmentIndex).first;
		return std::make_pair(std::make_shared<CopiedPiecewiseTFVSegment>(getID(), std::make_pair(startTime, selectedSegment)), "Copied 1 time-function variable segment");
	}
	return std::make_pair(nullptr, "");
}

std::string TFVGroup::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	auto derived = std::static_pointer_cast<CopiedPiecewiseTFVSegment>(pastedObject);
	if (derived) {
		selectSegment(tfv->insertSegment(derived->getSegment(), tfvLifespan));
		onValueChange.emit(this, std::make_pair(oldTFV, tfv));

		return "Pasted 1 time-function variable segment";
	}
	return "";
}

std::string TFVGroup::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	auto derived = std::static_pointer_cast<CopiedPiecewiseTFVSegment>(pastedObject);
	if (derived && selectedSegment) {
		auto pasted = derived->getSegment();
		tfv->changeSegment(selectedSegmentIndex, pasted.second, tfvLifespan);
		// Change segment start time second because it may change the segment's index
		selectSegment(tfv->changeSegmentStartTime(selectedSegmentIndex, pasted.first, tfvLifespan));
		onValueChange.emit(this, std::make_pair(oldTFV, tfv));

		return "Replaced 1 time-function variable segment";
	}
	return "";
}

bool TFVGroup::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed && segmentList->mouseOnWidget(parentWindow.getLastMousePressPos() - getAbsolutePosition())) {
		if (event.key.code == sf::Keyboard::Delete) {
			// Can't delete the last segment
			if (selectedSegment && tfv->getSegmentsCount() > 1) {
				tfv->removeSegment(selectedSegmentIndex, tfvLifespan);
				onValueChange.emit(this, std::make_pair(oldTFV, tfv));
				return true;
			}
		} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::C) {
				clipboard.copy(this);
				return true;
			} else if (event.key.code == sf::Keyboard::V) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
					clipboard.paste2(this);
				} else {
					clipboard.paste(this);
				}
				return true;
			}
		}
	}
	return false;
}

void TFVGroup::setTFV(std::shared_ptr<TFV> tfv, float tfvLifespan) {
	oldTFV = tfv;
	this->tfvLifespan = tfvLifespan;
	if (dynamic_cast<PiecewiseTFV*>(tfv.get()) != nullptr) {
		this->tfv = std::dynamic_pointer_cast<PiecewiseTFV>(tfv->clone());
	} else {
		this->tfv = std::make_shared<PiecewiseTFV>();
		this->tfv->insertSegment(0, std::make_pair(0, tfv->clone()), tfvLifespan);
	}

	populateSegmentList();
	// Reselect segment to set widget values
	selectSegment(selectedSegmentIndex);
}

tgui::Signal& TFVGroup::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return tgui::Group::getSignal(signalName);
}

void TFVGroup::deselectSegment() {
	selectedSegmentIndex = -1;
	selectedSegment = nullptr;
	deleteSegment->setEnabled(false);
	changeSegmentType->setVisible(false);
	segmentList->getListBox()->deselectItem();
	bool f1 = false, f2 = false, f3 = false, f4 = false, i1 = false;
	tfvFloat1Label->setVisible(f1);
	tfvFloat1EditBox->setVisible(f1);
	tfvFloat2Label->setVisible(f2);
	tfvFloat2EditBox->setVisible(f2);
	tfvFloat3Label->setVisible(f3);
	tfvFloat3EditBox->setVisible(f3);
	tfvFloat4Label->setVisible(f4);
	tfvFloat4EditBox->setVisible(f4);
	tfvInt1Label->setVisible(i1);
	tfvInt1EditBox->setVisible(i1);
	startTimeLabel->setVisible(false);
	startTime->setVisible(false);
}

void TFVGroup::selectSegment(int index) {
	if (index == -1 || index >= tfv->getSegmentsCount()) {
		deselectSegment();
		return;
	}

	std::lock_guard<std::recursive_mutex> lock(tfvMutex);

	selectedSegmentIndex = index;
	selectedSegment = tfv->getSegment(index).second;
	// Cannot delete the last segment
	deleteSegment->setEnabled(tfv->getSegmentsCount() != 1);
	changeSegmentType->setVisible(true);

	ignoreSignals = true;
	segmentList->getListBox()->setSelectedItemById(std::to_string(index));

	// whether to use tfvFloat1EditBox, tfvFloat2EditBox, ...
	bool f1 = false, f2 = false, f3 = false, f4 = false, i1 = false;

	if (dynamic_cast<LinearTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = true;

		tfvFloat1Label->setText("Start value");
		tfvFloat2Label->setText("End value");

		tfvFloat1Label->setToolTip(createToolTip("The value of A in y = A + (B - A)(x / T)."));
		tfvFloat2Label->setToolTip(createToolTip("The value of B in y = A + (B - A)(x / T)."));

		auto ptr = dynamic_cast<LinearTFV*>(selectedSegment.get());
		tfvFloat1EditBox->setValue(ptr->getStartValue());
		tfvFloat2EditBox->setValue(ptr->getEndValue());
	} else if (dynamic_cast<ConstantTFV*>(selectedSegment.get()) != nullptr) {
		f1 = true;

		tfvFloat1Label->setText("Value");
		tfvFloat1Label->setToolTip(createToolTip("The value of C in y = C."));

		auto ptr = dynamic_cast<ConstantTFV*>(selectedSegment.get());
		tfvFloat1EditBox->setValue(ptr->getValue());
	} else if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = f3 = f4 = true;

		tfvFloat1Label->setText("Period");
		tfvFloat2Label->setText("Amplitude");
		tfvFloat3Label->setText("Value shift");
		tfvFloat4Label->setText("Phase shift");

		tfvFloat1Label->setToolTip(createToolTip("The value of in seconds T in y = A*sin(x*2*pi/T + P) + C."));
		tfvFloat2Label->setToolTip(createToolTip("The value of A in y = A*sin(x*2*pi/T + P) + C."));
		tfvFloat3Label->setToolTip(createToolTip("The value of C in y = A*sin(x*2*pi/T + P) + C."));
		tfvFloat4Label->setToolTip(createToolTip("The value in seconds of P in y = A*sin(x*2*pi/T + P) + C."));

		auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
		tfvFloat1EditBox->setValue(ptr->getPeriod());
		tfvFloat2EditBox->setValue(ptr->getAmplitude());
		tfvFloat3EditBox->setValue(ptr->getValueShift());
		tfvFloat4EditBox->setValue(ptr->getPhaseShift());
	} else if (dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = f3 = true;

		tfvFloat1Label->setText("Initial distance");
		tfvFloat2Label->setText("Initial velocity");
		tfvFloat3Label->setText("Acceleration");

		tfvFloat1Label->setToolTip(createToolTip("The value of C in y = C + V*x + 0.5*A*(T^2), where T is the lifespan in seconds of this segment."));
		tfvFloat2Label->setToolTip(createToolTip("The value of V in y = C + V*x + 0.5*A*(T^2), where T is the lifespan in seconds of this segment."));
		tfvFloat3Label->setToolTip(createToolTip("The value of A in y = C + V*x + 0.5*A*(T^2), where T is the lifespan in seconds of this segment."));

		auto ptr = dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get());
		tfvFloat1EditBox->setValue(ptr->getInitialDistance());
		tfvFloat2EditBox->setValue(ptr->getInitialVelocity());
		tfvFloat3EditBox->setValue(ptr->getAcceleration());
	} else if (dynamic_cast<DampenedStartTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = i1 = true;

		tfvFloat1Label->setText("Start value");
		tfvFloat2Label->setText("End value");
		tfvInt1Label->setText("Dampening factor");

		tfvFloat1Label->setToolTip(createToolTip("The value of A in y = (B - A) / T^(0.08*D + 1) * x^(0.08f*D + 1) + A, where T is the lifespan in seconds of this segment."));
		tfvFloat2Label->setToolTip(createToolTip("The value of B in y = (B - A) / T^(0.08*D + 1) * x^(0.08f*D + 1) + A, where T is the lifespan in seconds of this segment."));
		tfvInt1Label->setToolTip(createToolTip("The integer value of D in y = (B - A) / T^(0.08*D + 1) * x^(0.08f*D + 1) + A, where T is the lifespan in seconds of this segment."));

		auto ptr = dynamic_cast<DampenedStartTFV*>(selectedSegment.get());
		tfvFloat1EditBox->setValue(ptr->getStartValue());
		tfvFloat2EditBox->setValue(ptr->getEndValue());
		tfvInt1EditBox->setValue(ptr->getDampeningFactor());
	} else if (dynamic_cast<DampenedEndTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = i1 = true;

		tfvFloat1Label->setText("Start value");
		tfvFloat2Label->setText("End value");
		tfvInt1Label->setText("Dampening factor");

		tfvFloat1Label->setToolTip(createToolTip("The value of A in y = (B - A) / T^(0.08*D + 1) * (T - x)^(0.08f*D + 1) + B, where T is the lifespan in seconds of this segment."));
		tfvFloat2Label->setToolTip(createToolTip("The value of B in y = (B - A) / T^(0.08*D + 1) * (T - x)^(0.08f*D + 1) + B, where T is the lifespan in seconds of this segment."));
		tfvInt1Label->setToolTip(createToolTip("The integer value of D in y = (B - A) / T^(0.08*D + 1) * (T - x)^(0.08f*D + 1) + B, where T is the lifespan in seconds of this segment."));

		auto ptr = dynamic_cast<DampenedEndTFV*>(selectedSegment.get());
		tfvFloat1EditBox->setValue(ptr->getStartValue());
		tfvFloat2EditBox->setValue(ptr->getEndValue());
		tfvInt1EditBox->setValue(ptr->getDampeningFactor());
	} else if (dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = i1 = true;

		tfvFloat1Label->setText("Start value");
		tfvFloat2Label->setText("End value");
		tfvInt1Label->setText("Dampening factor");

		tfvFloat1Label->setToolTip(createToolTip("The value of A in y = DampenedStart(x, A, B, D, T/2) if x <= T/2; y = DampenedEnd(x, A, B, D, T/2) if x > T/2, where T is the lifespan in seconds of this segment."));
		tfvFloat2Label->setToolTip(createToolTip("The value of B in y = DampenedStart(x, A, B, D, T/2) if x <= T/2; y = DampenedEnd(x, A, B, D, T/2) if x > T/2, where T is the lifespan in seconds of this segment."));
		tfvInt1Label->setToolTip(createToolTip("The integer value of D in y = DampenedStart(x, A, B, D, T/2) if x <= T/2; y = DampenedEnd(x, A, B, D, T/2) if x > T/2, where T is the lifespan in seconds of this segment."));

		auto ptr = dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get());
		tfvFloat1EditBox->setValue(ptr->getStartValue());
		tfvFloat2EditBox->setValue(ptr->getEndValue());
		tfvInt1EditBox->setValue(ptr->getDampeningFactor());
	} else {
		// You missed a case
		assert(false);
	}
	startTime->setValue(tfv->getSegment(index).first);
	startTime->setMin(0);
	startTime->setMax(tfvLifespan);

	tfvFloat1Label->setVisible(f1);
	tfvFloat1EditBox->setVisible(f1);
	tfvFloat2Label->setVisible(f2);
	tfvFloat2EditBox->setVisible(f2);
	tfvFloat3Label->setVisible(f3);
	tfvFloat3EditBox->setVisible(f3);
	tfvFloat4Label->setVisible(f4);
	tfvFloat4EditBox->setVisible(f4);
	tfvInt1Label->setVisible(i1);
	tfvInt1EditBox->setVisible(i1);
	startTimeLabel->setVisible(true);
	startTime->setVisible(true);

	// For some reason selecting segments will mess up stuff so this is to fix that
	ignoreResizeSignal = true;
	this->setSize(this->getSizeLayout().x, changeSegmentType->getPosition().y + changeSegmentType->getSize().y);
	ignoreResizeSignal = false;
	startTime->setCaretPosition(0);
	tfvFloat1EditBox->setCaretPosition(0);
	tfvFloat2EditBox->setCaretPosition(0);
	tfvFloat3EditBox->setCaretPosition(0);
	tfvFloat4EditBox->setCaretPosition(0);
	tfvInt1EditBox->setCaretPosition(0);

	ignoreSignals = false;
}

void TFVGroup::populateSegmentList() {
	ignoreSignals = true;
	std::lock_guard<std::recursive_mutex> lock(tfvMutex);
	std::string selectedIndexString = segmentList->getListBox()->getSelectedItemId();
	segmentList->getListBox()->removeAllItems();
	int index = 0;
	for (int i = 0; i < tfv->getSegmentsCount(); i++) {
		auto segment = tfv->getSegment(i);

		float nextTime = tfvLifespan;
		if (i + 1 < tfv->getSegmentsCount()) {
			nextTime = tfv->getSegment(i + 1).first;
		}

		segmentList->getListBox()->addItem("[" + std::to_string(index + 1) + "] " + segment.second->getName() + " (t=" + formatNum(segment.first) + " to t=" + formatNum(nextTime) + ")", std::to_string(index));
		index++;
	}
	// Reselect old segment if any
	if (selectedIndexString != "") {
		segmentList->getListBox()->setSelectedItemById(selectedIndexString);
	}
	ignoreSignals = false;
}