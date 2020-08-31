#include <Editor/CustomWidgets/MarkerPlacer.h>

#include <Mutex.h>
#include <GuiConfig.h>

// Index, x, and y in that order
const std::string MarkerPlacer::MARKERS_LIST_VIEW_ITEM_FORMAT = "%d (%.2f, %.2f)";
const sf::Color MarkerPlacer::GRID_COLOR = sf::Color(229, 229, 229);
const sf::Color MarkerPlacer::MAP_BORDER_COLOR = sf::Color(255, 0, 0);
const sf::Color MarkerPlacer::MAP_LINE_COLOR = sf::Color(143, 0, 0);
const float MarkerPlacer::MAX_GRID_SNAP_DISTANCE = 15.0f;
const float MarkerPlacer::MAX_GRID_SNAP_DISTANCE_SQUARED = MAX_GRID_SNAP_DISTANCE * MAX_GRID_SNAP_DISTANCE;

MarkerPlacer::MarkerPlacer(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution, int undoStackSize) 
	: CopyPasteable("Marker"), parentWindow(parentWindow), clipboard(clipboard), resolution(resolution), undoStack(UndoStack(undoStackSize)) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	gridLines.setPrimitiveType(sf::PrimitiveType::Lines);

	currentCursor = sf::CircleShape(-1);
	currentCursor.setOutlineColor(selectedMarkerColor);
	currentCursor.setOutlineThickness(outlineThickness);
	currentCursor.setFillColor(sf::Color::Transparent);

	viewController = std::make_unique<ViewController>(parentWindow);
	leftPanel = tgui::ScrollablePanel::create();
	markersListView = ListViewScrollablePanel::create();
	addMarker = tgui::Button::create();
	deleteMarker = tgui::Button::create();
	showGridLines = tgui::CheckBox::create();
	gridLinesIntervalLabel = tgui::Label::create();
	gridLinesInterval = SliderWithEditBox::create(false);
	selectedMarkerXLabel = tgui::Label::create();
	selectedMarkerYLabel = tgui::Label::create();
	selectedMarkerX = NumericalEditBoxWithLimits::create();
	selectedMarkerY = NumericalEditBoxWithLimits::create();
	std::shared_ptr<tgui::CheckBox> snapToGridCheckBox = tgui::CheckBox::create();
	mouseWorldPosPanel = tgui::Panel::create();
	mouseWorldPosLabel = tgui::Label::create();
	extraWidgetsPanel = tgui::ScrollablePanel::create();

	markersListView->setTextSize(TEXT_SIZE);
	addMarker->setTextSize(TEXT_SIZE);
	deleteMarker->setTextSize(TEXT_SIZE);
	showGridLines->setTextSize(TEXT_SIZE);
	gridLinesIntervalLabel->setTextSize(TEXT_SIZE);
	gridLinesInterval->setTextSize(TEXT_SIZE);
	selectedMarkerXLabel->setTextSize(TEXT_SIZE);
	selectedMarkerYLabel->setTextSize(TEXT_SIZE);
	selectedMarkerX->setTextSize(TEXT_SIZE);
	selectedMarkerY->setTextSize(TEXT_SIZE);
	snapToGridCheckBox->setTextSize(TEXT_SIZE);
	mouseWorldPosLabel->setTextSize(TEXT_SIZE);

	showGridLines->setText("Show grid lines");
	gridLinesIntervalLabel->setText("Distance between lines");
	selectedMarkerXLabel->setText("X");
	selectedMarkerYLabel->setText("Y");
	addMarker->setText("Add");
	deleteMarker->setText("Delete");
	snapToGridCheckBox->setText("Snap to grid");

	gridLinesInterval->setIntegerMode(true);
	gridLinesInterval->setMin(1);
	gridLinesInterval->setMax(std::max(MAP_WIDTH, MAP_HEIGHT) / 2.0f);
	gridLinesInterval->setValue(25);
	gridLinesInterval->setStep(1);

	markersListView->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	addMarker->setPosition(GUI_PADDING_X, tgui::bindBottom(markersListView) + GUI_PADDING_Y);
	deleteMarker->setPosition(tgui::bindRight(addMarker) + GUI_PADDING_X, tgui::bindTop(addMarker));
	selectedMarkerXLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(deleteMarker) + GUI_PADDING_Y);
	selectedMarkerX->setPosition(tgui::bindLeft(selectedMarkerXLabel), tgui::bindBottom(selectedMarkerXLabel) + GUI_LABEL_PADDING_Y);
	selectedMarkerYLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(selectedMarkerX) + GUI_PADDING_Y);
	selectedMarkerY->setPosition(tgui::bindLeft(selectedMarkerYLabel), tgui::bindBottom(selectedMarkerYLabel) + GUI_LABEL_PADDING_Y);

	mouseWorldPosPanel->setSize(tgui::bindWidth(mouseWorldPosLabel) + GUI_PADDING_X * 2, tgui::bindHeight(mouseWorldPosLabel) + GUI_LABEL_PADDING_Y * 2);
	mouseWorldPosPanel->setPosition(tgui::bindRight(leftPanel), tgui::bindBottom(leftPanel) - tgui::bindHeight(mouseWorldPosPanel));
	mouseWorldPosLabel->setPosition(GUI_PADDING_X, GUI_LABEL_PADDING_Y);

	showGridLines->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	snapToGridCheckBox->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);

	leftPanel->setSize(tgui::bindMin("25%", 250), "100%");
	extraWidgetsPanel->setPosition(tgui::bindRight(leftPanel), tgui::bindTop(leftPanel));

	leftPanel->connect("SizeChanged", [this](sf::Vector2f newSize) {
		// Height of left panel minus y needed for the widgets that are not the list view
		float markersListViewHeight = newSize.y - GUI_PADDING_Y * 5 - GUI_LABEL_PADDING_Y * 2 - selectedMarkerXLabel->getSize().y * 2 - selectedMarkerX->getSize().y * 2 - addMarker->getSize().y;
		markersListView->setSize(newSize.x - GUI_PADDING_X * 2, std::max(markersListViewHeight, markersListView->getListView()->getItemHeight() * 6.0f));
		float buttonWidth = (markersListView->getSize().x - GUI_PADDING_X) / 2.0f;
		addMarker->setSize(buttonWidth, TEXT_BUTTON_HEIGHT);
		deleteMarker->setSize(buttonWidth, TEXT_BUTTON_HEIGHT);
		selectedMarkerX->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		selectedMarkerY->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	});

	markersListView->getListView()->connect("ItemSelected", [this](int index) {
		if (ignoreSignals) {
			return;
		}
		if (index < 0) {
			deselectMarker();
		} else {
			selectMarker(index);
		}
	});
	markersListView->getListView()->connect("DoubleClicked", [this](int index) {
		if (ignoreSignals) {
			return;
		}
		if (index < 0) {
			deselectMarker();
		} else {
			// Invert y because lookAt() inverts y already
			lookAt(sf::Vector2f(markers[index].getPosition().x, -markers[index].getPosition().y));
		}
	});
	addMarker->connect("Pressed", [this]() {
		if (ignoreSignals) {
			return;
		}
		setPlacingNewMarker(true);
	});
	deleteMarker->connect("Pressed", [this]() {
		if (ignoreSignals) {
			return;
		}
		manualDelete();
	});
	showGridLines->connect("Changed", [this](bool checked) {
		if (ignoreSignals) {
			return;
		}

		setGridLinesVisible(checked);
	});
	gridLinesInterval->connect("ValueChanged", [this]() {
		calculateGridLines();
	});
	selectedMarkerX->connect("ValueChanged", [this](float value) {
		if (ignoreSignals) {
			return;
		}

		markers[selectedMarkerIndex].setPosition(value, markers[selectedMarkerIndex].getPosition().y);
		updateMarkersListViewItem(selectedMarkerIndex);
	});
	selectedMarkerY->connect("ValueChanged", [this](float value) {
		if (ignoreSignals) {
			return;
		}

		markers[selectedMarkerIndex].setPosition(markers[selectedMarkerIndex].getPosition().x, -value);
		updateMarkersListViewItem(selectedMarkerIndex);
	});
	snapToGridCheckBox->connect("Changed", [this](bool checked) {
		if (ignoreSignals) {
			return;
		}

		this->snapToGrid = checked;
	});

	leftPanel->add(markersListView);
	leftPanel->add(addMarker);
	leftPanel->add(deleteMarker);
	leftPanel->add(selectedMarkerXLabel);
	leftPanel->add(selectedMarkerYLabel);
	leftPanel->add(selectedMarkerX);
	leftPanel->add(selectedMarkerY);
	add(leftPanel);

	extraWidgetsPanel->connect("SizeChanged", [this](sf::Vector2f newSize) {
		gridLinesInterval->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	});
	addExtraRowWidget(showGridLines, GUI_LABEL_PADDING_Y);
	addExtraRowWidget(snapToGridCheckBox, GUI_LABEL_PADDING_Y);
	addExtraRowWidget(gridLinesIntervalLabel, GUI_LABEL_PADDING_Y);
	addExtraRowWidget(gridLinesInterval, GUI_LABEL_PADDING_Y);
	add(extraWidgetsPanel);

	mouseWorldPosPanel->add(mouseWorldPosLabel);
	add(mouseWorldPosPanel);

	connect("PositionChanged", [this]() {
		updateWindowView();
	});
	connect("SizeChanged", [this](sf::Vector2f newSize) {
		updateWindowView();
	});

	deselectMarker();

	leftPanel->setHorizontalScrollAmount(SCROLL_AMOUNT);
	leftPanel->setVerticalScrollAmount(SCROLL_AMOUNT);
	setGridLinesVisible(gridLinesVisible);
}

