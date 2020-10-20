#include <Editor/CustomWidgets/EMPAListVisualizer.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>
#include <Editor/Util/TguiUtils.h>

const float EMPAListVisualizer::EVALUATOR_CIRCLE_RADIUS = 3.0f;

EMPAListVisualizer::EMPAListVisualizer(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution, int undoStackSize)
	: MarkerPlacer(parentWindow, clipboard, resolution, undoStackSize) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	leftPanel->setVisible(false);
	leftPanel->setSize(0, 0);

	evaluatorCircle = sf::CircleShape(EVALUATOR_CIRCLE_RADIUS);
	evaluatorCircle.setFillColor(sf::Color::Transparent);
	evaluatorCircle.setOutlineColor(sf::Color::Magenta);
	evaluatorCircle.setOrigin(sf::Vector2f(EVALUATOR_CIRCLE_RADIUS, EVALUATOR_CIRCLE_RADIUS));
	evaluatorCircle.setOutlineThickness(1);

	evaluator = SliderWithEditBox::create(false);
	evaluatorResult = tgui::Label::create();
	std::shared_ptr<tgui::Button> cycleMovementPathPrimitiveTypeButton = tgui::Button::create();

	evaluator->setToolTip(createToolTip("Evaluates a position given some time in seconds. Note that homing actions cannot be evaluated and will be treated as not moving at all when \
calculating the movement path and when evaluating positions."));
	evaluatorResult->setToolTip(createToolTip("The result of the evaluation. Note that homing actions cannot be evaluated and will be treated as not moving at all when \
calculating the movement path and when evaluating positions."));
	cycleMovementPathPrimitiveTypeButton->setToolTip(createToolTip("Cycles between points and lines for movement path"));

	evaluator->setTextSize(TEXT_SIZE);
	evaluatorResult->setTextSize(TEXT_SIZE);
	cycleMovementPathPrimitiveTypeButton->setTextSize(TEXT_SIZE);

	evaluatorResult->setText("Result:                   ");
	cycleMovementPathPrimitiveTypeButton->setText("Cycle path type");

	evaluator->setIntegerMode(false);
	evaluator->setMin(0);
	evaluator->setMax(0);
	evaluator->setStep(MAX_PHYSICS_DELTA_TIME);

	evaluator->onValueChange.connect([this](float value) {
		updateEvaluatorResult();
	});
	cycleMovementPathPrimitiveTypeButton->onPress.connect([this]() {
		cycleMovementPathPrimitiveType();
	});

	onSizeChange.connect([this]() {
		updateEvaluatorResult();
	});

	evaluator->setSize("50%", TEXT_BOX_HEIGHT);
	cycleMovementPathPrimitiveTypeButton->setSize("50%", TEXT_BOX_HEIGHT);

	extraWidgetsPanel->addExtraRowWidget(evaluator, GUI_PADDING_Y);
	extraWidgetsPanel->addExtraColumnWidget(evaluatorResult, GUI_PADDING_X);
	extraWidgetsPanel->addExtraRowWidget(cycleMovementPathPrimitiveTypeButton, GUI_LABEL_PADDING_Y);
}

void EMPAListVisualizer::draw(tgui::BackendRenderTargetBase& target, tgui::RenderStates states) const {
	MarkerPlacer::draw(target, states);

	sf::RenderStates sfmlStates = toSFMLRenderStates(states);

	// Draw movement path
	sf::View originalView = parentWindow.getView();
	// Adjust to account for the window's view
	sf::View offsetView = viewFromViewController;
	offsetView.setCenter(offsetView.getCenter() + getAbsolutePosition());
	
	parentWindow.setView(offsetView);
	parentWindow.draw(movementPath, sfmlStates);
	if (evaluatorCircle.getRadius() > 0) {
		parentWindow.draw(evaluatorCircle, sfmlStates);
	}
	parentWindow.setView(originalView);
}

