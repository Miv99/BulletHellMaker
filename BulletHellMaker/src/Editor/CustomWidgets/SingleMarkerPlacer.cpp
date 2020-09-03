#include <Editor/CustomWidgets/SingleMarkerPlacer.h>

SingleMarkerPlacer::SingleMarkerPlacer(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution, int undoStackSize) : MarkerPlacer(parentWindow, clipboard, resolution, undoStackSize) {
	addMarker->setVisible(false);
	deleteMarker->setVisible(false);

	setMarkers({ std::make_pair(sf::Vector2f(0, 0), sf::Color::Red) });
}

PasteOperationResult SingleMarkerPlacer::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Disable marker paste
	return PasteOperationResult(false, "");
}

void SingleMarkerPlacer::manualDelete() {
	// Disable marker deletion
}