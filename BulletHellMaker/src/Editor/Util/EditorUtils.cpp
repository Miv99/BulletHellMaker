#include <Editor/Util/EditorUtils.h>

#include <Mutex.h>
#include <GuiConfig.h>

void sendToForeground(sf::RenderWindow& window) {
	// Windows only
#ifdef _WIN32
	SetForegroundWindow(window.getSystemHandle());
#endif
	//TODO other OS's if necessary
}

std::shared_ptr<tgui::Label> createToolTip(std::string text) {
	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	auto tooltip = tgui::Label::create();
	tooltip->setMaximumTextWidth(300);
	tooltip->setText(text);
	tooltip->setTextSize(TEXT_SIZE);
	tooltip->getRenderer()->setBackgroundColor(tgui::Color::White);

	return tooltip;
}

std::shared_ptr<tgui::ListBox> createMenuPopup(std::vector<std::pair<std::string, std::function<void()>>> elements) {
	auto popup = tgui::ListBox::create();
	int i = 0;
	int longestStringLength = 0;
	for (std::pair<std::string, std::function<void()>> element : elements) {
		longestStringLength = std::max(longestStringLength, (int)element.first.length());
		popup->addItem(element.first, std::to_string(i));
		i++;
	}
	popup->setItemHeight(20);
	popup->setSize(std::max(150, (int)(longestStringLength * popup->getTextSize())), (popup->getItemHeight() + 1) * popup->getItemCount());
	popup->onMousePress.connect([popup, elements](tgui::String item, tgui::String id) {
		elements[id.toInt()].second();
		popup->deselectItem();
	});
	return popup;
}

sf::VertexArray generateVertexArray(std::vector<std::shared_ptr<EMPAction>> actions, float timeResolution, float x, float y, bool invertY, sf::Color startColor, sf::Color endColor) {
	float totalTime = 0;
	for (auto action : actions) {
		totalTime += action->getTime();
	}

	sf::VertexArray ret;
	float totalTimeElapsed = 0;
	float time = 0;
	float curX = x, curY = y;
	int invertYMultiplier = 1;
	if (invertY) {
		invertYMultiplier = -1;
	}
	for (auto action : actions) {
		if (skippedByGenerateVertexArray(action)) {
			// Skip action if it requires the registry
			totalTimeElapsed += action->getTime();
			continue;
		}

		auto mp = action->generateStandaloneMP(curX, curY, 0, 0);
		while (time < action->getTime()) {
			sf::Vector2f pos = mp->compute(sf::Vector2f(0, 0), time);
			pos.y *= invertYMultiplier;
			sf::Color color = sf::Color((endColor.r - startColor.r) * (totalTimeElapsed / totalTime) + startColor.r, (endColor.g - startColor.g) * (totalTimeElapsed / totalTime) + startColor.g, (endColor.b - startColor.b) * (totalTimeElapsed / totalTime) + startColor.b, (endColor.a - startColor.a) * (totalTimeElapsed / totalTime) + startColor.a);
			ret.append(sf::Vertex(pos + sf::Vector2f(curX, curY), color));
			time += timeResolution;
			totalTimeElapsed += timeResolution;
		}
		time -= action->getTime();
		sf::Vector2f newPos = mp->compute(sf::Vector2f(0, 0), mp->getLifespan());
		curX += newPos.x;
		if (invertY) {
			newPos.y *= -1;
		}
		curY += newPos.y;
	}
	return ret;
}

sf::VertexArray generateVertexArray(std::shared_ptr<EMPAction> action, float timeResolution, float x, float y, sf::Color startColor, sf::Color endColor) {
	float totalTime = action->getTime();
	sf::VertexArray ret;
	float totalTimeElapsed = 0;
	float time = 0;
	auto mp = action->generateStandaloneMP(x, y, 0, 0);
	while (time < action->getTime()) {
		sf::Vector2f pos = mp->compute(sf::Vector2f(0, 0), time);
		sf::Color color = sf::Color((endColor.r - startColor.r) * (totalTimeElapsed / totalTime) + startColor.r, (endColor.g - startColor.g) * (totalTimeElapsed / totalTime) + startColor.g, (endColor.b - startColor.b) * (totalTimeElapsed / totalTime) + startColor.b, (endColor.a - startColor.a) * (totalTimeElapsed / totalTime) + startColor.a);
		ret.append(sf::Vertex(pos + sf::Vector2f(x, y), color));
		time += timeResolution;
		totalTimeElapsed += timeResolution;
	}
	return ret;
}

std::vector<std::pair<std::vector<float>, std::vector<float>>> generateMPPoints(std::shared_ptr<PiecewiseTFV> tfv, float tfvLifespan, float timeResolution) {
	std::vector<std::pair<std::vector<float>, std::vector<float>>> ret;
	float time = 0;
	int prevSegmentIndex = -1;
	std::vector<float> singleSegmentX;
	std::vector<float> singleSegmentY;
	while (time < tfvLifespan) {
		try {
			std::pair<float, int> valueAndSegmentIndex = tfv->piecewiseEvaluate(time);
			sf::Vector2f pos(time, valueAndSegmentIndex.first);

			if (valueAndSegmentIndex.second != prevSegmentIndex) {
				if (prevSegmentIndex != -1) {
					// Put a point at the end of the segment
					float nextSegmentStartTime = tfv->getSegment(valueAndSegmentIndex.second).first;
					sf::Vector2f endPos(nextSegmentStartTime, tfv->getSegment(prevSegmentIndex).second->evaluate(nextSegmentStartTime));
					singleSegmentX.push_back(pos.x);
					singleSegmentY.push_back(pos.y);

					ret.push_back(std::make_pair(singleSegmentX, singleSegmentY));
					singleSegmentX.clear();
					singleSegmentY.clear();
				}

				prevSegmentIndex = valueAndSegmentIndex.second;
			}

			singleSegmentX.push_back(pos.x);
			singleSegmentY.push_back(pos.y);

			time += timeResolution;
		} catch (InvalidEvaluationDomainException e) {
			// Skip to a valid time
			if (tfv->getSegmentsCount() > 0) {
				time = tfv->getSegment(0).first;
			} else {
				break;
			}
		}
	}
	ret.push_back(std::make_pair(singleSegmentX, singleSegmentY));
	return ret;
}

bool skippedByGenerateVertexArray(std::shared_ptr<EMPAction> empa) {
	if (std::dynamic_pointer_cast<MovePlayerHomingEMPA>(empa) || std::dynamic_pointer_cast<MoveGlobalHomingEMPA>(empa)) {
		return true;
	}
	return false;
}