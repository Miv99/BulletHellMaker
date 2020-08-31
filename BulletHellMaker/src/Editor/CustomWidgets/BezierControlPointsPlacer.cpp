#include <Editor/CustomWidgets/BezierControlPointsPlacer.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>

const float BezierControlPointsPlacer::EVALUATOR_CIRCLE_RADIUS = 3.0f;

BezierControlPointsPlacer::BezierControlPointsPlacer(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution, int undoStackSize) 
	: MarkerPlacer(parentWindow, clipboard, resolution, undoStackSize) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	evaluatorCircle = sf::CircleShape(EVALUATOR_CIRCLE_RADIUS);
	evaluatorCircle.setFillColor(sf::Color::Transparent);
	evaluatorCircle.setOutlineColor(sf::Color::Magenta);
	evaluatorCircle.setOrigin(sf::Vector2f(EVALUATOR_CIRCLE_RADIUS, EVALUATOR_CIRCLE_RADIUS));
	evaluatorCircle.setOutlineThickness(1);

	timeResolution = SliderWithEditBox::create();
	evaluator = SliderWithEditBox::create(false);
	evaluatorResult = tgui::Label::create();
	std::shared_ptr<tgui::Button> cycleMovementPathPrimitiveTypeButton = tgui::Button::create();

	timeResolution->setToolTip(createToolTip("Amount of time in seconds between each movement dot"));
	evaluator->setToolTip(createToolTip("Evaluates a position given some time in seconds"));
	evaluatorResult->setToolTip(createToolTip("The result of the evaluation relative to the first control point"));
	cycleMovementPathPrimitiveTypeButton->setToolTip(createToolTip("Cycles between points and lines for movement path"));

	timeResolution->setTextSize(TEXT_SIZE);
	evaluator->setTextSize(TEXT_SIZE);
	evaluatorResult->setTextSize(TEXT_SIZE);
	cycleMovementPathPrimitiveTypeButton->setTextSize(TEXT_SIZE);

	evaluatorResult->setText("Result:                   ");
	cycleMovementPathPrimitiveTypeButton->setText("Cycle path type");

	timeResolution->setIntegerMode(false);
	timeResolution->setMin(MAX_PHYSICS_DELTA_TIME);
	timeResolution->setMax(1.0f);
	timeResolution->setStep(MAX_PHYSICS_DELTA_TIME);

	evaluator->setIntegerMode(false);
	evaluator->setMin(0);
	evaluator->setMax(0);
	evaluator->setStep(MAX_PHYSICS_DELTA_TIME);

	timeResolution->connect("ValueChanged", [this]() {
		updatePath();
	});
	evaluator->connect("ValueChanged", [this](float value) {
		updateEvaluatorResult();
	});
	cycleMovementPathPrimitiveTypeButton->connect("Pressed", [this]() {
		cycleMovementPathPrimitiveType();
	});

	timeResolution->setSize("50%", SLIDER_HEIGHT);
	evaluator->setSize("50%", TEXT_BOX_HEIGHT);
	cycleMovementPathPrimitiveTypeButton->setSize("50%", TEXT_BOX_HEIGHT);

	addExtraRowWidget(timeResolution, GUI_PADDING_Y);
	addExtraRowWidget(evaluator, GUI_LABEL_PADDING_Y);
	addExtraColumnWidget(evaluatorResult, GUI_PADDING_X);
	addExtraRowWidget(cycleMovementPathPrimitiveTypeButton, GUI_LABEL_PADDING_Y);
}

std::string BezierControlPointsPlacer::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	std::string result = MarkerPlacer::pasteInto(pastedObject);
	updatePath();
	return result;
}

std::string BezierControlPointsPlacer::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	std::string result = MarkerPlacer::paste2Into(pastedObject);
	updatePath();
	return result;
}

