#include <Editor/Windows/EditorWindow.h>

#include <algorithm>
#include <sstream>
#include <iterator>
#include <set>

#include <GuiConfig.h>
#include <Mutex.h>
#include <Editor/Util/EditorUtils.h>

const float EditorWindow::PROMPT_BUTTON_WIDTH = 100.0f;

EditorWindow::EditorWindow(std::string windowTitle, int width, int height, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval) :
	windowTitle(windowTitle), windowWidth(width), windowHeight(height), scaleWidgetsOnResize(scaleWidgetsOnResize), letterboxingEnabled(letterboxingEnabled), renderInterval(renderInterval) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	gui = std::make_shared<tgui::Gui>();
	closeSignal = std::make_shared<entt::SigH<void()>>();

	confirmationWindow = ChildWindow::create();
	confirmationWindow->setTitleButtons(tgui::ChildWindow::TitleButton::None);

	confirmationText = tgui::Label::create();
	confirmationYes = tgui::Button::create();
	confirmationNo = tgui::Button::create();
	confirmationCancel = tgui::Button::create();
	confirmationText->setTextSize(TEXT_SIZE);
	confirmationYes->setTextSize(TEXT_SIZE);
	confirmationNo->setTextSize(TEXT_SIZE);
	confirmationText->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	confirmationYes->setText("Yes");
	confirmationNo->setText("No");
	confirmationCancel->setText("Cancel");
	confirmationYes->setSize(PROMPT_BUTTON_WIDTH, TEXT_BUTTON_HEIGHT);
	confirmationNo->setSize(PROMPT_BUTTON_WIDTH, TEXT_BUTTON_HEIGHT);
	confirmationCancel->setSize(PROMPT_BUTTON_WIDTH, TEXT_BUTTON_HEIGHT);

	confirmationWindow->connect("SizeChanged", [this](sf::Vector2f newSize) {
		confirmationText->setMaximumTextWidth(confirmationWindow->getSize().x - GUI_PADDING_X * 2);
		if (confirmationCancel->isVisible()) {
			// All 3 buttons are visible
			confirmationCancel->setPosition(newSize.x - tgui::bindWidth(confirmationNo) - GUI_PADDING_X, newSize.y - tgui::bindHeight(confirmationNo) - GUI_PADDING_Y);
			confirmationNo->setPosition(tgui::bindLeft(confirmationCancel) - tgui::bindWidth(confirmationNo) - GUI_PADDING_X, tgui::bindTop(confirmationCancel));
			confirmationYes->setPosition(tgui::bindLeft(confirmationNo) - tgui::bindWidth(confirmationYes) - GUI_PADDING_X, tgui::bindTop(confirmationNo));
		} else {
			// Only confirmationYes and confirmationNo are visible
			confirmationNo->setPosition(newSize.x - tgui::bindWidth(confirmationNo) - GUI_PADDING_X, newSize.y - tgui::bindHeight(confirmationNo) - GUI_PADDING_Y);
			confirmationYes->setPosition(tgui::bindLeft(confirmationNo) - tgui::bindWidth(confirmationYes) - GUI_PADDING_X, tgui::bindTop(confirmationNo));
		}
	});

	confirmationWindow->add(confirmationText);
	confirmationWindow->add(confirmationYes);
	confirmationWindow->add(confirmationNo);
	confirmationWindow->add(confirmationCancel);
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

