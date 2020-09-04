#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <entt/entt.hpp>

#include <Constants.h>

/*
If the underlying RenderWindow is closed, one only needs to call start() or startAndHide() again to reopen the RenderWindow.
*/
class EditorWindow : public std::enable_shared_from_this<EditorWindow> {
public:
	/*
	renderInterval - time between each render call. If the gui has a ListBox, renderInterval should be some relatively large number (~0.1) because tgui gets
		messed up with multithreading.
	*/
	EditorWindow(std::string windowTitle, int width, int height, bool scaleWidgetsOnResize = false, bool letterboxingEnabled = false, float renderInterval = RENDER_INTERVAL);

	/*
	Starts the main loop.
	This function blocks the current thread.
	*/
	void start();
	/*
	Starts the main loop and then hides the window.
	This function blocks the current thread.
	*/
	void startAndHide();
	/*
	Closes the window.
	*/
	void close();
	/*
	Pauses the EditorWindow and hides it. The EditorWindow object data is maintained.
	*/
	void hide();
	/*
	Shows the EditorWindow.
	*/
	void show();

	/*
	Disables all widgets and then prompts the user with a custom message to which the user can respond with either a yes or a no.
	A signal with one parameter is returned. The signal will be published only once, when the user responds. The bool parameter
	will be true if the user responds with yes, false if no. After the user response, all widgets' enabled/disabled statuses are
	set back to what they were before this function call.
	Each call to promptConfirmation() returns a new signal.

	widgetToFocusAfter - the widget that should be focused after the user answers. Set to nullptr to not focus anything afterwards.
	*/
	std::shared_ptr<entt::SigH<void(bool)>> promptConfirmation(std::string message, tgui::Widget* widgetToFocusAfter);
	/*
	Disables all widgets and then prompts the user with a custom message to which the user can respond with either a yes or a no.
	A signal with two parameters is returned. The signal will be published only once, when the user responds. The bool parameter
	will be true if the user responds with yes, false if no. The user object parameter will contain a copy of the userObject parameter.
	After the user response, all widgets' enabled/disabled statuses are set back to what they were before this function call.
	Each call to promptConfirmation() returns a new signal.

	widgetToFocusAfter - the widget that should be focused after the user answers. Set to nullptr to not focus anything afterwards.
	*/
	template<class T>
	std::shared_ptr<entt::SigH<void(bool, T)>> promptConfirmation(std::string message, T userObject, tgui::Widget* widgetToFocusAfter);

	/*
	Add a popup as a top-level widget in the Gui. If part of the popup 

	preferredX, preferredY - the preferred position of the popup; cannot be less than 0
	preferredWidth, preferredHeight - the preferred size of the popup; cannot be less than 0
	maxWidthFraction, maxHeightFraction - the maximum size of the popup as a multiplier of the window width/height
	*/
	void addPopupWidget(std::shared_ptr<tgui::Widget> popup, float preferredX, float preferredY, float preferredWidth, float preferredHeight, float maxWidthFraction = 1.0f, float maxHeightFraction = 1.0f);
	/*
	Add a popup as a child of the popupContainer.
	*/
	void addPopupWidget(std::shared_ptr<tgui::Container> popupContainer, std::shared_ptr<tgui::Widget> popup);
	void closePopupWidget();

	/*
	Returns the ID of the added vertex array.
	*/
	int addVertexArray(sf::VertexArray vertexArray);
	void removeVertexArray(int id);
	/*
	Sets the old VertexArray with the given id to be newVertexArray.
	If there was no VertexArray with the given id, nothing will happen.
	*/
	void modifyVertexArray(int id, sf::VertexArray newVertexArray);
	void removeAllVertexArrays();

	/*
	Called every time window size changes.
	*/
	virtual void updateWindowView(int windowWidth, int windowHeight);

