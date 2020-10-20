#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

class ViewController {
public:
	/*
	controllableWithWASD - whether WASD can be used to pan the camera
	controllableWithArrowKeys - whether arrow keys can be used to pan the camera
	maxCameraZoom - the max camera zoom level
	*/
	ViewController(sf::RenderWindow& window, bool controllableWithWASD = true, 
		bool controllableWithArrowKeys = true, float maxCameraZoom = 8.0f);

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
	void setOriginalViewSize(float width, float height);

	void setMaxCameraZoom(sf::View& view, float maxCameraZoom);

	inline float getZoomAmount() const { return cameraZoom; }
	entt::SigH<void(float)>& getOnZoomAmountChange() { return onCameraZoomChange; }

private:
	static const float MIN_CAMERA_ZOOM;
	static const int NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM;
	static const float KEYBOARD_PAN_STRENGTH;

	sf::RenderWindow& window;

	// The original width and height of the view at camera zoom level 1
	float originalViewWidth, originalViewHeight;

	// Screen coordinates of the mouse when camera dragging started
	sf::Vector2i draggingStartScreenCoords;
	// View center when camera dragging started
	sf::Vector2f draggingStartViewCenter;
	bool draggingCamera = false;
	// cameraZoom should be modified only by setCameraZoom()
	float cameraZoom = 1.0f;
	// Always in range [0, NUM_ZOOM_ACTIONS_TO_CHANGE_FROM_MIN_TO_MAX_ZOOM)
	int cameraZoomIntegerLevel;
	float maxCameraZoom;
	float cameraZoomDelta;

	bool controllableWithWASD;
	bool controllableWithArrowKeys;

	// Whether the camera is currently being panned by keyboard
	bool panningUp = false, panningDown = false, panningLeft = false, panningRight = false;

	// Parameter: the new zoom amount
	entt::SigH<void(float)> onCameraZoomChange;

	void setCameraZoom(sf::View& view, int cameraZoomIntegerLevel);
	/*
	Moves the camera by some amount of world coordinates.
	*/
	void moveCamera(sf::View& view, float xOffset, float yOffset);
};