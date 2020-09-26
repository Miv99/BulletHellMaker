#pragma once
#include <TGUI/TGUI.hpp>

/*
A timeline whose elements are clickable buttons.

Signals:
ElementClicked - emitted when an element is clicked;
	Optional parameter: the index of the element as given in setElements() (int)
*/
class ClickableTimeline : public tgui::Group {
public:
	ClickableTimeline();
	static std::shared_ptr<ClickableTimeline> create() {
		return std::make_shared<ClickableTimeline>();
	}

	/*
	Sets the elements in this timeline. The duration of each element is assumed to be such that it ends right as
	the next element begins. The only exception is the last element, whose duration ends at maxTime.

	itemStartTimes - the start time and text of every element;
		this vector must be sorted in nondecreasing order by the start time and no element can overlap in time
	maxDuration - the maximum time of the timeline
	*/
	void setElements(std::vector<std::pair<float, std::string>> elementStartTimes, float maxTime);

	/*
	Sets the elements in this timeline. The duration of each element must be given.
	The timeline's max time is assumed to be the last element's start time added to its duration.

	elementStartTimesAndDuration - a vector of tuples representing the start time, duration, and text of each element, respectively;
		this vector must be sorted in nondecreasing order by the start time and no element can overlap in time
	*/
	void setElements(std::vector<std::tuple<float, float, std::string>> elementStartTimesAndDuration);

	/*
	Sets the elements in this timeline. The duration of each element must be given.

	elementStartTimesAndDuration - a vector of tuples representing the start time, duration, and text of each element, respectively;
		this vector must be sorted in nondecreasing order by the start time and no element can overlap in time
	maxTime - the maximum time that can be displayed in the timeline
	*/
	void setElements(std::vector<std::tuple<float, float, std::string>> elementStartTimesAndDuration, float maxTime);

	tgui::Signal& getSignal(tgui::String signalName) override;

private:
	tgui::SignalInt onElementPressed = { "ElementPressed" };

	// A vector of tuples containing start time, duration, and corresponding Button of each element, in that order
	// This vector is not guaranteed to be sorted in any way
	std::vector<std::tuple<float, float, std::shared_ptr<tgui::Button>>> elements;

	std::shared_ptr<tgui::ScrollablePanel> panel;
	std::shared_ptr<tgui::RangeSlider> cameraController;

	// How much to scale the x-coordinates and width of every button in the panel by
	float buttonScalar = 1.0f;
	// Another scalar that does the same thing; value comes from cameraController
	float buttonScalarScalar = 1.0f;
	// The maximum time value of the timeline
	float maxTimeValue = 1.0f;

	void updateButtonsPositionsAndSizes();
};