bool MarkerPlacer::handleEvent(sf::Event event) {
	if (event.type == sf::Event::MouseMoved) {
		sf::View originalView = parentWindow.getView();
		parentWindow.setView(viewFromViewController);
		sf::Vector2f mouseWorldPos = parentWindow.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
		parentWindow.setView(originalView);

		mouseWorldPosLabel->setText(format("(%.3f, %.3f)", mouseWorldPos.x, -mouseWorldPos.y));
	} else if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				undoStack.undo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				undoStack.redo();
				return true;
			} else if (event.key.code == sf::Keyboard::C) {
				clipboard.copy(this);
				return true;
			} else if (event.key.code == sf::Keyboard::V) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
					clipboard.paste2(this);
				} else {
					clipboard.paste(this);
				}
				return true;
			}
		} else if (event.key.code == sf::Keyboard::G) {
			// Toggle grid lines visibility
			setGridLinesVisible(!gridLinesVisible);
		} else if (event.key.code == sf::Keyboard::Delete) {
			manualDelete();
		}
	}

	if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && leftPanel->mouseOnWidget(sf::Vector2f(event.mouseButton.x, event.mouseButton.y) - getAbsolutePosition())) {
		return true;
	} else if (viewController->handleEvent(viewFromViewController, event)) {
		calculateGridLines();
		return true;
	}

	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Right) {
			setPlacingNewMarker(false);
			deselectMarker();
		} else if (event.mouseButton.button == sf::Mouse::Left) {
			sf::View originalView = parentWindow.getView();
			parentWindow.setView(viewFromViewController);
			sf::Vector2f mouseWorldPos = parentWindow.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
			parentWindow.setView(originalView);

			if (placingNewMarker) {
				setPlacingNewMarker(false);
				int newIndex;
				if (selectedMarkerIndex == -1) {
					newIndex = markers.size();
				} else {
					newIndex = selectedMarkerIndex;
				}
				undoStack.execute(UndoableCommand(
					[this, newIndex, mouseWorldPos]() {
					// Negate y because insertMarker() negates y already
					insertMarker(newIndex, sf::Vector2f(mouseWorldPos.x, -mouseWorldPos.y), addButtonMarkerColor);
				},
					[this, newIndex]() {
					removeMarker(newIndex);
				}));
			} else {
				// Check if user can begin dragging the selected marker first before every other marker
				if (selectedMarkerIndex != -1) {
					sf::CircleShape p = markers[selectedMarkerIndex];
					if (std::sqrt((mouseWorldPos.x - p.getPosition().x) * (mouseWorldPos.x - p.getPosition().x) + (mouseWorldPos.y - p.getPosition().y) * (mouseWorldPos.y - p.getPosition().y)) <= p.getRadius()) {
						draggingMarker = true;
						previousMarkerDragCoordsX = event.mouseButton.x;
						previousMarkerDragCoordsY = event.mouseButton.y;
						markerPosBeforeDragging = markers[selectedMarkerIndex].getPosition();
						return true;
					}
				}

				int i = 0;
				for (auto p : markers) {
					if (std::sqrt((mouseWorldPos.x - p.getPosition().x) * (mouseWorldPos.x - p.getPosition().x) + (mouseWorldPos.y - p.getPosition().y) * (mouseWorldPos.y - p.getPosition().y)) <= p.getRadius()) {
						if (selectedMarkerIndex == i) {
							draggingMarker = true;
							previousMarkerDragCoordsX = event.mouseButton.x;
							previousMarkerDragCoordsY = event.mouseButton.y;
							markerPosBeforeDragging = markers[selectedMarkerIndex].getPosition();
						} else {
							selectMarker(i);
							justSelectedMarker = true;
						}
						return true;
					}
					i++;
				}

				initialMousePressX = event.mouseButton.x;
				initialMousePressY = event.mouseButton.y;
			}
		}
	} else if (event.type == sf::Event::MouseMoved) {
		if (draggingMarker) {
			// Move selected placeholder depending on difference in world coordinates between event.mouseMove.x/y and previousPlaceholderDragCoordsX/Y
			sf::View originalView = parentWindow.getView();
			parentWindow.setView(viewFromViewController);

			sf::Vector2f newPos = parentWindow.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
			if (snapToGrid) {
				int interval = (int)std::round(gridLinesInterval->getValue());
				int xGridLine = roundToNearestMultiple((int)(std::round(newPos.x)), interval);
				int yGridLine = roundToNearestMultiple((int)(std::round(newPos.y)), interval);

				sf::Vector2i intersectionPosScreenCoords = parentWindow.mapCoordsToPixel(sf::Vector2f(xGridLine, yGridLine));
				float squaredScreenDistToNearestXGridLine = (intersectionPosScreenCoords.x - event.mouseMove.x) * (intersectionPosScreenCoords.x - event.mouseMove.x);
				float squaredScreenDistToNearestYGridLine = (intersectionPosScreenCoords.y - event.mouseMove.y) * (intersectionPosScreenCoords.y - event.mouseMove.y);
				float squaredScreenDistToNearestIntersection = squaredScreenDistToNearestXGridLine + squaredScreenDistToNearestYGridLine;

				// Attempt to snap to the intersection created by the 2 nearest perpendicular grid lines first
				if (squaredScreenDistToNearestIntersection <= MAX_GRID_SNAP_DISTANCE_SQUARED) {
					newPos.x = xGridLine;
					newPos.y = yGridLine;
				} else {
					// Attempt to snap to the nearest grid line if nearest intersection is too far

					if (squaredScreenDistToNearestXGridLine <= squaredScreenDistToNearestYGridLine && squaredScreenDistToNearestXGridLine <= MAX_GRID_SNAP_DISTANCE_SQUARED) {
						newPos.x = xGridLine;
					} else if (squaredScreenDistToNearestYGridLine <= MAX_GRID_SNAP_DISTANCE_SQUARED) {
						newPos.y = yGridLine;
					}
				}
			}

			parentWindow.setView(originalView);

			markers[selectedMarkerIndex].setPosition(newPos);
			setSelectedMarkerXWidgetValue(markers[selectedMarkerIndex].getPosition().x);
			setSelectedMarkerYWidgetValue(-markers[selectedMarkerIndex].getPosition().y);
			updateMarkersListViewItem(selectedMarkerIndex);
		}

		if (draggingMarker) {
			previousMarkerDragCoordsX = event.mouseMove.x;
			previousMarkerDragCoordsY = event.mouseMove.y;
		}
	} else if (event.type == sf::Event::MouseButtonReleased) {
		if (event.mouseButton.button == sf::Mouse::Left) {
			// Release event was from left mouse

			// Check if initial press was in gameplay area and in relative spatial proximity to mouse release
			float screenDist = std::sqrt((initialMousePressX - event.mouseButton.x) * (initialMousePressX - event.mouseButton.x) + (initialMousePressY - event.mouseButton.y) * (initialMousePressY - event.mouseButton.y));
			if (!justSelectedMarker && screenDist < 15 && !leftPanel->mouseOnWidget(sf::Vector2f(event.mouseButton.x, event.mouseButton.y))) {
				deselectMarker();
			}

			if (draggingMarker) {
				draggingMarker = false;

				if (selectedMarkerIndex != -1) {
					sf::Vector2f endPos = markers[selectedMarkerIndex].getPosition();
					sf::Vector2f prevPos = sf::Vector2f(markerPosBeforeDragging);
					undoStack.execute(UndoableCommand(
						[this, endPos]() {
						markers[selectedMarkerIndex].setPosition(endPos);
						setSelectedMarkerXWidgetValue(endPos.x);
						setSelectedMarkerYWidgetValue(-endPos.y);
						updateMarkersListViewItem(selectedMarkerIndex);
					},
						[this, prevPos]() {
						markers[selectedMarkerIndex].setPosition(prevPos);
						setSelectedMarkerXWidgetValue(prevPos.x);
						setSelectedMarkerYWidgetValue(-prevPos.y);
						updateMarkersListViewItem(selectedMarkerIndex);
					}));
				}
			}

			justSelectedMarker = false;
		}
	}

	return false;
}

