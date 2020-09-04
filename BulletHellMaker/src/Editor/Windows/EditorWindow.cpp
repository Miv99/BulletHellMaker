#include <Editor/Windows/EditorWindow.h>

#include <algorithm>
#include <sstream>
#include <iterator>
#include <set>

#include <GuiConfig.h>
#include <Mutex.h>
#include <Editor/Util/EditorUtils.h>

EditorWindow::EditorWindow(std::string windowTitle, int width, int height, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval) :
	windowTitle(windowTitle), windowWidth(width), windowHeight(height), scaleWidgetsOnResize(scaleWidgetsOnResize), letterboxingEnabled(letterboxingEnabled), renderInterval(renderInterval) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	gui = std::make_shared<tgui::Gui>();
	closeSignal = std::make_shared<entt::SigH<void()>>();

	confirmationPanel = tgui::Panel::create();
	confirmationText = tgui::Label::create();
	confirmationYes = tgui::Button::create();
	confirmationNo = tgui::Button::create();
	confirmationText->setTextSize(TEXT_SIZE);
	confirmationYes->setTextSize(TEXT_SIZE);
	confirmationNo->setTextSize(TEXT_SIZE);
	confirmationText->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	confirmationYes->setText("Yes");
	confirmationNo->setText("No");
	confirmationYes->setSize(100.0f, TEXT_BUTTON_HEIGHT);
	confirmationNo->setSize(100.0f, TEXT_BUTTON_HEIGHT);

	confirmationPanel->add(confirmationText);
	confirmationPanel->add(confirmationYes);
	confirmationPanel->add(confirmationNo);
}

void EditorWindow::start() {
	if (!window || !window->isOpen()) {
		// SFML requires the RenderWindow to be created in the thread

		std::lock_guard<std::recursive_mutex> lock(tguiMutex);
		window = std::make_shared<sf::RenderWindow>(sf::VideoMode(windowWidth, windowHeight), windowTitle, sf::Style::Default);
		window->setKeyRepeatEnabled(false);
		window->setActive(true);
		gui->setTarget(*window);

		updateWindowView(window->getSize().x, window->getSize().y);
		onRenderWindowInitialization();
	}

	sf::Clock deltaClock;
	sf::Clock renderClock;

	// Main loop
	while (window->isOpen()) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < RENDER_INTERVAL) {
			sf::Event event;
			while (!windowCloseQueued && window->pollEvent(event)) {
				if (windowCloseQueued || event.type == sf::Event::Closed) {
					closeSignal->publish();
					window->close();
					return;
				} else if (event.type == sf::Event::Resized) {
					updateWindowView(event.size.width, event.size.height);
				} else {
					if (popup) {
						if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
							// When mouse is released, remove the pop-up menu

							closePopupWidget();
						}
					}
					handleEvent(event);
				}

				float dt = std::min(RENDER_INTERVAL, renderClock.restart().asSeconds());
				window->clear();
				render(dt);
				if (renderSignal) {
					renderSignal->publish(dt);
				}
				window->display();
			}

			if (windowCloseQueued) {
				return;
			}

			float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
			timeSinceLastRender += dt;
			physicsUpdate(dt);
		}

		float dt = std::min(RENDER_INTERVAL, renderClock.restart().asSeconds());
		window->clear();
		render(dt);
		if (renderSignal) {
			renderSignal->publish(dt);
		}
		window->display();
	}
}

void EditorWindow::startAndHide() {
	if (!window || !window->isOpen()) {
		// SFML requires the RenderWindow to be created in the thread

		std::lock_guard<std::recursive_mutex> lock(tguiMutex);
		window = std::make_shared<sf::RenderWindow>(sf::VideoMode(windowWidth, windowHeight), windowTitle, sf::Style::Default);
		window->setKeyRepeatEnabled(false);
		window->setActive(true);
		gui->setTarget(*window);

		updateWindowView(window->getSize().x, window->getSize().y);
		onRenderWindowInitialization();
	}
	hide();

	sf::Clock deltaClock;
	sf::Clock renderClock;

	// Main loop
	while (window->isOpen()) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < RENDER_INTERVAL) {
			sf::Event event;
			while (!windowCloseQueued && window->pollEvent(event)) {
				if (windowCloseQueued || event.type == sf::Event::Closed) {
					closeSignal->publish();
					window->close();
					return;
				} else if (event.type == sf::Event::Resized) {
					updateWindowView(event.size.width, event.size.height);
				} else {
					if (popup) {
						if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
							// When mouse is released, remove the pop-up menu

							closePopupWidget();
						}
					}
					handleEvent(event);
				}

				float dt = std::min(RENDER_INTERVAL, renderClock.restart().asSeconds());
				window->clear();
				render(dt);
				if (renderSignal) {
					renderSignal->publish(dt);
				}
				window->display();
			}

			float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
			timeSinceLastRender += dt;
			physicsUpdate(dt);
		}

		float dt = std::min(RENDER_INTERVAL, renderClock.restart().asSeconds());
		window->clear();
		render(dt);
		if (renderSignal) {
			renderSignal->publish(dt);
		}
		window->display();
	}
}

void EditorWindow::close() {
	if (window && window->isOpen()) {
		windowCloseQueued = true;
	}
}

void EditorWindow::hide() {
	if (window) {
		window->setVisible(false);
	}
}

void EditorWindow::show() {
	if (window) {
		window->setVisible(true);
	}
}

