#include <cmath>
#include "EditorMovablePointAction.h"
#include "EditorMovablePointSpawnType.h"

std::string EMPAAngleOffsetToPlayer::format() {
	std::string res = "";
	res += "EMPAAngleOffsetToPlayer" + delim;
	res += "(" + tos(xOffset) + ")" + delim;
	res += "(" + tos(yOffset) + ")";
	return res;
}

void EMPAAngleOffsetToPlayer::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	xOffset = std::stof(items[1]);
	yOffset = std::stof(items[2]);
}

float EMPAAngleOffsetToPlayer::evaluate(const entt::DefaultRegistry & registry, float xFrom, float yFrom) {
	uint32_t player = registry.attachee<PlayerTag>();
	auto playerPos = registry.get<PositionComponent>(player);
	return std::atan2(playerPos.getY() + yOffset - yFrom, playerPos.getX() + xOffset - xFrom);
}

std::string EMPAAngleOffsetToGlobalPosition::format() {
	std::string res = "";
	res += "EMPAAngleOffsetToGlobalPosition" + delim;
	res += "(" + tos(x) + ")" + delim;
	res += "(" + tos(y) + ")";
	return res;
}

void EMPAAngleOffsetToGlobalPosition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = std::stof(items[1]);
	y = std::stof(items[2]);
}

float EMPAAngleOffsetToGlobalPosition::evaluate(const entt::DefaultRegistry & registry, float xFrom, float yFrom) {
	return std::atan2(y - yFrom, x - xFrom);
}

std::string EMPAAngleOffsetZero::format() {
	return "EMPAAngleOffsetZero";
}

void EMPAAngleOffsetZero::load(std::string formattedString) {
}


std::string EMPAngleOffsetPlayerSpriteAngle::format() {
	return "EMPAngleOffsetPlayerSpriteAngle";
}

void EMPAngleOffsetPlayerSpriteAngle::load(std::string formattedString) {
}

float EMPAngleOffsetPlayerSpriteAngle::evaluate(const entt::DefaultRegistry & registry, float xFrom, float yFrom) {
	return registry.get<SpriteComponent>(registry.attachee<PlayerTag>()).getInheritedRotationAngle();
}


std::string DetachFromParentEMPA::format() {
	return "DetachFromParentEMPA";
}

void DetachFromParentEMPA::load(std::string formattedString) {
}

std::shared_ptr<MovablePoint> DetachFromParentEMPA::execute(EntityCreationQueue& queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	auto& lastPos = registry.get<PositionComponent>(entity);

	// Since this EMPA is used only by bullets and bullets' DespawnComponents are only ever attached to other bullets or enemies,
	// when this bullet detaches from its parent, it should no longer despawn with the attached entity.
	registry.get<DespawnComponent>(entity).removeEntityAttachment(registry, entity);

	// Queue creation of the reference entity
	queue.pushBack(std::make_unique<EMPADetachFromParentCommand>(registry, entity, lastPos.getX(), lastPos.getY()));

	return std::make_shared<StationaryMP>(sf::Vector2f(lastPos.getX(), lastPos.getY()), 0);
}

std::string StayStillAtLastPositionEMPA::format() {
	return "StayStillAtLastPositionEMPA" + delim + tos(duration);
}

void StayStillAtLastPositionEMPA::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	duration = stof(items[1]);
}

std::shared_ptr<MovablePoint> StayStillAtLastPositionEMPA::execute(EntityCreationQueue& queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	auto& mpc = registry.get<MovementPathComponent>(entity);

	// Last known global position
	auto& lastPos = registry.get<PositionComponent>(entity);
	// Queue creation of the reference entity
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, entity, timeLag, lastPos.getX(), lastPos.getY()));

	return std::make_shared<StationaryMP>(sf::Vector2f(0, 0), duration);
}

std::string MoveCustomPolarEMPA::format() {
	return "MoveCustomPolarEMPA" + delim + "(" + distance->format() + ")" + delim + "(" + angle->format() + ")" + delim + "(" + tos(time) + ")";
}

void MoveCustomPolarEMPA::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	distance = TFVFactory::create(items[1]);
	angle = TFVFactory::create(items[2]);
	time = std::stof(items[3]);
}

