#include "ViewController.h"
#include <algorithm>

sf::View ViewController::handleEvent(sf::View view, sf::Event event) {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Dash) {
				setCameraZoom(view, std::max(0.2f, cameraZoom - 0.2f));
			} else if (event.key.code == sf::Keyboard::Equal) {
				setCameraZoom(view, std::min(4.0f, cameraZoom + 0.2f));
			}
		} else if (event.type == sf::Event::MouseWheelScrolled) {
			if (event.mouseWheelScroll.delta < 0) {
				setCameraZoom(view, std::max(0.2f, cameraZoom - 0.2f));
			} else if (event.mouseWheelScroll.delta > 0) {
				setCameraZoom(view, std::min(4.0f, cameraZoom + 0.2f));
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
			sf::Vector2f diff = window.mapPixelToCoords(sf::Vector2i(previousCameraDragCoordsX, previousCameraDragCoordsY)) - window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
			moveCamera(view, diff.x, diff.y);

			previousCameraDragCoordsX = event.mouseMove.x;
			previousCameraDragCoordsY = event.mouseMove.y;
		}
	} else if (event.type == sf::Event::MouseButtonReleased) {
		if (draggingCamera && event.mouseButton.button == sf::Mouse::Middle) {
			draggingCamera = false;
		}
	}

	return view;
}

void ViewController::setCameraZoom(sf::View& view, float zoom) {
	cameraZoom = zoom;
	view.setSize(originalViewWidth / zoom, originalViewHeight / zoom);
}

void ViewController::moveCamera(sf::View& view, float xOffset, float yOffset) {
	auto currentCenter = view.getCenter();
	view.setCenter(currentCenter.x + xOffset, currentCenter.y + yOffset);
}