std::shared_ptr<entt::SigH<void(bool)>> EditorWindow::promptConfirmation(std::string message, tgui::Widget* widgetToFocusAfter) {
	// Don't allow 2 confirmation prompts at the same time
	if (confirmationPanelOpen) {
		return nullptr;
	}

	std::shared_ptr<entt::SigH<void(bool)>> confirmationSignal = std::make_shared<entt::SigH<void(bool)>>();

	// Disable all widgets
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
	for (std::shared_ptr<tgui::Widget> ptr : gui->getWidgets()) {
		if (ptr->isEnabled()) {
			widgetsToBeEnabledAfterConfirmationPrompt.push_back(ptr);
		}
		ptr->setEnabled(false);
	}

	confirmationYes->connect("Pressed", [this, confirmationSignal, widgetToFocusAfter]() {
		confirmationSignal->publish(true);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});
	confirmationNo->connect("Pressed", [this, confirmationSignal, widgetToFocusAfter]() {
		confirmationSignal->publish(false);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});

	confirmationText->setText(message);
	gui->add(confirmationPanel);
	confirmationPanelOpen = true;

	return confirmationSignal;
}

void EditorWindow::addPopupWidget(std::shared_ptr<tgui::Widget> popup, float preferredX, float preferredY, float preferredWidth, float preferredHeight, float maxWidthFraction, float maxHeightFraction) {
	if (this->popup) {
		closePopupWidget();
	}

	// Set popup position
	popup->setPosition(preferredX, preferredY);
	// Set popup size
	float popupWidth = std::min(preferredWidth, windowWidth * maxWidthFraction);
	float popupHeight = std::min(preferredHeight, windowHeight * maxHeightFraction);
	// Popup width/height can't be such that part of the popup goes past the window width/height
	float oldWidth = popupWidth;
	popupWidth = std::min(popupWidth, windowWidth - popup->getAbsolutePosition().x);
	float oldHeight = popupHeight;
	popupHeight = std::min(popupHeight, windowHeight - popup->getAbsolutePosition().y);
	// If it was going to go past the window width/height, move the popup back the amount it was going to go past the window width/height
	popup->setPosition(std::max(0.0f, popup->getPosition().x - (oldWidth - popupWidth)), std::max(0.0f, popup->getPosition().y - (oldHeight - popupHeight)));
	popup->setSize(oldWidth, oldHeight);

	this->popup = popup;
	popupContainer = nullptr;
	gui->add(popup);
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
	if (popupContainer) {
		popupContainer->remove(popup);
	} else {
		// If not in the popupContainer, it must be in the Gui
		gui->remove(popup);
	}
	popup = nullptr;
	popupContainer = nullptr;
}

int EditorWindow::addVertexArray(sf::VertexArray vertexArray) {
	int id = nextVertexArrayID;
	vertexArrays[id] = vertexArray;
	nextVertexArrayID++;
	return id;
}

void EditorWindow::removeVertexArray(int id) {
	if (vertexArrays.count(id) > 0) {
		vertexArrays.erase(id);
	}
}

void EditorWindow::modifyVertexArray(int id, sf::VertexArray newVertexArray) {
	if (vertexArrays.count(id) > 0) {
		vertexArrays[id] = newVertexArray;
	}
}

void EditorWindow::removeAllVertexArrays() {
	vertexArrays.clear();
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

	confirmationPanel->setSize(std::max(300.0f, windowWidth * 0.35f), std::max(200.0f, windowHeight * 0.25f));
	confirmationPanel->setPosition(windowWidth / 2.0f - confirmationPanel->getSize().x / 2.0f, windowHeight / 2.0f - confirmationPanel->getSize().y / 2.0f);
	confirmationText->setMaximumTextWidth(confirmationPanel->getSize().x - GUI_PADDING_X * 2);
	confirmationYes->setPosition(confirmationPanel->getSize().x/2.0f - confirmationYes->getSize().x - GUI_PADDING_X/2.0f, confirmationPanel->getSize().y - confirmationYes->getSize().y - GUI_PADDING_Y);
	confirmationNo->setPosition(tgui::bindRight(confirmationYes) + GUI_PADDING_X, tgui::bindTop(confirmationYes));

	if (resizeSignal) {
		resizeSignal->publish(windowWidth, windowHeight);
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

std::shared_ptr<entt::SigH<void()>> EditorWindow::getCloseSignal() {
	return closeSignal;
}

void EditorWindow::physicsUpdate(float deltaTime) {
}

void EditorWindow::render(float deltaTime) {
	std::lock_guard<std::recursive_mutex> lock(tguiMutex);
	gui->draw();

	for (auto it = vertexArrays.begin(); it != vertexArrays.end(); it++) {
		window->draw(it->second);
	}
}

bool EditorWindow::handleEvent(sf::Event event) {
	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	if (event.type == sf::Event::MouseMoved) {
		mousePos.x = event.mouseMove.x;
		mousePos.y = event.mouseMove.y;
	} else if (event.type == sf::Event::MouseButtonPressed) {
		lastMousePressPos = mousePos;
	}

	// Disable keyboard events when confirmation panel is open
	if (!(confirmationPanelOpen && (event.type == sf::Event::TextEntered || event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased))) {
		if (event.type == sf::Event::KeyPressed) {
			return gui->handleEvent(event);
		} else {
			gui->handleEvent(event);
			return false;
		}
	} else {
		return true;
	}
}

void EditorWindow::onRenderWindowInitialization() {
}

void EditorWindow::closeConfirmationPanel() {
	gui->remove(confirmationPanel);
	confirmationPanelOpen = false;

	for (auto ptr : widgetsToBeEnabledAfterConfirmationPrompt) {
		ptr->setEnabled(true);
	}
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
}