void EMPAListVisualizer::updatePath(std::vector<std::shared_ptr<EMPAction>> empas) {
	this->empas = empas;
	movementPath = generateVertexArray(empas, MAX_PHYSICS_DELTA_TIME, startX, startY, true, sf::Color::Red, sf::Color::Blue);
	movementPath.setPrimitiveType(movementPathPrimitiveType);
	movementPathWithoutSpecialColor = sf::VertexArray(movementPath);

	empaEndingPos.clear();
	empaActiveTime.clear();
	float curTime = 0;
	sf::Vector2f lastPos(startX, startY);
	for (std::shared_ptr<EMPAction> empa : empas) {
		empaActiveTime.push_back(curTime);
		curTime += empa->getTime();

		// Cannot be calculated without a registry
		if (std::dynamic_pointer_cast<MovePlayerHomingEMPA>(empa) || std::dynamic_pointer_cast<MoveGlobalHomingEMPA>(empa)) {
			empaEndingPos.push_back(lastPos);
			continue;
		}

		sf::Vector2f endingPos = empa->generateStandaloneMP(lastPos.x, lastPos.y, 0, 0)->compute(sf::Vector2f(0, 0), empa->getTime());
		empaEndingPos.push_back(endingPos + lastPos);
		lastPos = endingPos + lastPos;
	}
	evaluator->setMax(curTime);

	updateEvaluatorResult();
	setEMPAPathColor(specialColorIndex, specialColor);
}

void EMPAListVisualizer::cycleMovementPathPrimitiveType() {
	if (movementPathPrimitiveType == sf::PrimitiveType::Points) {
		movementPathPrimitiveType = sf::PrimitiveType::LineStrip;
	} else {
		movementPathPrimitiveType = sf::PrimitiveType::Points;
	}
	updatePath(this->empas);
}

void EMPAListVisualizer::setEMPAPathColor(int empaIndex, sf::Color color) {
	specialColorIndex = empaIndex;
	specialColor = color;

	if (empaIndex == -1 || empaIndex >= empas.size()) {
		movementPath = sf::VertexArray(movementPathWithoutSpecialColor);
		return;
	}
	movementPath = sf::VertexArray(movementPathWithoutSpecialColor);
	if (skippedByGenerateVertexArray(empas[empaIndex])) {
		return;
	}
	// O(n) where n is empaIndex
	float empaTimeStart = 0;
	for (int i = 0; i < empaIndex; i++) {
		// If the EMPA is skipped by generateVertexArray(), its points won't be in movementPath
		if (!skippedByGenerateVertexArray(empas[i])) {
			empaTimeStart += empas[i]->getTime();
		}
	}
	int start = empaTimeStart / MAX_PHYSICS_DELTA_TIME;
	for (int i = start; i <= std::min((int)(start + (empas[empaIndex]->getTime() / MAX_PHYSICS_DELTA_TIME)), (int)movementPath.getVertexCount() - 1); i++) {
		movementPath[i].color = color;
	}
}

void EMPAListVisualizer::setStartPosX(float x) {
	startX = x;
	updatePath(empas);
}

void EMPAListVisualizer::setStartPosY(float y) {
	// MarkerPlacer uses an inverted y coordinate system
	startY = -y;
	updatePath(empas);
}

void EMPAListVisualizer::updateEvaluatorResult() {
	float time = evaluator->getValue();
	auto it = std::lower_bound(empaActiveTime.begin(), empaActiveTime.end(), time);
	if (it == empaActiveTime.begin()) {
		evaluatorResult->setText(format("Result: (%.3f, %.3f)", startX, startY));
		evaluatorCircle.setPosition(sf::Vector2f(startX, -startY));
		evaluatorCircle.setRadius(EVALUATOR_CIRCLE_RADIUS);
	} else {
		it--;
		int index = it - empaActiveTime.begin();
		sf::Vector2f start(startX, startY);
		if (index != 0) {
			start = empaEndingPos[index - 1];
		}
		// Cannot be calculated without a registry
		if (skippedByGenerateVertexArray(empas[index])) {
			evaluatorResult->setText("Result: Unknown (homing action)");

			evaluatorCircle.setRadius(-1);
		} else {
			sf::Vector2f res = empas[index]->generateStandaloneMP(start.x, start.y, 0, 0)->compute(sf::Vector2f(0, 0), time - empaActiveTime[index]) + start;
			evaluatorResult->setText(format("Result: (%.3f, %.3f)", res.x, res.y));

			evaluatorCircle.setPosition(sf::Vector2f(res.x, -res.y));
			evaluatorCircle.setRadius(EVALUATOR_CIRCLE_RADIUS);
		}
	}
	evaluator->setSize(std::min((getSize().x - leftPanel->getSize().x) / 2.0f, (getSize().x - leftPanel->getSize().x) - evaluatorResult->getSize().x - GUI_PADDING_X), TEXT_BOX_HEIGHT);
}