void MarkerPlacer::selectMarker(int index) {
	deselectMarker();
	selectedMarkerIndex = index;

	ignoreSignals = true;

	deleteMarker->setEnabled(true);
	setSelectedMarkerXWidgetValue(markers[index].getPosition().x);
	setSelectedMarkerYWidgetValue(-markers[index].getPosition().y);
	markersListView->getListView()->setSelectedItem(index);

	selectedMarkerXLabel->setVisible(true);
	selectedMarkerX->setVisible(true);
	selectedMarkerYLabel->setVisible(true);
	selectedMarkerY->setVisible(true);

	ignoreSignals = false;
}

void MarkerPlacer::setPlacingNewMarker(bool placingNewMarker) {
	deselectMarker();
	this->placingNewMarker = placingNewMarker;
	if (placingNewMarker) {
		currentCursor.setRadius(circleRadius);
	} else {
		currentCursor.setRadius(-1);
	}
}

void MarkerPlacer::deselectMarker() {
	selectedMarkerIndex = -1;
	markersListView->getListView()->deselectItems();
	deleteMarker->setEnabled(false);
	selectedMarkerXLabel->setVisible(false);
	selectedMarkerX->setVisible(false);
	selectedMarkerYLabel->setVisible(false);
	selectedMarkerY->setVisible(false);
}

