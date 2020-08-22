#pragma once
#include <Editor/CustomWidgets/MarkerPlacer.h>

/*
A MarkerPlacer limited to editing only a single marker.
*/
class SingleMarkerPlacer : public MarkerPlacer {
public:
	SingleMarkerPlacer(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution = sf::Vector2u(MAP_WIDTH, MAP_HEIGHT), int undoStackSize = 50);
	static std::shared_ptr<SingleMarkerPlacer> create(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution = sf::Vector2u(MAP_WIDTH, MAP_HEIGHT), int undoStackSize = 50) {
		return std::make_shared<SingleMarkerPlacer>(parentWindow, clipboard, resolution, undoStackSize);
	}

	std::string pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;

protected:
	void manualDelete() override;
};