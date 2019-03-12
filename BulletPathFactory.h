#pragma once
#include "TimeFunctionVariable.h"
#include "MovablePoint.h"
#include "Components.h"
#include <memory>

class BulletPathFactory {
public:
	// auto calculate time so that bullet disappears after leaving map bounds
	std::shared_ptr<MovablePoint> linearInDirectionOf(PositionComponent from, PositionComponent to, float speed);

private:

};