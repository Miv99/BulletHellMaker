#pragma once
#include <Editor/CustomWidgets/MarkerPlacer.h>

/*
A general visualizer for a list of EMPAs.
*/
class EMPAListVisualizer : public MarkerPlacer {
public:
	EMPAListVisualizer(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution = sf::Vector2u(MAP_WIDTH, MAP_HEIGHT), int undoStackSize = 50);
	static std::shared_ptr<EMPAListVisualizer> create(sf::RenderWindow& parentWindow, Clipboard& clipboard, sf::Vector2u resolution = sf::Vector2u(MAP_WIDTH, MAP_HEIGHT), int undoStackSize = 50) {
		return std::make_shared<EMPAListVisualizer>(parentWindow, clipboard, resolution, undoStackSize);
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	/*
	Will not modify empas.
	*/
	void updatePath(std::vector<std::shared_ptr<EMPAction>> empas);
	void cycleMovementPathPrimitiveType();

	/*
	Sets the path of an EMPA in the list of EMPAs given in updatePath() to have a special color.
	Only one EMPA can have a special color at a time.
	Set empaIndex to -1 to have no EMPA have a special color.
	*/
	void setEMPAPathColor(int empaIndex, sf::Color color);
	/*
	Set starting position of the first EMPA.
	*/
	void setStartPosX(float x);
	/*
	Set starting position of the first EMPA.
	*/
	void setStartPosY(float y);

private:
	const static float EVALUATOR_CIRCLE_RADIUS;

	sf::VertexArray movementPath;
	sf::VertexArray movementPathWithoutSpecialColor;
	float movementPathTime;
	sf::PrimitiveType movementPathPrimitiveType = sf::PrimitiveType::LineStrip;
	// Shows position on movement path of the evaluator; radius <= 0 if invisible
	sf::CircleShape evaluatorCircle;

	std::shared_ptr<SliderWithEditBox> timeResolution;
	std::shared_ptr<SliderWithEditBox> evaluator;
	std::shared_ptr<tgui::Label> evaluatorResult;

	std::vector<std::shared_ptr<EMPAction>> empas;
	// The ending position of every EMPA in empas (including the movement from the previous EMPA)
	std::vector<sf::Vector2f> empaEndingPos;
	// The time at which every EMPA in empas becomes active
	std::vector<float> empaActiveTime;
	int specialColorIndex = -1;
	sf::Color specialColor;

	float startX = 0, startY = 0;

	/*
	Updates
	*/
	void updateEvaluatorResult();
};