#pragma once
#include <TGUI/TGUI.hpp>

#include <Editor/CopyPaste.h>
#include <Editor/EventCapturable.h>
#include <Editor/CustomWidgets/NumericalEditBoxWithLimits.h>
#include <Editor/CustomWidgets/SliderWithEditBox.h>
#include <Editor/CustomWidgets/SimpleWidgetsContainerPanel.h>
#include <Editor/ViewController.h>
#include <DataStructs/UndoStack.h>

/*
A panel used to place markers on a 2D area. The ordering of the markers is maintained.
Extra widgets added inside this widget from classes derived from MarkerPlacer should do so using addExtraRowWidget().

Note for me: the markers are stored internally with a negative y position than what is inserted
to maintain a standard coordinate system. getMarkerPositions() will return the originally inputted
positions.
*/
class MarkerPlacer : public tgui::Panel, public CopyPasteable, public EventCapturable {
public:
	MarkerPlacer(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution = sf::Vector2u(MAP_WIDTH, MAP_HEIGHT), int undoStackSize = 50);
	static std::shared_ptr<MarkerPlacer> create(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution = sf::Vector2u(MAP_WIDTH, MAP_HEIGHT), int undoStackSize = 50) {
		return std::make_shared<MarkerPlacer>(parentWindow, clipboard, resolution, undoStackSize);
	}

	virtual CopyOperationResult copyFrom() override;
	virtual PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	virtual PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	virtual void draw(tgui::BackendRenderTargetBase& target, tgui::RenderStates states) const override;
	bool updateTime(tgui::Duration elapsedTime) override;
	bool handleEvent(sf::Event event) override;

	void lookAt(sf::Vector2f pos);
	void clearUndoStack();

	void setMarkers(std::vector<std::pair<sf::Vector2f, sf::Color>> markers);
	void setMarker(int index, sf::Vector2f position, sf::Color color);
	sf::CircleShape getMarker(int index);
	void insertMarker(int index, sf::Vector2f position, sf::Color color);
	void removeMarker(int index);
	void removeMarkers();
	virtual std::vector<sf::Vector2f> getMarkerPositions();

	void setCircleRadius(float circleRadius);
	void setOutlineThickness(float outlineThickness);
	/*
	Sets the color that future markers from the "Add" button will be.
	Default is red.
	*/
	void setAddButtonMarkerColor(sf::Color color);

protected:
	const static std::string MARKERS_LIST_VIEW_ITEM_FORMAT;
	sf::RenderWindow& parentWindow;
	Clipboard& clipboard;

	// The color of the selected marker
	sf::Color selectedMarkerColor = sf::Color::Green;
	// The color of future markers
	sf::Color addButtonMarkerColor = sf::Color::Red;

	// Index of the selected marker. -1 if nothing is selected.
	int selectedMarkerIndex = -1;

	// Whether the grid lines should be visible
	bool gridLinesVisible = true;
	// Whether to snap to grid
	bool snapToGrid = false;

	std::shared_ptr<tgui::Panel> mouseWorldPosPanel;

	std::vector<sf::CircleShape> markers;
	std::shared_ptr<tgui::ScrollablePanel> leftPanel;
	std::shared_ptr<tgui::ListView> markersListView;
	std::shared_ptr<NumericalEditBoxWithLimits> selectedMarkerX;
	std::shared_ptr<NumericalEditBoxWithLimits> selectedMarkerY;
	std::shared_ptr<tgui::Button> addMarker;
	std::shared_ptr<tgui::Button> deleteMarker;
	// Don't do connect("SizeChanged") with this in any derived classes or it'll break
	std::shared_ptr<SimpleWidgetsContainerPanel> extraWidgetsPanel;

	sf::View viewFromViewController;

	void selectMarker(int index);
	virtual void setSelectedMarkerXWidgetValue(float value);
	virtual void setSelectedMarkerYWidgetValue(float value);
	virtual void updateMarkersListView();
	virtual void updateMarkersListViewItem(int index);

	bool ignoreSignals = false;

	void setGridLinesVisible(bool showGridLines);

	virtual void manualDelete();

	/*
	Returns the top-left scren position where the world is drawn to.
	*/
	sf::Vector2f getWorldViewOffsetInPixels() const;

private:
	// Color of grid lines
	static const sf::Color GRID_COLOR;
	// Color of lines for the map border
	static const sf::Color MAP_BORDER_COLOR;
	// Color of lines for the map lines outside of the map border
	static const sf::Color MAP_LINE_COLOR;
	// Maximum screen distance before snapping to grid doesn't work anymore
	static const float MAX_GRID_SNAP_DISTANCE;
	// For faster calculations
	static const float MAX_GRID_SNAP_DISTANCE_SQUARED;

	const sf::Vector2u resolution;
	UndoStack undoStack;

	std::shared_ptr<tgui::Label> selectedMarkerXLabel;
	std::shared_ptr<tgui::Label> selectedMarkerYLabel;

	float circleRadius = 10.0f;
	float outlineThickness = 3.0f;

	sf::VertexArray gridLines;

	std::shared_ptr<tgui::CheckBox> showGridLines;
	std::shared_ptr<tgui::Label> gridLinesIntervalLabel;
	std::shared_ptr<SliderWithEditBox> gridLinesInterval;

	std::shared_ptr<tgui::Label> mouseWorldPosLabel;

	sf::FloatRect viewportFloatRect, viewFloatRect;

	std::unique_ptr<ViewController> viewController;

	// Radius will be < 0 if no cursor
	sf::CircleShape currentCursor;

	bool draggingMarker = false;
	// World pos of the placeholder being dragged at the moment it started being dragged
	sf::Vector2f markerPosBeforeDragging;

	// Set to true while left mouse is held down right after selecting a placeholder.
	// Used for preventing deselection from occurring in the same mouse press/release sequence that triggers selection.
	bool justSelectedMarker = false;

	// Screen coordinates of the mouse in the last MouseMove event while
	// draggingMarker was true
	int previousMarkerDragCoordsX, previousMarkerDragCoordsY;
	// Screen coordinates of the first left  mouse pressed event since the last left mouse released event
	int initialMousePressX, initialMousePressY;

	// Whether the user is currently placing a new marker. Should be modified only through setter.
	bool placingNewMarker = false;

	void setPlacingNewMarker(bool placingNewMarker);
	void deselectMarker();
	/*
	Updates gridLines according to the current view.
	*/
	void calculateGridLines();
	void updateWindowView();
	int roundToNearestMultiple(int num, int multiple);
};