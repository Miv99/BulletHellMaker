#pragma once
#include <SFML/Graphics.hpp>

class ViewController {
public:
	inline ViewController(const sf::RenderWindow& window, bool controllableWithWASD = true, bool controllableWithArrowKeys = true) 
		: window(window), controllableWithWASD(controllableWithWASD), controllableWithArrowKeys(controllableWithArrowKeys) {}

	/*
	Modifies view.
	Should be called whenever the window receives an event.
	Returns whether the event should be consumed by this handleEvent() call.
	*/
	bool handleEvent(sf::View& view, sf::Event event);

	/*
	Modifies view.
	Should be called every render frame.
	*/
	bool update(sf::View& view, float deltaTime);

	/*
	This should be used to set the view's view zone.
	*/
	void setViewZone(sf::View& view, sf::FloatRect viewZone);

	/*
	Sets the size of the view at 1x camera zoom.
	*/
	inline void setOriginalViewSize(float width, float height) {
		originalViewWidth = width;
		originalViewHeight = height;
	}

	inline float getZoomAmount() { return cameraZoom; }

private:
	static const float MIN_CAMERA_ZOOM;
	static const float MAX_CAMERA_ZOOM;
	static const float CAMERA_ZOOM_DELTA;
	static const float KEYBOARD_PAN_STRENGTH;

	const sf::RenderWindow& window;

	// The original width and height of the view at camera zoom level 1
	float originalViewWidth, originalViewHeight;

	// Screen coordinates of the mouse in the last MouseMove event while
	// draggingCamera was true
	int previousCameraDragCoordsX, previousCameraDragCoordsY;
	bool draggingCamera = false;
	// This should be modified only by setCameraZoom()
	float cameraZoom = 1.0f;

	bool controllableWithWASD;
	bool controllableWithArrowKeys;

	// Whether the camera is currently being panned by keyboard
	bool panningUp = false, panningDown = false, panningLeft = false, panningRight = false;

	void setCameraZoom(sf::View& view, float zoom);
	/*
	Moves the camera by some amount of world coordinates.
	*/
	void moveCamera(sf::View& view, float xOffset, float yOffset);
};