void MarkerPlacer::lookAt(sf::Vector2f pos) {
	viewFromViewController.setCenter(sf::Vector2f(pos.x, -pos.y));
	calculateGridLines();
}

void MarkerPlacer::clearUndoStack() {
	undoStack.clear();
}

void MarkerPlacer::setMarkers(std::vector<std::pair<sf::Vector2f, sf::Color>> markers) {
	this->markers.clear();
	for (auto p : markers) {
		auto shape = sf::CircleShape(circleRadius);
		shape.setFillColor(sf::Color::Transparent);
		shape.setOrigin(circleRadius, circleRadius);
		shape.setPosition(p.first.x, -p.first.y);
		shape.setOutlineColor(p.second);
		shape.setOutlineThickness(outlineThickness);
		this->markers.push_back(shape);
	}
	updateMarkersListView();
}

void MarkerPlacer::setMarker(int index, sf::Vector2f position, sf::Color color) {
	auto shape = sf::CircleShape(circleRadius);
	shape.setFillColor(sf::Color::Transparent);
	shape.setOrigin(circleRadius, circleRadius);
	shape.setPosition(position.x, -position.y);
	shape.setOutlineColor(color);
	shape.setOutlineThickness(outlineThickness);
	markers[index] = shape;
	updateMarkersListViewItem(index);
}

