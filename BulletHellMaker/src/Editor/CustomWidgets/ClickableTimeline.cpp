#include <Editor/CustomWidgets/ClickableTimeline.h>

#include <Mutex.h>
#include <GuiConfig.h>

ClickableTimeline::ClickableTimeline() {
	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	cameraController = tgui::RangeSlider::create();
	cameraController->setPosition(6, 6);
	cameraController->connect("RangeChanged", [&](float lower, float upper) {
		if ((upper - lower) / maxTimeValue < 0.01) {
			cameraController->setSelectionEnd(lower + maxTimeValue * 0.01);
			cameraController->setSelectionStart(upper - maxTimeValue * 0.01);
			return;
		}

		buttonScalarScalar = 1 / ((upper - lower) / maxTimeValue);
		panel->setHorizontalScrollbarValue(std::lrint(lower * buttonScalar * buttonScalarScalar));

		updateButtonsPositionsAndSizes();
	});
	add(cameraController);

	panel = tgui::ScrollablePanel::create();
	panel->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	panel->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	add(panel);

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		cameraController->setSize(newSize.x - 12, SLIDER_HEIGHT);
		panel->setSize("100%", newSize.y - SLIDER_HEIGHT - 5);
		panel->setPosition(0, tgui::bindBottom(cameraController) + 5);
		buttonScalar = newSize.x / maxTimeValue;

		updateButtonsPositionsAndSizes();
	});
}

void ClickableTimeline::setElements(std::vector<std::pair<float, std::string>> elementStartTimesAndDuration, float maxTime) {
	if (elementStartTimesAndDuration.size() == 0) {
		elements.clear();
		panel->removeAllWidgets();
		return;
	}

	std::vector<std::tuple<float, float, std::string>> converted;
	for (int i = 1; i < elementStartTimesAndDuration.size(); i++) {
		converted.push_back(std::make_tuple(elementStartTimesAndDuration[i - 1].first, elementStartTimesAndDuration[i].first - elementStartTimesAndDuration[i - 1].first, elementStartTimesAndDuration[i - 1].second));
	}
	converted.push_back(std::make_tuple(elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1].first, maxTime - elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1].first, elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1].second));
	setElements(converted, maxTime);
}

void ClickableTimeline::setElements(std::vector<std::tuple<float, float, std::string>> elementStartTimesAndDuration) {
	elements.clear();
	panel->removeAllWidgets();

	if (elementStartTimesAndDuration.size() == 0) return;

	maxTimeValue = std::get<0>(elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1]) + std::get<1>(elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1]);
	buttonScalar = panel->getSize().x / maxTimeValue;

	cameraController->setMaximum(maxTimeValue);
	cameraController->setStep(maxTimeValue / 200.0f);
	cameraController->setSelectionStart(0);
	cameraController->setSelectionEnd(maxTimeValue);

	int i = 0;
	for (std::tuple<float, float, std::string> tuple : elementStartTimesAndDuration) {
		std::shared_ptr<tgui::Button> button = tgui::Button::create();
		button->setText(std::get<2>(tuple));
		button->connect("Pressed", [&, i]() {
			onElementPressed.emit(this, i);
		});
		panel->add(button);
		elements.push_back(std::make_tuple(std::get<0>(tuple), std::get<1>(tuple), button));
		i++;
	}

	updateButtonsPositionsAndSizes();
}

void ClickableTimeline::setElements(std::vector<std::tuple<float, float, std::string>> elementStartTimesAndDuration, float maxTime) {
	elements.clear();
	panel->removeAllWidgets();

	if (elementStartTimesAndDuration.size() == 0) return;

	maxTimeValue = maxTime;
	buttonScalar = panel->getSize().x / maxTimeValue;

	cameraController->setMaximum(maxTimeValue);
	cameraController->setStep(maxTimeValue / 200.0f);
	cameraController->setSelectionStart(0);
	cameraController->setSelectionEnd(maxTimeValue);

	int i = 0;
	for (std::tuple<float, float, std::string> tuple : elementStartTimesAndDuration) {
		std::shared_ptr<tgui::Button> button = tgui::Button::create();
		button->setText(std::get<2>(tuple));
		button->connect("Pressed", [&, i]() {
			onElementPressed.emit(this, i);
		});
		panel->add(button);
		elements.push_back(std::make_tuple(std::get<0>(tuple), std::get<1>(tuple), button));
		i++;
	}

	updateButtonsPositionsAndSizes();
}

tgui::Signal& ClickableTimeline::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onElementPressed.getName())) {
		return onElementPressed;
	}
	return tgui::Group::getSignal(signalName);
}

void ClickableTimeline::updateButtonsPositionsAndSizes() {
	for (auto tuple : elements) {
		std::shared_ptr<tgui::Button> button = std::get<2>(tuple);
		button->setPosition(std::get<0>(tuple) * buttonScalar * buttonScalarScalar, 0);
		button->setSize(std::get<1>(tuple) * buttonScalar * buttonScalarScalar, panel->getSize().y - 2);
	}
}