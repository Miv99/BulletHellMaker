#pragma once
#include "MovablePoint.h"
#include "UndoStack.h"
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <entt/entt.hpp>

class EditorWindow {
public:
	/*
	renderInterval - time between each render call. If the gui has a ListBox, renderInterval should be some relatively large number (~0.1) because tgui gets
		messed up with multithreading.
	*/
	EditorWindow(std::shared_ptr<std::mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);

	/*
	Starts the main loop.
	This function blocks the current thread.
	*/
	void start();
	/*
	Closes the window.
	*/
	void close();
	/*
	Pauses the EditorWindow and hides it. The EditorWindow object data is maintained.
	*/
	void hide();

	void addPopupWidget(std::shared_ptr<tgui::Container> popupContainer, std::shared_ptr<tgui::Widget> popup);
	void closePopupWidget();

	/*
	Called every time window size changes.
	*/
	virtual void updateWindowView(int windowWidth, int windowHeight);

	std::shared_ptr<tgui::Gui> getGui() { return gui; }
	inline int getWindowWidth() { return windowWidth; }
	inline int getWindowHeight() { return windowHeight; }
	std::shared_ptr<entt::SigH<void(float)>> getRenderSignal();
	std::shared_ptr<entt::SigH<void(int, int)>> getResizeSignal();

protected:
	virtual void physicsUpdate(float deltaTime);
	virtual void render(float deltaTime);
	virtual void handleEvent(sf::Event event);

private:
	std::shared_ptr<sf::RenderWindow> window;
	std::string windowTitle;
	int windowWidth, windowHeight;

	std::shared_ptr<tgui::Gui> gui;
	// The popup widget, if one exists.
	// Only one popup widget can exist at any time. If a new widget pops up,
	// the old one is removed from the Gui. When the user clicks outside the popup
	// widget, it closes.
	std::shared_ptr<tgui::Widget> popup;
	// The container that contains the popup
	std::shared_ptr<tgui::Container> popupContainer;

	bool letterboxingEnabled;
	bool scaleWidgetsOnResize;

	// Mutex used to make sure multiple tgui widgets aren't being instantiated at the same time in different threads.
	// tgui::Gui draw() calls also can't be done at the same time.
	// Apparently tgui gets super messed up with multithreading.
	std::shared_ptr<std::mutex> tguiMutex;

	float renderInterval;
	// Signal that's emitted every time a render call is made
	// function accepts 1 argument: the time since the last render
	std::shared_ptr<entt::SigH<void(float)>> renderSignal;
	// Signal that's emitted every time the window resizes
	std::shared_ptr<entt::SigH<void(int, int)>> resizeSignal;
};

/*
An EditorWindow that has the capability of redoing and undoing commands in the UndoStack.
Undo is done with control+z and redo with control+y.
Commands must be added to the UndoStack by the user.
*/
class UndoableEditorWindow : public EditorWindow {
public:
	inline UndoableEditorWindow(std::shared_ptr<std::mutex> tguiMutex, std::string windowTitle, int width, int height, UndoStack& undoStack, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL) :
		EditorWindow(tguiMutex, windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval), undoStack(undoStack) {
	}

protected:
	virtual void handleEvent(sf::Event event);

private:
	UndoStack& undoStack;
};