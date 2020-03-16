#include "ViewController.h"
#include <algorithm>

bool ViewController::handleEvent(sf::View& view, sf::Event event) {
	bool consumeEvent = false;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Dash) {
				setCameraZoom(view, std::max(0.2f, cameraZoom - 0.2f));
				consumeEvent = true;
			} else if (event.key.code == sf::Keyboard::Equal) {
				setCameraZoom(view, std::min(4.0f, cameraZoom + 0.2f));
				consumeEvent = true;
			}
		} else if (event.type == sf::Event::MouseWheelScrolled) {
			if (event.mouseWheelScroll.delta < 0) {
				setCameraZoom(view, std::max(0.2f, cameraZoom - 0.2f));
				consumeEvent = true;
			} else if (event.mouseWheelScroll.delta > 0) {
				setCameraZoom(view, std::min(4.0f, cameraZoom + 0.2f));
				consumeEvent = true;
			}
		}
	}
	
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Middle) {
			draggingCamera = true;
			previousCameraDragCoordsX = event.mouseButton.x;
			previousCameraDragCoordsY = event.mouseButton.y;
		}
	} else if (event.type == sf::Event::MouseMoved) {
		if (draggingCamera) {
			// Move camera depending on difference in world coordinates between event.mouseMove.x/y and previousCameraDragCoordsX/Y
			// magic numbers idk why you have to multiply by 0.86; it's not perfect but hardly noticeable
			sf::Vector2f diff = (window.mapPixelToCoords(sf::Vector2i(previousCameraDragCoordsX, previousCameraDragCoordsY)) - window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y))) / (cameraZoom * 0.86f);
			moveCamera(view, diff.x, diff.y);

			previousCameraDragCoordsX = event.mouseMove.x;
			previousCameraDragCoordsY = event.mouseMove.y;

			consumeEvent = true;
		}
	} else if (event.type == sf::Event::MouseButtonReleased) {
		if (draggingCamera && event.mouseButton.button == sf::Mouse::Middle) {
			draggingCamera = false;
		}
	}

	return consumeEvent;
}

void ViewController::setCameraZoom(sf::View& view, float zoom) {
	cameraZoom = zoom;
	view.setSize(originalViewWidth / zoom, originalViewHeight / zoom);
}

void ViewController::moveCamera(sf::View& view, float xOffset, float yOffset) {
	auto currentCenter = view.getCenter();
	view.setCenter(currentCenter.x + xOffset, currentCenter.y + yOffset);
}