std::shared_ptr<MovablePoint> MoveCustomPolarEMPA::execute(EntityCreationQueue& queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {		
	// Last known global position
	auto& lastPos = registry.get<PositionComponent>(entity);

	// Queue creation of the reference entity
	float referenceEntityAngleOffset = angle->evaluate(0);
	float referenceEntityDistanceOffset = distance->evaluate(0);
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, entity, timeLag, lastPos.getX() + referenceEntityDistanceOffset * std::cos(referenceEntityAngleOffset + PI), lastPos.getY() + referenceEntityDistanceOffset * std::sin(referenceEntityAngleOffset + PI)));
	
	if (angleOffset == nullptr) {
		return std::make_shared<PolarMP>(time, distance, angle);
	} else {
		// Create a new TFV with the offset added
		std::shared_ptr<TFV> angleWithOffset = std::make_shared<TranslationWrapperTFV>(angle, angleOffset->evaluate(registry, lastPos.getX(), lastPos.getY()));

		return std::make_shared<PolarMP>(time, distance, angleWithOffset);
	}
}

std::string MoveCustomBezierEMPA::format() {
	std::string ret = "MoveCustomBezierEMPA" + delim;
	ret += "(" + tos(time) + ")";
	for (auto p : controlPoints) {
		ret += delim + "(" + tos(p.x) + ")" + delim + "(" + tos(p.y) + ")";
	}
	return ret;
}

void MoveCustomBezierEMPA::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	time = std::stof(items[1]);
	for (int i = 2; i < items.size() + 2; i += 2) {
		controlPoints.push_back(sf::Vector2f(std::stof(items[i]), std::stof(items[i + 1])));
	}
}

std::shared_ptr<MovablePoint> MoveCustomBezierEMPA::execute(EntityCreationQueue & queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	assert(controlPoints[0] == sf::Vector2f(0, 0) && "Bezier curves must start at (0, 0)");

	auto& lastPos = registry.get<PositionComponent>(entity);
	// Queue creation of the reference entity
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, entity, timeLag, lastPos.getX(), lastPos.getY()));

	if (rotationAngle) {
		// Rotate all control points around (0, 0)
		float radians = rotationAngle->evaluate(registry, lastPos.getX(), lastPos.getY());
		float cos = std::cos(radians);
		float sin = std::sin(radians);
		// Skip the first control point since it's always going to be (0, 0)
		for (int i = 1; i < controlPoints.size(); i++) {
			controlPoints[i].x = unrotatedControlPoints[i].x * cos - unrotatedControlPoints[i].y * sin;
			controlPoints[i].y = unrotatedControlPoints[i].x * sin + unrotatedControlPoints[i].y * cos;
		}
		return std::make_shared<BezierMP>(time, controlPoints);
	} else {
		return std::make_shared<BezierMP>(time, controlPoints);
	}
}

std::string MovePlayerHomingEMPA::format() {
	return "MovePlayerHomingEMPA" + delim + "(" + homingStrength->format() + ")" + delim + "(" + speed->format() + ")" + delim + tos(time);
}

void MovePlayerHomingEMPA::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	homingStrength = TFVFactory::create(items[1]);
	speed = TFVFactory::create(items[2]);
	time = std::stof(items[3]);
}

std::shared_ptr<MovablePoint> MovePlayerHomingEMPA::execute(EntityCreationQueue & queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	// Queue creation of the reference entity
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, entity, timeLag, 0, 0));

	return std::make_shared<HomingMP>(time, speed, homingStrength, entity, registry.attachee<PlayerTag>(), registry);
}

std::shared_ptr<EMPAction> EMPActionFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<EMPAction> ptr;
	if (name == "DetachFromParentEMPA") {
		ptr = std::make_shared<DetachFromParentEMPA>();
	}
	else if (name == "StayStillAtLastPositionEMPA") {
		ptr = std::make_shared<StayStillAtLastPositionEMPA>();
	}
	else if (name == "MoveCustomPolarEMPA") {
		ptr = std::make_shared<MoveCustomPolarEMPA>();
	}
	else if (name == "MoveCustomBezierEMPA") {
		ptr = std::make_shared<MoveCustomBezierEMPA>();
	}
	else if (name == "MovePlayerHomingEMPA") {
		ptr = std::make_shared<MovePlayerHomingEMPA>();
	}
	ptr->load(formattedString);
	return ptr;
}

std::shared_ptr<EMPAAngleOffset> EMPAngleOffsetFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<EMPAAngleOffset> ptr;
	if (name == "EMPAAngleOffsetToPlayer") {
		ptr = std::make_shared<EMPAAngleOffsetToPlayer>();
	}
	else if (name == "EMPAAngleOffsetToGlobalPosition") {
		ptr = std::make_shared<EMPAAngleOffsetToGlobalPosition>();
	}
	else if (name == "EMPAAngleOffsetZero") {
		ptr = std::make_shared<EMPAAngleOffsetZero>();
	}
	else if (name == "EMPAngleOffsetPlayerSpriteAngle") {
		ptr = std::make_shared<EMPAngleOffsetPlayerSpriteAngle>();
	}
	ptr->load(formattedString);
	return ptr;
}