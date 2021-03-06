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

	confirmationInput = EditBox::create();
	confirmationInput->setTextSize(TEXT_SIZE);
	confirmationInput->setPosition(tgui::bindLeft(confirmationText), tgui::bindBottom(confirmationText) + GUI_LABEL_PADDING_Y);

	confirmationWindow->onSizeChange.connect([this]() {
		updateConfirmationWindowLayout();
	});

	confirmationWindow->add(confirmationText);
	confirmationWindow->add(confirmationInput);
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
					handleWindowCloseEvent();
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
					handleWindowCloseEvent();
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
	if (confirmationWindowOpen) {
		return nullptr;
	}

	if (includeCancelButton) {
		confirmationButtonLayout = CONFIRMATION_WINDOW_BUTTON_LAYOUT::CANCEL_NO_YES;
	} else {
		confirmationButtonLayout = CONFIRMATION_WINDOW_BUTTON_LAYOUT::NO_YES;
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

	confirmationInput->setVisible(false);

	confirmationYes->setText("Yes");
	confirmationYes->setVisible(true);
	confirmationYes->onPress.connect([this, confirmationSignal, widgetToFocusAfter]() {
		closeConfirmationWindow();

		confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES);
		confirmationSignal->sink().disconnect();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});
	confirmationNo->setVisible(true);
	confirmationNo->onPress.connect([this, confirmationSignal, widgetToFocusAfter]() {
		closeConfirmationWindow();

		confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::NO);
		confirmationSignal->sink().disconnect();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});

	confirmationCancel->setVisible(includeCancelButton);
	if (includeCancelButton) {
		confirmationCancel->setText("Cancel");
		confirmationCancel->onPress.connect([this, confirmationSignal, widgetToFocusAfter]() {
			closeConfirmationWindow();

			confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::CANCEL);
			confirmationSignal->sink().disconnect();

			if (widgetToFocusAfter) {
				widgetToFocusAfter->setFocused(true);
			}
		});

		confirmationWindow->setFallbackEventHandler([this, confirmationSignal, widgetToFocusAfter](sf::Event event) {
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
				closeConfirmationWindow();

				confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::CANCEL);
				confirmationSignal->sink().disconnect();

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
	confirmationWindow->setPosition(window->getSize().x / 2.0f - confirmationWindow->getClientSize().x / 2.0f, window->getSize().y / 2.0f - confirmationWindow->getClientSize().y / 2.0f);
	addChildWindow(confirmationWindow);
	confirmationWindowOpen = true;

	updateConfirmationWindowLayout();

	return confirmationSignal;
}

void EditorWindow::showPopupMessageWindow(std::string message, tgui::Widget* widgetToFocusAfter) {
	// Don't allow 2 confirmation prompts at the same time
	if (confirmationWindowOpen) {
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

	confirmationButtonLayout = CONFIRMATION_WINDOW_BUTTON_LAYOUT::OK;

	confirmationInput->setVisible(false);
	confirmationCancel->setVisible(false);
	confirmationNo->setVisible(false);

	confirmationYes->setText("Ok");
	confirmationYes->setVisible(true);
	confirmationYes->onPress.connect([this, widgetToFocusAfter]() {
		closeConfirmationWindow();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});

	confirmationWindow->setFallbackEventHandler([this, widgetToFocusAfter](sf::Event event) {
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			closeConfirmationWindow();

			if (widgetToFocusAfter) {
				widgetToFocusAfter->setFocused(true);
			}

			return true;
		}
		return false;
	});

	confirmationWindow->setSize(std::max(calculateMinPromptPanelWidth(1), getWindow()->getSize().x * 0.5f), std::max(350.0f, getWindow()->getSize().y * 0.5f));

	confirmationText->setText(message);
	confirmationWindow->setPosition(window->getSize().x / 2.0f - confirmationWindow->getClientSize().x / 2.0f, window->getSize().y / 2.0f - confirmationWindow->getClientSize().y / 2.0f);
	addChildWindow(confirmationWindow);
	confirmationWindowOpen = true;

	updateConfirmationWindowLayout();
}

