#include <Editor/ViewController.h>

#include <algorithm>

#include <iostream>

const float ViewController::MIN_CAMERA_ZOOM = 0.2f;
const int ViewController::NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM = 38;
const float ViewController::KEYBOARD_PAN_STRENGTH = 300.0f;

ViewController::ViewController(sf::RenderWindow& window, bool controllableWithWASD, bool controllableWithArrowKeys, float maxCameraZoom)
	: window(window), controllableWithWASD(controllableWithWASD), controllableWithArrowKeys(controllableWithArrowKeys) {
	
	// Set cameraZoomIntegerLevel such that zoom level is as close to 1.0 as possible
	cameraZoomIntegerLevel = std::lround(std::log((1 - MIN_CAMERA_ZOOM) / (maxCameraZoom - MIN_CAMERA_ZOOM)) / (NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM * 0.1177f) + NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM);

	cameraZoomDelta = (maxCameraZoom - MIN_CAMERA_ZOOM) / NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM;
}

bool ViewController::handleEvent(sf::View& view, sf::Event event) {
	bool consumeEvent = false;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
		if (event.type == sf::Event::KeyPressed) {
			// TODO: change this so that cameraZoom changes nonlinearly
			if (event.key.code == sf::Keyboard::Dash) {
				setCameraZoom(view, cameraZoomIntegerLevel - 1);
				consumeEvent = true;
			} else if (event.key.code == sf::Keyboard::Equal) {
				setCameraZoom(view, cameraZoomIntegerLevel + 1);
				consumeEvent = true;
			}
		} else if (event.type == sf::Event::MouseWheelScrolled) {
			if (event.mouseWheelScroll.delta < 0) {
				setCameraZoom(view, cameraZoomIntegerLevel - 1);
				consumeEvent = true;
			} else if (event.mouseWheelScroll.delta > 0) {
				setCameraZoom(view, cameraZoomIntegerLevel + 1);
				consumeEvent = true;
			}
		}
	}
	
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Middle || event.mouseButton.button == sf::Mouse::Right) {
			draggingCamera = true;
			draggingStartScreenCoords = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
			draggingStartViewCenter = view.getCenter();
		}
	} else if (event.type == sf::Event::MouseMoved) {
		if (draggingCamera) {
			// Move camera depending on difference in world coordinates between event.mouseMove.x/y and draggingStartWorldCoords
			const sf::View oldView = window.getView();
			window.setView(view);
			sf::Vector2i screenDiff = sf::Vector2i(event.mouseMove.x, event.mouseMove.y) - draggingStartScreenCoords;
			sf::Vector2f diff = window.mapPixelToCoords(screenDiff) - window.mapPixelToCoords(sf::Vector2i(0, 0));
			window.setView(oldView);

			view.setCenter(draggingStartViewCenter.x - diff.x, draggingStartViewCenter.y - diff.y);

			consumeEvent = true;
		}
	} else if (event.type == sf::Event::MouseButtonReleased) {
		if (draggingCamera && (event.mouseButton.button == sf::Mouse::Middle || event.mouseButton.button == sf::Mouse::Right)) {
			draggingCamera = false;
		}
	} else if (event.type == sf::Event::KeyPressed) {
		if (controllableWithWASD) {
			if (event.key.code == sf::Keyboard::W) {
				panningUp = true;
			} else if (event.key.code == sf::Keyboard::S) {
				panningDown = true;
			} else if (event.key.code == sf::Keyboard::A) {
				panningLeft = true;
			} else if (event.key.code == sf::Keyboard::D) {
				panningRight = true;
			}
		}
		if (controllableWithArrowKeys) {
			if (event.key.code == sf::Keyboard::Up) {
				panningUp = true;
			} else if (event.key.code == sf::Keyboard::Down) {
				panningDown = true;
			} else if (event.key.code == sf::Keyboard::Left) {
				panningLeft = true;
			} else if (event.key.code == sf::Keyboard::Right) {
				panningRight = true;
			}
		}
	} else if (event.type == sf::Event::KeyReleased) {
		if (controllableWithWASD) {
			if (event.key.code == sf::Keyboard::W) {
				panningUp = false;
			} else if (event.key.code == sf::Keyboard::S) {
				panningDown = false;
			} else if (event.key.code == sf::Keyboard::A) {
				panningLeft = false;
			} else if (event.key.code == sf::Keyboard::D) {
				panningRight = false;
			}
		}
		if (controllableWithArrowKeys) {
			if (event.key.code == sf::Keyboard::Up) {
				panningUp = false;
			} else if (event.key.code == sf::Keyboard::Down) {
				panningDown = false;
			} else if (event.key.code == sf::Keyboard::Left) {
				panningLeft = false;
			} else if (event.key.code == sf::Keyboard::Right) {
				panningRight = false;
			}
		}
	}

	return consumeEvent;
}