void BezierControlPointsPlacer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	MarkerPlacer::draw(target, states);

	// Draw movement path
	sf::View originalView = parentWindow.getView();
	sf::View offsetView = viewFromViewController;
	// Not sure why this is necessary
	offsetView.setCenter(offsetView.getCenter() + getAbsolutePosition());
	parentWindow.setView(offsetView);
	parentWindow.draw(movementPath, states);
	if (evaluatorCircle.getRadius() > 0) {
		parentWindow.draw(evaluatorCircle, states);
	}
	parentWindow.setView(originalView);
}

void BezierControlPointsPlacer::cycleMovementPathPrimitiveType() {
	if (movementPathPrimitiveType == sf::PrimitiveType::Points) {
		movementPathPrimitiveType = sf::PrimitiveType::LineStrip;
	} else {
		movementPathPrimitiveType = sf::PrimitiveType::Points;
	}
	updatePath();
}

void BezierControlPointsPlacer::setMovementDuration(float time) {
	movementPathTime = time;
	evaluator->setMin(0);
	evaluator->setMax(time);
	updatePath();
}

std::vector<sf::Vector2f> BezierControlPointsPlacer::getMarkerPositions() {
	auto res = std::vector<sf::Vector2f>();
	for (auto p : markers) {
		res.push_back(sf::Vector2f(p.getPosition().x - markers[0].getPosition().x, -(p.getPosition().y - markers[0].getPosition().y)));
	}
	return res;
}

void BezierControlPointsPlacer::setSelectedMarkerXWidgetValue(float value) {
	selectedMarkerX->setValue(value - markers[0].getPosition().x);
}

void BezierControlPointsPlacer::setSelectedMarkerYWidgetValue(float value) {
	selectedMarkerY->setValue(-value - markers[0].getPosition().y);
}

void BezierControlPointsPlacer::updateMarkersListView() {
	ignoreSignals = true;

	markersListView->getListView()->removeAllItems();
	for (int i = 0; i < markers.size(); i++) {
		markersListView->getListView()->addItem(format(MARKERS_LIST_VIEW_ITEM_FORMAT, i + 1, markers[i].getPosition().x - markers[0].getPosition().x, -(markers[i].getPosition().y - markers[0].getPosition().y)));
	}
	if (selectedMarkerIndex >= 0) {
		markersListView->getListView()->setSelectedItem(selectedMarkerIndex);
	}
	updatePath();

	ignoreSignals = false;
}

void BezierControlPointsPlacer::updateMarkersListViewItem(int index) {
	updateMarkersListView();
}

void BezierControlPointsPlacer::updatePath() {
	if (markers.size() == 0) {
		movementPath = sf::VertexArray();
		return;
	}

	auto markerPositions = std::vector<sf::Vector2f>();
	for (auto p : markers) {
		markerPositions.push_back(p.getPosition() - markers[0].getPosition());
	}

	std::shared_ptr<EMPAction> empa = std::make_shared<MoveCustomBezierEMPA>(markerPositions, movementPathTime);
	movementPath = generateVertexArray(empa, timeResolution->getValue(), markers[0].getPosition().x, markers[0].getPosition().y, sf::Color::Red, sf::Color::Blue);
	movementPath.setPrimitiveType(movementPathPrimitiveType);

	updateEvaluatorResult();
}

void BezierControlPointsPlacer::updateEvaluatorResult() {
	std::shared_ptr<BezierMP> mp = std::make_shared<BezierMP>(movementPathTime, getMarkerPositions());
	sf::Vector2f res = mp->compute(sf::Vector2f(0, 0), evaluator->getValue());
	evaluatorResult->setText(format("Result: (%.3f, %.3f)", res.x, res.y));
	evaluator->setSize(std::min((getSize().x - leftPanel->getSize().x) / 2.0f, (getSize().x - leftPanel->getSize().x) - evaluatorResult->getSize().x - GUI_PADDING_X), TEXT_BOX_HEIGHT);

	evaluatorCircle.setPosition(sf::Vector2f(res.x, -res.y));
}