sf::CircleShape MarkerPlacer::getMarker(int index) {
	return markers[index];
}

void MarkerPlacer::insertMarker(int index, sf::Vector2f position, sf::Color color) {
	auto shape = sf::CircleShape(circleRadius);
	shape.setFillColor(sf::Color::Transparent);
	shape.setOrigin(circleRadius, circleRadius);
	shape.setPosition(position.x, -position.y);
	shape.setOutlineColor(color);
	shape.setOutlineThickness(outlineThickness);
	markers.insert(markers.begin() + index, shape);
	updateMarkersListView();
}

void MarkerPlacer::removeMarker(int index) {
	markers.erase(markers.begin() + index);
	if (markers.size() == 0) {
		deselectMarker();
	} else if (selectedMarkerIndex >= markers.size()) {
		selectedMarkerIndex = markers.size() - 1;
	}
	updateMarkersListView();
}

void MarkerPlacer::removeMarkers() {
	markers.clear();
	updateMarkersListView();
}

std::vector<sf::Vector2f> MarkerPlacer::getMarkerPositions() {
	auto res = std::vector<sf::Vector2f>();
	for (auto p : markers) {
		res.push_back(sf::Vector2f(p.getPosition().x, -p.getPosition().y));
	}
	return res;
}

void MarkerPlacer::setCircleRadius(float circleRadius) {
	this->circleRadius = circleRadius;
	for (auto marker : markers) {
		marker.setRadius(circleRadius);
		marker.setOrigin(circleRadius, circleRadius);
	}
	if (currentCursor.getRadius() > 0) {
		currentCursor.setRadius(circleRadius);
	}
	currentCursor.setOrigin(circleRadius, circleRadius);
}

void MarkerPlacer::setOutlineThickness(float outlineThickness) {
	this->outlineThickness = outlineThickness;
	for (auto marker : markers) {
		marker.setOutlineThickness(outlineThickness);
	}
	currentCursor.setOutlineThickness(outlineThickness);
}

