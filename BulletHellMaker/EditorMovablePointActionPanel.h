#pragma once
#include "LevelPack.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"
#include "EditorMovablePointSpawnType.h"
#include "EventCapturable.h"
#include "EditorWindow.h"
#include "UndoStack.h"
#include "ExtraSignals.h"
#include "SymbolTableEditor.h"
#include <TGUI/TGUI.hpp>

/*
Panel used to edit an EMPAction.

Signals:
	EMPAModified - emitted when the EMPA being edited is modified.
		Optional parameter: a shared_ptr to the newly modified EMPAction
*/
class EditorMovablePointActionPanel : public tgui::ScrollablePanel, public EventCapturable, public ValueSymbolTablesChangePropagator {
public:
	/*
	empa - the EMPAction being edited
	*/
	EditorMovablePointActionPanel(EditorWindow& parentWindow, Clipboard& clipboard, std::shared_ptr<EMPAction> empa, int undoStackSize = 50);
	~EditorMovablePointActionPanel();
	inline static std::shared_ptr<EditorMovablePointActionPanel> create(EditorWindow& parentWindow, Clipboard& clipboard, std::shared_ptr<EMPAction> empa, int undoStackSize = 50) {
		return std::make_shared<EditorMovablePointActionPanel>(parentWindow, clipboard, empa, undoStackSize);
	}

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

	void propagateChangesToChildren() override;
	ValueSymbolTable getLevelPackObjectSymbolTable() override;

private:
	static const std::string BEZIER_CONTROL_POINT_FORMAT;

	EditorWindow& parentWindow;
	UndoStack undoStack;
	Clipboard& clipboard;
	std::shared_ptr<EMPAction> empa;

	std::shared_ptr<tgui::Label> empaiInfo;
	std::shared_ptr<tgui::Button> empaiChangeType;
	std::shared_ptr<tgui::ListBox> changeTypePopup;
	std::shared_ptr<tgui::Label> empaiDurationLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> empaiDuration;

	std::shared_ptr<tgui::Label> empaiXLabel;
	std::shared_ptr<EditBox> empaiX;
	std::shared_ptr<tgui::Label> empaiYLabel;
	std::shared_ptr<EditBox> empaiY;
	std::shared_ptr<tgui::Button> empaiXYManualSet;

	std::shared_ptr<tgui::Label> empaiPolarDistanceLabel;
	std::shared_ptr<TFVGroup> empaiPolarDistance;
	std::shared_ptr<tgui::Label> empaiPolarAngleLabel;
	std::shared_ptr<TFVGroup> empaiPolarAngle;
	std::shared_ptr<tgui::Label> empaiBezierControlPointsLabel;
	std::shared_ptr<ListViewScrollablePanel> empaiBezierControlPoints;
	std::shared_ptr<tgui::Label> empaiAngleOffsetLabel;
	std::shared_ptr<EMPAAngleOffsetGroup> empaiAngleOffset;
	std::shared_ptr<tgui::Label> empaiHomingStrengthLabel;
	std::shared_ptr<TFVGroup> empaiHomingStrength;
	std::shared_ptr<tgui::Label> empaiHomingSpeedLabel;
	std::shared_ptr<TFVGroup> empaiHomingSpeed;
	std::shared_ptr<tgui::Button> empaiEditBezierControlPoints;

	// Symbol table editor child window.
	// The window is added to the GUI directly and will be removed in this widget's destructor.
	std::shared_ptr<tgui::ChildWindow> symbolTableEditorWindow;
	// Symbol table editor
	std::shared_ptr<ValueSymbolTableEditor> symbolTableEditor;

	// For bezier control points
	bool editingBezierControlPoints = false;
	std::shared_ptr<BezierControlPointsPlacer> bezierControlPointsMarkerPlacer;
	std::shared_ptr<tgui::Button> bezierControlPointsMarkerPlacerFinishEditing;

	// For manually setting empaiX/Y
	bool placingXY = false;
	std::shared_ptr<SingleMarkerPlacer> xyPositionMarkerPlacer;
	std::shared_ptr<tgui::Button> xyPositionMarkerPlacerFinishEditing;
	
	// Data from this panel before empaiEditBezierControlPoints/xyMarkerPlacerFinishEditing was clicked
	std::vector<std::shared_ptr<tgui::Widget>> savedWidgets;
	float horizontalScrollPos;
	float verticalScrollPos;

	/*
	Signal emitted when the EMPA being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EMPA
	*/
	tgui::SignalEMPA onEMPAModify = { "EMPAModified" };

	bool ignoreSignals = false;

	// Should be called whenever empa's type is changed
	void onEMPATypeChange();
	void updateEmpaiBezierControlPoints();
	void finishEditingBezierControlPoints();
	void finishEditingXYPosition();
};