std::shared_ptr<entt::SigH<void(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE, std::string)>> EditorWindow::promptInput(std::string message, 
	tgui::Widget* widgetToFocusAfter, bool includeCancelButton) {
	
	// Don't allow 2 confirmation prompts at the same time
	if (confirmationWindowOpen) {
		return nullptr;
	}

	if (includeCancelButton) {
		confirmationButtonLayout = CONFIRMATION_WINDOW_BUTTON_LAYOUT::CANCEL_OK;
	} else {
		confirmationButtonLayout = CONFIRMATION_WINDOW_BUTTON_LAYOUT::OK;
	}

	std::shared_ptr<entt::SigH<void(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE, std::string)>> confirmationSignal
		= std::make_shared<entt::SigH<void(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE, std::string)>>();

	// Disable all widgets
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
	for (std::shared_ptr<tgui::Widget> ptr : gui->getWidgets()) {
		if (ptr->isEnabled()) {
			widgetsToBeEnabledAfterConfirmationPrompt.push_back(ptr);
		}
		ptr->setEnabled(false);
	}

	confirmationInput->setVisible(true);

	confirmationYes->setText("Ok");
	confirmationYes->setVisible(true);
	confirmationYes->onPress.connect([this, confirmationSignal, widgetToFocusAfter]() {
		closeConfirmationWindow();

		confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES,
			static_cast<std::string>(confirmationInput->getText()));
		confirmationSignal->sink().disconnect();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});
	confirmationNo->setVisible(false);

	confirmationCancel->setVisible(includeCancelButton);
	if (includeCancelButton) {
		confirmationCancel->setText("Cancel");
		confirmationCancel->onPress.connect([this, confirmationSignal, widgetToFocusAfter]() {
			closeConfirmationWindow();

			confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::CANCEL,
				static_cast<std::string>(confirmationInput->getText()));
			confirmationSignal->sink().disconnect();

			if (widgetToFocusAfter) {
				widgetToFocusAfter->setFocused(true);
			}
		});

		confirmationWindow->setFallbackEventHandler([this, confirmationSignal, widgetToFocusAfter](sf::Event event) {
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
				closeConfirmationWindow();

				confirmationSignal->publish(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::CANCEL,
					static_cast<std::string>(confirmationInput->getText()));
				confirmationSignal->sink().disconnect();

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
	confirmationInput->setText("");
	confirmationWindow->setPosition(window->getSize().x / 2.0f - confirmationWindow->getClientSize().x / 2.0f, window->getSize().y / 2.0f - confirmationWindow->getClientSize().y / 2.0f);
	addChildWindow(confirmationWindow);
	confirmationWindowOpen = true;

	updateConfirmationWindowLayout();

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
	std::weak_ptr<ChildWindow> weakChildWindowPtr = std::weak_ptr(childWindow);

	childWindows.insert(childWindow);
	childWindow->onClose([this, weakChildWindowPtr]() {
		childWindows.erase(weakChildWindowPtr.lock());
	});
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
		gui->setAbsoluteView(tgui::FloatRect(0, 0, windowWidth, windowHeight));
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
		lastMousePressPos = sf::Vector2f(mousePos);
	}

	// Disable all events except KeyPressed when confirmation panel is open because confirmation panel
	// can be interacted with using KeyPressed events
	if (confirmationWindowOpen) {
		gui->handleEvent(event);
		return true;
	} else {
		// Check if any child window is focused first and call its event handler
		for (std::shared_ptr<ChildWindow> childWindow : childWindows) {
			if (childWindow->isFocused()) {
				if (childWindow->handleEvent(event)) {
					return true;
				}
				break;
			}
		}

		// Workaround for TGUI's gui consuming KeyPressed events that would otherwise be used
		if (event.type == sf::Event::KeyPressed && std::dynamic_pointer_cast<tgui::EditBox>(gui->getFocusedLeaf())) {
			gui->handleEvent(event);
			return true;
		} else {
			gui->handleEvent(event);
			return false;
		}
	}
}

void EditorWindow::onRenderWindowInitialization() {
}

void EditorWindow::handleWindowCloseEvent() {
	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	gui->removeAllWidgets();
	closeSignal->publish();
	window->close();
	window.reset();
}

float EditorWindow::calculateMinPromptPanelWidth(int numButtons) {
	return GUI_PADDING_X * (numButtons + 1) + PROMPT_BUTTON_WIDTH * numButtons;
}

void EditorWindow::closeConfirmationWindow() {
	removeChildWindow(confirmationWindow);
	confirmationWindowOpen = false;

	for (auto ptr : widgetsToBeEnabledAfterConfirmationPrompt) {
		ptr->setEnabled(true);
	}
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
}

void EditorWindow::updateConfirmationWindowLayout() {
	sf::Vector2f newSize = confirmationWindow->getClientSize();

	confirmationText->setMaximumTextWidth(confirmationWindow->getClientSize().x - GUI_PADDING_X * 2);
	confirmationInput->setSize(confirmationWindow->getClientSize().x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	if (confirmationButtonLayout == CONFIRMATION_WINDOW_BUTTON_LAYOUT::CANCEL_NO_YES) {
		confirmationCancel->setPosition(newSize.x - tgui::bindWidth(confirmationCancel) - GUI_PADDING_X, newSize.y - tgui::bindHeight(confirmationCancel) - GUI_PADDING_Y);
		confirmationNo->setPosition(tgui::bindLeft(confirmationCancel) - tgui::bindWidth(confirmationNo) - GUI_PADDING_X, tgui::bindTop(confirmationCancel));
		confirmationYes->setPosition(tgui::bindLeft(confirmationNo) - tgui::bindWidth(confirmationYes) - GUI_PADDING_X, tgui::bindTop(confirmationNo));
	} else if (confirmationButtonLayout == CONFIRMATION_WINDOW_BUTTON_LAYOUT::NO_YES) {
		confirmationNo->setPosition(newSize.x - tgui::bindWidth(confirmationNo) - GUI_PADDING_X, newSize.y - tgui::bindHeight(confirmationNo) - GUI_PADDING_Y);
		confirmationYes->setPosition(tgui::bindLeft(confirmationNo) - tgui::bindWidth(confirmationYes) - GUI_PADDING_X, tgui::bindTop(confirmationNo));
	} else if (confirmationButtonLayout == CONFIRMATION_WINDOW_BUTTON_LAYOUT::CANCEL_OK) {
		confirmationCancel->setPosition(newSize.x - tgui::bindWidth(confirmationCancel) - GUI_PADDING_X, newSize.y - tgui::bindHeight(confirmationCancel) - GUI_PADDING_Y);
		confirmationYes->setPosition(tgui::bindLeft(confirmationCancel) - tgui::bindWidth(confirmationYes) - GUI_PADDING_X, tgui::bindTop(confirmationCancel));
	} else if (confirmationButtonLayout == CONFIRMATION_WINDOW_BUTTON_LAYOUT::OK) {
		confirmationYes->setPosition(newSize.x - tgui::bindWidth(confirmationYes) - GUI_PADDING_X, newSize.y - tgui::bindHeight(confirmationYes) - GUI_PADDING_Y);
	}
}