void MarkerPlacer::setAddButtonMarkerColor(sf::Color color) {
	addButtonMarkerColor = color;
}

void MarkerPlacer::setSelectedMarkerXWidgetValue(float value) {
	selectedMarkerX->setValue(value);
}

void MarkerPlacer::setSelectedMarkerYWidgetValue(float value) {
	selectedMarkerY->setValue(value);
}

void MarkerPlacer::updateMarkersListView() {
	ignoreSignals = true;

	markersListView->getListView()->removeAllItems();
	for (int i = 0; i < markers.size(); i++) {
		markersListView->getListView()->addItem(format(MARKERS_LIST_VIEW_ITEM_FORMAT, i + 1, markers[i].getPosition().x, -markers[i].getPosition().y));
	}
	if (selectedMarkerIndex >= 0) {
		markersListView->getListView()->setSelectedItem(selectedMarkerIndex);
	}

	ignoreSignals = false;
}

void MarkerPlacer::calculateGridLines() {
	sf::Vector2f topLeftWorldCoords = viewFromViewController.getCenter() - viewFromViewController.getSize() / 2.0f;
	sf::Vector2f bottomRightWorldCoords = viewFromViewController.getCenter() + viewFromViewController.getSize() / 2.0f;

	int interval = (int)std::round(gridLinesInterval->getValue());
	int xStart = roundToNearestMultiple((int)topLeftWorldCoords.x, interval);
	int xEnd = roundToNearestMultiple((int)bottomRightWorldCoords.x, interval);
	int yStart = roundToNearestMultiple((int)topLeftWorldCoords.y, interval);
	int yEnd = roundToNearestMultiple((int)bottomRightWorldCoords.y, interval);
	gridLines.clear();
	// Vertical lines
	// Add some extra line padding by starting at (xStart - interval) and ending at (xEnd + interval) just in case
	for (int x = xStart - interval; x <= xEnd + interval; x += interval) {
		sf::Vertex v1(sf::Vector2f(x, yStart));
		v1.color = GRID_COLOR;
		sf::Vertex v2(sf::Vector2f(x, yEnd));
		v2.color = GRID_COLOR;
		gridLines.append(v1);
		gridLines.append(v2);
	}
	// Horizontal lines
	for (int y = yStart - interval; y <= yEnd + interval; y += interval) {
		sf::Vertex v1(sf::Vector2f(xStart, y));
		v1.color = GRID_COLOR;
		sf::Vertex v2(sf::Vector2f(xEnd, y));
		v2.color = GRID_COLOR;
		gridLines.append(v1);
		gridLines.append(v2);
	}

	// Add map lines; negative y values because screen coordinate system has (0, 0) at top-left of map
	sf::Vertex v1(sf::Vector2f(0, yStart));
	v1.color = MAP_LINE_COLOR;
	sf::Vertex v2(sf::Vector2f(0, yEnd));
	v2.color = MAP_LINE_COLOR;
	gridLines.append(v1);
	gridLines.append(v2);

	sf::Vertex v3(sf::Vector2f(MAP_WIDTH, yStart));
	v3.color = MAP_LINE_COLOR;
	sf::Vertex v4(sf::Vector2f(MAP_WIDTH, yEnd));
	v4.color = MAP_LINE_COLOR;
	gridLines.append(v3);
	gridLines.append(v4);

	sf::Vertex v5(sf::Vector2f(xStart, 0));
	v5.color = MAP_LINE_COLOR;
	sf::Vertex v6(sf::Vector2f(xEnd, 0));
	v6.color = MAP_LINE_COLOR;
	gridLines.append(v5);
	gridLines.append(v6);

	sf::Vertex v7(sf::Vector2f(xStart, -MAP_HEIGHT));
	v7.color = MAP_LINE_COLOR;
	sf::Vertex v8(sf::Vector2f(xEnd, -MAP_HEIGHT));
	v8.color = MAP_LINE_COLOR;
	gridLines.append(v7);
	gridLines.append(v8);

	// Add map border lines
	{
		sf::Vertex v1(sf::Vector2f(0, 0));
		v1.color = MAP_BORDER_COLOR;
		sf::Vertex v2(sf::Vector2f(0, -MAP_HEIGHT));
		v2.color = MAP_BORDER_COLOR;
		gridLines.append(v1);
		gridLines.append(v2);

		sf::Vertex v3(sf::Vector2f(0, 0));
		v3.color = MAP_BORDER_COLOR;
		sf::Vertex v4(sf::Vector2f(MAP_WIDTH, 0));
		v4.color = MAP_BORDER_COLOR;
		gridLines.append(v3);
		gridLines.append(v4);

		sf::Vertex v5(sf::Vector2f(0, -MAP_HEIGHT));
		v5.color = MAP_BORDER_COLOR;
		sf::Vertex v6(sf::Vector2f(MAP_WIDTH, -MAP_HEIGHT));
		v6.color = MAP_BORDER_COLOR;
		gridLines.append(v5);
		gridLines.append(v6);

		sf::Vertex v7(sf::Vector2f(MAP_WIDTH, 0));
		v7.color = MAP_BORDER_COLOR;
		sf::Vertex v8(sf::Vector2f(MAP_WIDTH, -MAP_HEIGHT));
		v8.color = MAP_BORDER_COLOR;
		gridLines.append(v7);
		gridLines.append(v8);
	}
}

