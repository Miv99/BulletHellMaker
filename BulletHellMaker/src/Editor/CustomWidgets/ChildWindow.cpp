#include <Editor/CustomWidgets/ChildWindow.h>

#include <Cursors.h>

#include <iostream>

ChildWindow::ChildWindow() : tgui::ChildWindow() {
    setResizable(true);
}

bool ChildWindow::handleEvent(sf::Event event) {
    if (fallbackEventHandler) {
        return fallbackEventHandler(event);
    }

    return false;
}

void ChildWindow::setFallbackEventHandler(std::function<bool(sf::Event)> handler) {
    fallbackEventHandler = handler;
}
