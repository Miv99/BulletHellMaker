#pragma once
#include <memory>

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>

#include <LevelPack/EditorMovablePointAction.h>

/*
Sends window to the foreground of the computer display.
*/
void sendToForeground(sf::RenderWindow& window);
/*
Returns a tooltip containing some text.
*/
std::shared_ptr<tgui::Label> createToolTip(std::string text);
/*
Returns a menu popup with clickable buttons. The height of the popup will always be the exact height needed to fit
all menu items without needing a scrollbar.

elements - a vector of pairs containing the text of the button and the function to be called when the button is pressed
*/
std::shared_ptr<tgui::ListBox> createMenuPopup(std::vector<std::pair<std::string, std::function<void()>>> elements);
/*
Returns a sf::VertexArray that contains the positions that an entity following the array of EMPAs will be in over time.
Will skip EMPAs that require the registry (Homing EMPAs).
The vertices linearly interpolate from startColor to endColor as the path progresses.

timeResolution - the amount of time between each point
x, y - the starting position of the returned vertex array
invertY - whether to negate all y values
*/
sf::VertexArray generateVertexArray(std::vector<std::shared_ptr<EMPAction>> actions, float timeResolution, float x, float y, bool invertY = false, sf::Color startColor = sf::Color::Red, sf::Color endColor = sf::Color::Blue);
/*
Returns a sf::VertexArray that contains the positions that an entity following the EMPA will be in over time.
Will not work if the EMPA requires the registry (Homing EMPAs).
The vertices linearly interpolate from startColor to endColor as the path progresses.

timeResolution - the amount of time between each point
x, y - the starting position of the returned vertex array
invertY - whether to negate all y values
*/
sf::VertexArray generateVertexArray(std::shared_ptr<EMPAction> action, float timeResolution, float x, float y, sf::Color startColor = sf::Color::Red, sf::Color endColor = sf::Color::Blue);
/*
Returns an array of segment data. Each segment data is 2 vectors of floats in order:
the x-coordinates and the y-coordinates of a matplotlibc curve.
The x-axis represents time in seconds and the y-axis the TFV's value.

timeResolution - the amount of time between each point
colors - the list of colors that will be looped to determine the color of each segment of the curve
*/
std::vector<std::pair<std::vector<float>, std::vector<float>>> generateMPPoints(std::shared_ptr<PiecewiseTFV> tfv, float tfvLifespan, float timeResolution);
/*
Returns whether the EMPA will be skipped by generateVertexArray().
*/
bool skippedByGenerateVertexArray(std::shared_ptr<EMPAction> empa);