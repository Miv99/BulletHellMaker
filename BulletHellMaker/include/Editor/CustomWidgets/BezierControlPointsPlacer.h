#pragma once
#include <Editor/CustomWidgets/MarkerPlacer.h>

/*
A MarkerPlacer used to edit the control points of a bezier curve.
*/
class BezierControlPointsPlacer : public MarkerPlacer {
public:
	BezierControlPointsPlacer(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution = sf::Vector2u(MAP_WIDTH, MAP_HEIGHT), int undoStackSize = 50);
	static std::shared_ptr<BezierControlPointsPlacer> create(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution = sf::Vector2u(MAP_WIDTH, MAP_HEIGHT), int undoStackSize = 50) {
		return std::make_shared<BezierControlPointsPlacer>(parentWindow, clipboard, resolution, undoStackSize);
	}

	virtual std::string pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	virtual std::string paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	void cycleMovementPathPrimitiveType();

	/*
	Sets the duration of the bezier movement.
	*/
	void setMovementDuration(float time);

	/*
	The first marker returned by this function is always at (0, 0) and
	every other marker's position is relative to the first marker.
	*/
	std::vector<sf::Vector2f> getMarkerPositions() override;

private:
	const static float EVALUATOR_CIRCLE_RADIUS;

	sf::VertexArray movementPath;
	float movementPathTime;
	sf::PrimitiveType movementPathPrimitiveType = sf::PrimitiveType::LineStrip;
	// Shows position on movement path of the evaluator; radius <= 0 if invisible
	sf::CircleShape evaluatorCircle;

	std::shared_ptr<SliderWithEditBox> timeResolution;
	std::shared_ptr<SliderWithEditBox> evaluator;
	std::shared_ptr<tgui::Label> evaluatorResult;

	/*
	Update the movement path to reflect changes in the markers.
	*/
	void updatePath();
	void updateEvaluatorResult();

	void setSelectedMarkerXWidgetValue(float value) override;
	void setSelectedMarkerYWidgetValue(float value) override;
	void updateMarkersListView() override;
	void updateMarkersListViewItem(int index) override;
};