void MarkerPlacer::updateWindowView() {
	auto windowSize = parentWindow.getSize();
	auto size = getSize();
	float sizeRatio = size.x / (float)size.y;
	float playAreaViewRatio = resolution.x / (float)resolution.y;

	float viewWidth, viewHeight;
	if (sizeRatio > playAreaViewRatio) {
		viewHeight = resolution.y;
		viewWidth = resolution.y * size.x / (float)size.y;
		float viewX = -(viewWidth - resolution.x) / 2.0f;
		float viewY = 0;
		viewFloatRect = sf::FloatRect(viewX, viewY, viewWidth, viewHeight);
	} else {
		viewWidth = resolution.x;
		viewHeight = resolution.x * size.y / (float)size.x;
		float viewX = 0;
		float viewY = -(viewHeight - resolution.y) / 2.0f;
		viewFloatRect = sf::FloatRect(viewX, viewY, viewWidth, viewHeight);
	}

	viewController->setOriginalViewSize(viewWidth, viewHeight);

	float viewportX = getAbsolutePosition().x / windowSize.x;
	float viewportY = getAbsolutePosition().y / windowSize.y;
	float viewportWidth = getSize().x / windowSize.x;
	float viewportHeight = getSize().y / windowSize.y;
	viewportFloatRect = sf::FloatRect(viewportX, viewportY, viewportWidth, viewportHeight);

	sf::Vector2f oldCenter = viewFromViewController.getCenter();
	viewController->setViewZone(viewFromViewController, viewFloatRect);
	viewFromViewController.setViewport(viewportFloatRect);
	viewFromViewController.setCenter(oldCenter);

	if (gridLinesVisible) {
		calculateGridLines();
	}
}

int MarkerPlacer::roundToNearestMultiple(int num, int multiple) {
	const auto ratio = static_cast<double>(num) / multiple;
	const auto iratio = std::lround(ratio);
	return iratio * multiple;
}

void MarkerPlacer::manualDelete() {
	sf::CircleShape markerToBeDeleted = markers[selectedMarkerIndex];
	int index = selectedMarkerIndex;
	undoStack.execute(UndoableCommand(
		[this, index]() {
		removeMarker(index);
	},
		[this, markerToBeDeleted, index]() {
		insertMarker(index, sf::Vector2f(markerToBeDeleted.getPosition().x, -markerToBeDeleted.getPosition().y), markerToBeDeleted.getOutlineColor());
	}));
}

void MarkerPlacer::updateMarkersListViewItem(int index) {
	markersListView->getListView()->changeItem(index, { format(MARKERS_LIST_VIEW_ITEM_FORMAT, index + 1, markers[index].getPosition().x, -markers[index].getPosition().y) });
}

void MarkerPlacer::setGridLinesVisible(bool gridLinesVisible) {
	this->gridLinesVisible = gridLinesVisible;

	ignoreSignals = true;
	showGridLines->setChecked(gridLinesVisible);
	ignoreSignals = false;

	if (gridLinesVisible) {
		calculateGridLines();
	}
}

void MarkerPlacer::addExtraRowWidget(std::shared_ptr<tgui::Widget> widget, float topPadding) {
	if (bottomLeftMostExtraWidget) {
		widget->setPosition(GUI_PADDING_X, tgui::bindBottom(bottomLeftMostExtraWidget) + topPadding);
	} else {
		widget->setPosition(GUI_PADDING_X, topPadding);
	}
	extraWidgetsPanel->setSize("100%" - tgui::bindWidth(leftPanel), tgui::bindBottom(widget) + GUI_PADDING_Y);
	extraWidgetsPanel->add(widget);
	bottomLeftMostExtraWidget = widget;
	bottomRightMostExtraWidget = widget;
}