	std::shared_ptr<tgui::Gui> getGui() { return gui; }
	inline int getWindowWidth() { return windowWidth; }
	inline int getWindowHeight() { return windowHeight; }
	std::shared_ptr<entt::SigH<void(float)>> getRenderSignal();
	std::shared_ptr<entt::SigH<void(int, int)>> getResizeSignal();
	std::shared_ptr<entt::SigH<void()>> getCloseSignal();
	inline std::shared_ptr<sf::RenderWindow> getWindow() { return window; }
	inline sf::Vector2f getMousePos() { return mousePos; }
	inline sf::Vector2f getLastMousePressPos() { return lastMousePressPos; }

protected:
	std::shared_ptr<sf::RenderWindow> window;
	std::shared_ptr<tgui::Gui> gui;

	// The last known position of the mouse
	sf::Vector2f mousePos = sf::Vector2f(0, 0);
	// The position of the mouse at the last mouse press
	sf::Vector2f lastMousePressPos = sf::Vector2f(0, 0);

	virtual void physicsUpdate(float deltaTime);
	virtual void render(float deltaTime);
	/*
	Returns whether the input was consumed.
	*/
	virtual bool handleEvent(sf::Event event);

	/*
	Called when the RenderWindow window is initialized.
	*/
	virtual void onRenderWindowInitialization();

private:
	int nextVertexArrayID = 0;
	std::map<int, sf::VertexArray> vertexArrays;

	std::string windowTitle;
	int windowWidth, windowHeight;

	bool windowCloseQueued = false;

	// The popup widget, if one exists.
	// Only one popup widget can exist at any time. If a new widget pops up,
	// the old one is removed from the Gui. When the user clicks outside the popup
	// widget, it closes.
	std::shared_ptr<tgui::Widget> popup;
	// The container that contains the popup
	std::shared_ptr<tgui::Container> popupContainer;

	// The widgets for the confirmation popup in promptConfirmation()
	std::shared_ptr<tgui::Panel> confirmationPanel; // Not added to gui until promptConfirmation() is called; removed after user responds
	std::shared_ptr<tgui::Label> confirmationText;
	std::shared_ptr<tgui::Button> confirmationYes;
	std::shared_ptr<tgui::Button> confirmationNo;

	bool confirmationPanelOpen = false;

	// A list of all Widgets in gui whose value of isEnabled() was true right before promptConfirmation() was called
	std::list<std::shared_ptr<tgui::Widget>> widgetsToBeEnabledAfterConfirmationPrompt;

	bool letterboxingEnabled;
	bool scaleWidgetsOnResize;
	
	float renderInterval;
	// Signal that's emitted every time a render call is made
	// function accepts 1 argument: the time since the last render
	std::shared_ptr<entt::SigH<void(float)>> renderSignal;
	// Signal that's emitted every time the window resizes
	std::shared_ptr<entt::SigH<void(int, int)>> resizeSignal;
	// Signal that's emitted right before the window closes
	std::shared_ptr<entt::SigH<void()>> closeSignal;

	/*
	Call to stop the confirmation prompt started by promptConfirmation().
	*/
	void closeConfirmationPanel();
};

template<class T>
inline std::shared_ptr<entt::SigH<void(bool, T)>> EditorWindow::promptConfirmation(std::string message, T userObject, tgui::Widget* widgetToFocusAfter) {
	// Don't allow 2 confirmation prompts at the same time
	if (confirmationPanelOpen) {
		return nullptr;
	}

	std::shared_ptr<entt::SigH<void(bool, T)>> confirmationSignal = std::make_shared<entt::SigH<void(bool, T)>>();

	// Disable all widgets
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
	for (std::shared_ptr<tgui::Widget> ptr : gui->getWidgets()) {
		if (ptr->isEnabled()) {
			widgetsToBeEnabledAfterConfirmationPrompt.push_back(ptr);
		}
		ptr->setEnabled(false);
	}
	
	confirmationYes->connect("Pressed", [this, confirmationSignal, userObject, widgetToFocusAfter]() {
		confirmationSignal->publish(true, userObject);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();

		if (widgetToFocusAfter) {
			widgetToFocusAfter->setFocused(true);
		}
	});
	confirmationNo->connect("Pressed", [this, confirmationSignal, userObject, widgetToFocusAfter]() {
		confirmationSignal->publish(false, userObject);
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