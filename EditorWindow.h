#pragma once
#include "MovablePoint.h"
#include <memory>
#include <string>
#include <SFML/Graphics.hpp>

class EditorWindow {
public:
	EditorWindow(std::string windowTitle, int width, int height);

	/*
	Starts the main loop.
	This function blocks the current thread.
	*/
	void start();

	void pause();
	void resume();

private:
	void physicsUpdate(float deltaTime);
	void render(float deltaTime);
	void handleEvent(sf::Event event);

	std::shared_ptr<sf::RenderWindow> window;
	std::string windowTitle;
	int windowWidth, windowHeight;

	bool windowCloseQueued = false;
	bool paused = false;
};