void MarkerPlacer::addExtraColumnWidget(std::shared_ptr<tgui::Widget> widget, float leftPadding) {
	assert(bottomRightMostExtraWidget != nullptr);
	widget->setPosition(tgui::bindRight(bottomRightMostExtraWidget) + leftPadding, tgui::bindTop(bottomRightMostExtraWidget));
	extraWidgetsPanel->add(widget);
	bottomRightMostExtraWidget = widget;
}

std::pair<std::shared_ptr<CopiedObject>, std::string> MarkerPlacer::copyFrom() {
	if (selectedMarkerIndex > 0) {
		return std::make_pair(std::make_shared<CopiedMarker>(getID(), markers[selectedMarkerIndex]), "Copied 1 marker");
	}
	return std::make_pair(nullptr, "");
}

std::string MarkerPlacer::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	std::shared_ptr<CopiedMarker> derived = std::static_pointer_cast<CopiedMarker>(pastedObject);
	if (derived) {
		sf::CircleShape marker = derived->getMarker();
		int index = this->selectedMarkerIndex;
		undoStack.execute(UndoableCommand(
			[this, marker, index]() {
			// Negate y because insertMarker() negates y already
			if (index == -1) {
				insertMarker(markers.size(), sf::Vector2f(marker.getPosition().x, -marker.getPosition().y), addButtonMarkerColor);
			} else {
				insertMarker(index, sf::Vector2f(marker.getPosition().x, -marker.getPosition().y), addButtonMarkerColor);
			}
		},
			[this, marker, index]() {
			removeMarker(index);
		}));

		return "Pasted 1 marker";
	}
	return "";
}

std::string MarkerPlacer::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	std::shared_ptr<CopiedMarker> derived = std::static_pointer_cast<CopiedMarker>(pastedObject);
	if (derived && selectedMarkerIndex != -1) {
		sf::CircleShape marker = derived->getMarker();
		sf::CircleShape oldMarker = markers[selectedMarkerIndex];
		int index = this->selectedMarkerIndex;
		undoStack.execute(UndoableCommand(
			[this, marker, index]() {
			markers[index] = marker;
			updateMarkersListView();
		},
			[this, oldMarker, index]() {
			markers[index] = oldMarker;
			updateMarkersListView();
		}));

		return "Replaced 1 marker";
	}
	return "";
}

void MarkerPlacer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	tgui::Panel::draw(target, states);

	// Viewport is set here because tgui::Gui's draw function changes it right before renderSystem is updated or something
	sf::View originalView = parentWindow.getView();
	parentWindow.setView(viewFromViewController);
	// Draw grid
	if (gridLinesVisible) {
		parentWindow.draw(gridLines);
	}
	// Draw markers
	if (selectedMarkerIndex == -1) {
		for (int i = 0; i < markers.size(); i++) {
			parentWindow.draw(markers[i]);
		}
	} else {
		sf::CircleShape selectedShape = sf::CircleShape(markers[selectedMarkerIndex]);
		selectedShape.setOutlineColor(selectedMarkerColor);
		for (int i = 0; i < markers.size(); i++) {
			if (i != selectedMarkerIndex) {
				parentWindow.draw(markers[i]);
			}
		}
		// Draw selected placeholder on top
		parentWindow.draw(selectedShape);
	}
	if (currentCursor.getRadius() > 0) {
		auto pos = sf::Mouse::getPosition(parentWindow);
		sf::CircleShape currentCursor = sf::CircleShape(this->currentCursor);
		currentCursor.setPosition(parentWindow.mapPixelToCoords(pos) - sf::Vector2f(circleRadius, circleRadius));
		parentWindow.draw(currentCursor);
	}
	parentWindow.setView(originalView);

	// Draw panels again so that it covers the markers
	if (leftPanel->isVisible()) {
		leftPanel->draw(target, states);
	}
	if (extraWidgetsPanel->isVisible()) {
		extraWidgetsPanel->draw(target, states);
	}
	if (mouseWorldPosPanel->isVisible()) {
		mouseWorldPosPanel->draw(target, states);
	}
}

bool MarkerPlacer::update(sf::Time elapsedTime) {
	bool ret = tgui::Panel::update(elapsedTime);

	if (viewController->update(viewFromViewController, elapsedTime.asSeconds())) {
		calculateGridLines();
		ret = true;
	}
	return ret;
}