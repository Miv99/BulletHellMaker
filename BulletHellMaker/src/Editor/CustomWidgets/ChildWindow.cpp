#include <Editor/CustomWidgets/ChildWindow.h>

#include <Cursors.h>

#include <iostream>

ChildWindow::ChildWindow() : tgui::ChildWindow() {
    setResizable(true);
}

ChildWindow::~ChildWindow() {
    SetCursor(DEFAULT_CURSOR);
}

bool ChildWindow::handleEvent(sf::Event event) {
    // Check if the cursor should be changed to look like a resizing one
    if (isResizable() && event.type == sf::Event::MouseMoved) {
        sf::Vector2f absolutePos = getAbsolutePosition();
        sf::Vector2f size = getSize();
        bool nearTop = isNearTop(absolutePos, size, event.mouseMove.x, event.mouseMove.y);
        bool nearBottom = isNearBottom(absolutePos, size, event.mouseMove.x, event.mouseMove.y);
        bool nearLeft = isNearLeft(absolutePos, size, event.mouseMove.x, event.mouseMove.y);
        bool nearRight = isNearRight(absolutePos, size, event.mouseMove.x, event.mouseMove.y);

        if (nearTop) {
            if (nearLeft) {
                SetCursor(RESIZE_NWSE_CURSOR);
            } else if (nearRight) {
                SetCursor(RESIZE_NESW_CURSOR);
            } else {
                SetCursor(RESIZE_NS_CURSOR);
            }
        } else if (nearBottom) {
            if (nearLeft) {
                SetCursor(RESIZE_NESW_CURSOR);
            } else if (nearRight) {
                SetCursor(RESIZE_NWSE_CURSOR);
            } else {
                SetCursor(RESIZE_NS_CURSOR);
            }
        } else if (nearLeft || nearRight) {
            SetCursor(RESIZE_WE_CURSOR);
        } else {
            SetCursor(DEFAULT_CURSOR);
        }
    }

    if (fallbackEventHandler) {
        return fallbackEventHandler(event);
    }

    return false;
}

void ChildWindow::setFallbackEventHandler(std::function<bool(sf::Event)> handler) {
    fallbackEventHandler = handler;
}

bool ChildWindow::isNearTop(const sf::Vector2f& absolutePos, const sf::Vector2f& size, const int& mouseX, const int& mouseY) {
    return std::abs(absolutePos.y - mouseY) < START_RESIZE_MAX_DISTANCE;
}

bool ChildWindow::isNearBottom(const sf::Vector2f& absolutePos, const sf::Vector2f& size, const int& mouseX, const int& mouseY) {
    return std::abs(absolutePos.y + size.y + m_titleBarHeightCached - mouseY) < START_RESIZE_MAX_DISTANCE;
}

bool ChildWindow::isNearLeft(const sf::Vector2f& absolutePos, const sf::Vector2f& size, const int& mouseX, const int& mouseY) {
    return std::abs(absolutePos.x - mouseX) < START_RESIZE_MAX_DISTANCE;
}

bool ChildWindow::isNearRight(const sf::Vector2f& absolutePos, const sf::Vector2f& size, const int& mouseX, const int& mouseY) {
    return std::abs(absolutePos.x + size.x - mouseX) < START_RESIZE_MAX_DISTANCE;
}
