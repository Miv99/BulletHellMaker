#include <Editor/CustomWidgets/SingleMarkerPlacer.h>

SingleMarkerPlacer::SingleMarkerPlacer(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution, int undoStackSize) : MarkerPlacer(parentWindow, clipboard, resolution, undoStackSize) {
	addMarker->setVisible(false);
	deleteMarker->setVisible(false);

	setMarkers({ std::make_pair(sf::Vector2f(0, 0), sf::Color::Red) });
}

std::string SingleMarkerPlacer::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Disable marker paste
	return "";
}

void SingleMarkerPlacer::manualDelete() {
	// Disable marker deletion
}