std::shared_ptr<entt::SigH<void(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE)>> EditorWindow::promptConfirmation(std::string message, 
	tgui::Widget* widgetToFocusAfter, bool includeCancelButton) {

	// Don't allow 2 confirmation prompts at the same time
	if (confirmationPanelOpen) {
		return nullptr;
	}

	std::shared_ptr<entt::SigH<void(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE)>> confirmationSignal = std::make_shared<entt::SigH<void(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE)>>();

	// Disable all widgets
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
	for (std::shared_ptr<tgui::Widget> ptr : gui->getWidgets()) {
		if (ptr->isEnabled()) {
			widgetsToBeEnabledAfterConfirmationPrompt.push_back(ptr);
		}
		ptr->setEnabled(false);
	}

	confirmationYes->setVisible(true);
	confirmationYes->connect("Pressed", [this, confirmationSignal, widgetToFocusAfter]() {
		confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});
	confirmationNo->setVisible(true);
	confirmationNo->connect("Pressed", [this, confirmationSignal, widgetToFocusAfter]() {
		confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::NO);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});

	confirmationCancel->setVisible(includeCancelButton);
	if (includeCancelButton) {
		confirmationCancel->setText("Cancel");
		confirmationCancel->connect("Pressed", [this, confirmationSignal, widgetToFocusAfter]() {
			confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::CANCEL);
			confirmationSignal->sink().disconnect();
			closeConfirmationPanel();

			if (widgetToFocusAfter) {
				widgetToFocusAfter->setFocused(true);
			}
		});

		confirmationWindow->setFallbackEventHandler([this, confirmationSignal, widgetToFocusAfter](sf::Event event) {
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
				confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::CANCEL);
				confirmationSignal->sink().disconnect();
				closeConfirmationPanel();

				if (widgetToFocusAfter) {
					widgetToFocusAfter->setFocused(true);
				}

				return true;
			}
			return false;
		});

		confirmationWindow->setSize(std::max(calculateMinPromptPanelWidth(3), getWindow()->getSize().x * 0.5f), std::max(350.0f, getWindow()->getSize().y * 0.5f));
	} else {
		confirmationWindow->setFallbackEventHandler([this, confirmationSignal, widgetToFocusAfter](sf::Event event) {
			return false;
		});

		confirmationWindow->setSize(std::max(calculateMinPromptPanelWidth(2), getWindow()->getSize().x * 0.5f), std::max(350.0f, getWindow()->getSize().y * 0.5f));
	}

	confirmationText->setText(message);
	confirmationWindow->setPosition(window->getSize().x / 2.0f - confirmationWindow->getSize().x / 2.0f, window->getSize().y / 2.0f - confirmationWindow->getSize().y / 2.0f);
	addChildWindow(confirmationWindow);
	confirmationPanelOpen = true;

	return confirmationSignal;
}

void EditorWindow::showPopupMessageWindow(std::string message, tgui::Widget* widgetToFocusAfter) {
	// Don't allow 2 confirmation prompts at the same time
	if (confirmationPanelOpen) {
		return;
	}

	// Disable all widgets
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
	for (std::shared_ptr<tgui::Widget> ptr : gui->getWidgets()) {
		if (ptr->isEnabled()) {
			widgetsToBeEnabledAfterConfirmationPrompt.push_back(ptr);
		}
		ptr->setEnabled(false);
	}

	confirmationYes->setVisible(false);
	confirmationNo->setVisible(false);

	confirmationCancel->setText("Ok");
	confirmationCancel->setVisible(true);
	confirmationCancel->connect("Pressed", [this, widgetToFocusAfter]() {
		closeConfirmationPanel();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});

	confirmationWindow->setFallbackEventHandler([this, widgetToFocusAfter](sf::Event event) {
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			closeConfirmationPanel();

			if (widgetToFocusAfter) {
				widgetToFocusAfter->setFocused(true);
			}

			return true;
		}
		return false;
	});

	confirmationWindow->setSize(std::max(calculateMinPromptPanelWidth(1), getWindow()->getSize().x * 0.5f), std::max(350.0f, getWindow()->getSize().y * 0.5f));

	confirmationText->setText(message);
	confirmationWindow->setPosition(window->getSize().x / 2.0f - confirmationWindow->getSize().x / 2.0f, window->getSize().y / 2.0f - confirmationWindow->getSize().y / 2.0f);
	addChildWindow(confirmationWindow);
	confirmationPanelOpen = true;
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
	if (vertexArrays.find(id) != vertexArrays.end()) {
		vertexArrays.erase(id);
	}
}

void EditorWindow::modifyVertexArray(int id, sf::VertexArray newVertexArray) {
	if (vertexArrays.find(id) != vertexArrays.end()) {
		vertexArrays[id] = newVertexArray;
	}
}

void EditorWindow::removeAllVertexArrays() {
	vertexArrays.clear();
}

void EditorWindow::addChildWindow(std::shared_ptr<ChildWindow> childWindow) {
	childWindows.insert(childWindow);
	gui->add(childWindow);
}

void EditorWindow::removeChildWindow(std::shared_ptr<ChildWindow> childWindow) {
	childWindows.erase(childWindow);
	gui->remove(childWindow);
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
		// Check if any child window is focused first and call its event handler
		for (std::shared_ptr<ChildWindow> childWindow : childWindows) {
			if (childWindow->isFocused()) {
				if (childWindow->handleEvent(event)) {
					return true;
				}
				break;
			}
		}

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

float EditorWindow::calculateMinPromptPanelWidth(int numButtons) {
	return GUI_PADDING_X * (numButtons + 1) + PROMPT_BUTTON_WIDTH * numButtons;
}

void EditorWindow::closeConfirmationPanel() {
	removeChildWindow(confirmationWindow);
	confirmationPanelOpen = false;

	for (auto ptr : widgetsToBeEnabledAfterConfirmationPrompt) {
		ptr->setEnabled(true);
	}
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
}