bool ViewController::update(sf::View& view, float deltaTime) {
	bool requireScreenRefresh = false;

	if (panningUp) {
		moveCamera(view, 0, -KEYBOARD_PAN_STRENGTH / cameraZoom * deltaTime);
		requireScreenRefresh = true;
	} else if (panningDown) {
		moveCamera(view, 0, KEYBOARD_PAN_STRENGTH / cameraZoom * deltaTime);
		requireScreenRefresh = true;
	}

	if (panningLeft) {
		moveCamera(view, -KEYBOARD_PAN_STRENGTH / cameraZoom * deltaTime, 0);
		requireScreenRefresh = true;
	} else if (panningRight) {
		moveCamera(view, KEYBOARD_PAN_STRENGTH / cameraZoom * deltaTime, 0);
		requireScreenRefresh = true;
	}

	return requireScreenRefresh;
}

void ViewController::setViewZone(sf::View& view, sf::FloatRect viewZone) {
	view.setSize(originalViewWidth / cameraZoom, originalViewHeight / cameraZoom);
	view.setCenter(viewZone.left + viewZone.width / 2.0f, viewZone.top + viewZone.height / 2.0f);
}

void ViewController::setOriginalViewSize(float width, float height) {
	originalViewWidth = width;
	originalViewHeight = height;
}

void ViewController::setMaxCameraZoom(sf::View& view, float maxCameraZoom) {
	this->maxCameraZoom = maxCameraZoom;

	// Set cameraZoomIntegerLevel such that zoom level is as close to 1.0 as possible using the cameraZoom curve defined in setCameraZoom()
	cameraZoomIntegerLevel = std::max(0, std::min(NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM - 1,
		(int)(std::lround(std::log((1.0f - MIN_CAMERA_ZOOM)/(maxCameraZoom - MIN_CAMERA_ZOOM)) + std::exp(0.1177f * (0 - (NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM - 1)))) / 0.1177f + (NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM - 1))));

	cameraZoomDelta = (maxCameraZoom - MIN_CAMERA_ZOOM) / NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM;

	setCameraZoom(view, cameraZoomIntegerLevel);
}

void ViewController::setCameraZoom(sf::View& view, int cameraZoomIntegerLevel) {
	if (cameraZoomIntegerLevel < 0 || cameraZoomIntegerLevel >= NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM) {
		return;
	}

	this->cameraZoomIntegerLevel = cameraZoomIntegerLevel;
	cameraZoom = std::max(0.0f, std::exp(0.1177f * (cameraZoomIntegerLevel - (NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM - 1))) - std::exp(0.1177f * (0 - (NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM - 1))))
		* (maxCameraZoom - MIN_CAMERA_ZOOM) + MIN_CAMERA_ZOOM;

	view.setSize(originalViewWidth / cameraZoom, originalViewHeight / cameraZoom);

	onCameraZoomChange.publish(cameraZoom);
}

void ViewController::moveCamera(sf::View& view, float xOffset, float yOffset) {
	auto currentCenter = view.getCenter();
	view.setCenter(currentCenter.x + xOffset, currentCenter.y + yOffset);
}