#include "EditorWindow.h"
#include <algorithm>

EditorWindow::EditorWindow(std::shared_ptr<std::mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval) :
	tguiMutex(tguiMutex), windowTitle(windowTitle), windowWidth(width), windowHeight(height), scaleWidgetsOnResize(scaleWidgetsOnResize), letterboxingEnabled(letterboxingEnabled), renderInterval(renderInterval) {
	std::lock_guard<std::mutex> lock(*tguiMutex);
	gui = std::make_shared<tgui::Gui>();
}

void EditorWindow::start() {
	if (!window) {
		// SFML requires the RenderWindow to be created in the thread

		std::lock_guard<std::mutex> lock(*tguiMutex);
		window = std::make_shared<sf::RenderWindow>(sf::VideoMode(windowWidth, windowHeight), windowTitle, sf::Style::Default);
		window->setKeyRepeatEnabled(false);
		window->setActive(true);
		gui->setTarget(*window);
	}

	sf::Clock deltaClock;

	// Main loop
	while (window->isOpen()) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < renderInterval) {
			sf::Event event;
			while (window->pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window->close();
				} else if (event.type == sf::Event::Resized) {
					updateWindowView(event.size.width, event.size.height);
					if (resizeSignal) {
						resizeSignal->publish(event.size.width, event.size.height);
					}
				} else {
					if (popup) {
						if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
							// When mouse is released, remove the pop-up menu

							closePopupWidget();
						}
					}
					handleEvent(event);
				}
			}

			float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
			physicsUpdate(dt);

			timeSinceLastRender += dt;
		}

		window->clear();
		render(timeSinceLastRender);
		if (renderSignal) {
			renderSignal->publish(timeSinceLastRender);
		}
		window->display();
	}
}

void EditorWindow::close() {
	window->close();
}

void EditorWindow::addPopupWidget(std::shared_ptr<tgui::Container> popupContainer, std::shared_ptr<tgui::Widget> popup) {
	if (this->popup) {
		closePopupWidget();
	}
	this->popup = popup;
	this->popupContainer = popupContainer;
	popupContainer->add(popup);
}

void EditorWindow::closePopupWidget() {
	popupContainer->remove(popup);
	popup = nullptr;
	popupContainer = nullptr;
}

void EditorWindow::updateWindowView(int windowWidth, int windowHeight) {
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	
	if (letterboxingEnabled) {
		sf::View view = window->getView();

		// Compares the aspect ratio of the window to the aspect ratio of the view,
		// and sets the view's viewport accordingly in order to archieve a letterbox effect.
		// A new view (with a new viewport set) is returned.
		float windowRatio = windowWidth / (float)windowHeight;
		float viewRatio = view.getSize().x / (float)view.getSize().y;
		float sizeX = 1;
		float sizeY = 1;
		float posX = 0;
		float posY = 0;

		bool horizontalSpacing = true;
		if (windowRatio < viewRatio)
			horizontalSpacing = false;

		// If horizontalSpacing is true, the black bars will appear on the left and right side.
		// Otherwise, the black bars will appear on the top and bottom.
		if (horizontalSpacing) {
			sizeX = viewRatio / windowRatio;
			posX = (1 - sizeX) / 2.f;
		} else {
			sizeY = windowRatio / viewRatio;
			posY = (1 - sizeY) / 2.f;
		}

		view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
		window->setView(view);
	}
	if (!scaleWidgetsOnResize) {
		gui->setView(sf::View(sf::Vector2f(windowWidth / 2.0f, windowHeight / 2.0f), sf::Vector2f(windowWidth, windowHeight)));
	}
}

std::shared_ptr<entt::SigH<void(float)>> EditorWindow::getRenderSignal() {
	if (!renderSignal) {
		renderSignal = std::make_shared<entt::SigH<void(float)>>();
	}
	return renderSignal;
}

std::shared_ptr<entt::SigH<void(int, int)>> EditorWindow::getResizeSignal() {
	if (!resizeSignal) {
		resizeSignal = std::make_shared<entt::SigH<void(int, int)>>();
	}
	return resizeSignal;
}

void EditorWindow::physicsUpdate(float deltaTime) {
}

void EditorWindow::render(float deltaTime) {
	std::lock_guard<std::mutex> lock(*tguiMutex);
	gui->draw();
}

void EditorWindow::handleEvent(sf::Event event) {
	std::lock_guard<std::mutex> lock(*tguiMutex);
	gui->handleEvent(event);
}

void UndoableEditorWindow::handleEvent(sf::Event event) {
	EditorWindow::handleEvent(event);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Z) {
			undoStack.undo();
		} else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Y) {
			undoStack.redo();
		}
	}
}
