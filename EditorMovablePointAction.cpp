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


std::string DetachFromParentEMPA::format() {
	return "DetachFromParentEMPA";
}

void DetachFromParentEMPA::load(std::string formattedString) {
}

std::shared_ptr<MovablePoint> DetachFromParentEMPA::execute(EntityCreationQueue& queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	auto& lastPos = registry.get<PositionComponent>(entity);

	// Queue creation of the reference entity
	queue.addCommand(std::make_unique<EMPADetachFromParentCommand>(registry, entity, lastPos.getX(), lastPos.getY()));

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

	// Last known position, relative to the entity's reference
	auto pos = mpc.getPath()->compute(sf::Vector2f(0, 0), mpc.getPath()->getLifespan());

	return std::make_shared<StationaryMP>(pos, duration);
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
	queue.addCommand(std::make_unique<CreateMovementRefereceEntityCommand>(registry, entity, timeLag, lastPos.getX(), lastPos.getY()));
	
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
	assert(registry.has<EnemyComponent>(entity) && "Bezier movement can only be done by enemies");
	assert(controlPoints[0] == sf::Vector2f(0, 0) && "Bezier curves must start at (0, 0)");
	// Since BezierMP can only be used by enemies, the creation of a new reference entity is not necessary because
	// enemies never have reference entities, and Bezier curves are in the standard basis and can easily be started at any
	// arbitrary point, unlike with polar coordinates.

	auto& lastPos = registry.get<PositionComponent>(entity);
	// Queue creation of the reference entity
	queue.addCommand(std::make_unique<CreateMovementRefereceEntityCommand>(registry, entity, timeLag, lastPos.getX(), lastPos.getY()));

	return std::make_shared<BezierMP>(time, controlPoints);
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
	ptr->load(formattedString);
	return ptr;
}