#include <LevelPack/EditorMovablePointAction.h>

#include <cmath>

#include <LevelPack/EditorMovablePointSpawnType.h>

EMPAAngleOffset::EMPAAngleOffset() {
}

EMPAAngleOffsetToPlayer::EMPAAngleOffsetToPlayer(std::string xOffset, std::string yOffset) 
	: xOffset(xOffset), yOffset(yOffset) {
}

std::shared_ptr<LevelPackObject> EMPAAngleOffsetToPlayer::clone() const {
	return std::make_shared<EMPAAngleOffsetToPlayer>(xOffset, yOffset);
}

std::string EMPAAngleOffsetToPlayer::format() const {
	return formatString("EMPAAngleOffsetToPlayer") + formatString(xOffset) + formatString(yOffset) + formatTMObject(symbolTable);
}

void EMPAAngleOffsetToPlayer::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	xOffset = items.at(1);
	yOffset = items.at(2);
	symbolTable.load(items.at(3));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EMPAAngleOffsetToPlayer::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	
	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(xOffset, x-offset)
	LEGAL_CHECK_EXPRESSION(yOffset, y-offset)

	return std::make_pair(status, messages);
}

void EMPAAngleOffsetToPlayer::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(xOffset)
	COMPILE_EXPRESSION_FOR_FLOAT(yOffset)
}

float EMPAAngleOffsetToPlayer::evaluate(const entt::DefaultRegistry & registry, float xFrom, float yFrom) {
	uint32_t player = registry.attachee<PlayerTag>();
	auto playerPos = registry.get<PositionComponent>(player);
	return std::atan2(playerPos.getY() + yOffsetExprCompiledValue - yFrom, playerPos.getX() + xOffsetExprCompiledValue - xFrom);
}

float EMPAAngleOffsetToPlayer::evaluate(float xFrom, float yFrom, float playerX, float playerY) {
	return std::atan2(playerY + yOffsetExprCompiledValue - yFrom, playerX + xOffsetExprCompiledValue - xFrom);
}

bool EMPAAngleOffsetToPlayer::operator==(const EMPAAngleOffset& other) const {
	const EMPAAngleOffsetToPlayer& derived = dynamic_cast<const EMPAAngleOffsetToPlayer&>(other);
	return xOffset == derived.xOffset && yOffset == derived.yOffset;
}

EMPAAngleOffsetToGlobalPosition::EMPAAngleOffsetToGlobalPosition() {
}

EMPAAngleOffsetToGlobalPosition::EMPAAngleOffsetToGlobalPosition(std::string x, std::string y) 
	: x(x), y(y) {
}

std::shared_ptr<LevelPackObject> EMPAAngleOffsetToGlobalPosition::clone() const {
	return std::make_shared<EMPAAngleOffsetToGlobalPosition>(x, y);
}

std::string EMPAAngleOffsetToGlobalPosition::format() const {
	return formatString("EMPAAngleOffsetToGlobalPosition") + formatString(x) + formatString(y) + formatTMObject(symbolTable);
}

void EMPAAngleOffsetToGlobalPosition::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	x = items.at(1);
	y = items.at(2);
	symbolTable.load(items.at(3));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EMPAAngleOffsetToGlobalPosition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(x, x)
	LEGAL_CHECK_EXPRESSION(y, y)

	return std::make_pair(status, messages);
}

void EMPAAngleOffsetToGlobalPosition::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(x)
	COMPILE_EXPRESSION_FOR_FLOAT(y)
}

float EMPAAngleOffsetToGlobalPosition::evaluate(const entt::DefaultRegistry & registry, float xFrom, float yFrom) {
	return std::atan2(yExprCompiledValue - yFrom, xExprCompiledValue - xFrom);
}

float EMPAAngleOffsetToGlobalPosition::evaluate(float xFrom, float yFrom, float playerX, float playerY) {
	return std::atan2(yExprCompiledValue - yFrom, xExprCompiledValue - xFrom);
}

bool EMPAAngleOffsetToGlobalPosition::operator==(const EMPAAngleOffset& other) const {
	const EMPAAngleOffsetToGlobalPosition& derived = dynamic_cast<const EMPAAngleOffsetToGlobalPosition&>(other);
	return x == derived.x && y == derived.y;
}

EMPAAngleOffsetZero::EMPAAngleOffsetZero() {
}

std::shared_ptr<LevelPackObject> EMPAAngleOffsetZero::clone() const {
	return std::make_shared<EMPAAngleOffsetZero>();
}

std::string EMPAAngleOffsetZero::format() const {
	return formatString("EMPAAngleOffsetZero") + formatTMObject(symbolTable);
}

void EMPAAngleOffsetZero::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	symbolTable.load(items.at(1));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EMPAAngleOffsetZero::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	// Always legal
	return std::make_pair(LEGAL_STATUS::LEGAL, std::vector<std::string>());
}

void EMPAAngleOffsetZero::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	// Nothing to be done
}

bool EMPAAngleOffsetZero::operator==(const EMPAAngleOffset& other) const {
	return true;
}

EMPAAngleOffsetConstant::EMPAAngleOffsetConstant() {
}

EMPAAngleOffsetConstant::EMPAAngleOffsetConstant(std::string value)
	: value(value) {
}

std::shared_ptr<LevelPackObject> EMPAAngleOffsetConstant::clone() const {
	return std::make_shared<EMPAAngleOffsetConstant>(value);
}

std::string EMPAAngleOffsetConstant::format() const {
	return formatString("EMPAAngleOffsetConstant") + formatString(value) + formatTMObject(symbolTable);
}

void EMPAAngleOffsetConstant::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	value = items.at(1);
	symbolTable.load(items.at(2));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EMPAAngleOffsetConstant::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(value, value)

	return std::make_pair(status, messages);
}

void EMPAAngleOffsetConstant::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(value)
}

bool EMPAAngleOffsetConstant::operator==(const EMPAAngleOffset& other) const {
	const EMPAAngleOffsetConstant& derived = dynamic_cast<const EMPAAngleOffsetConstant&>(other);
	return value == derived.value;
}

EMPAngleOffsetPlayerSpriteAngle::EMPAngleOffsetPlayerSpriteAngle() {
}

std::shared_ptr<LevelPackObject> EMPAngleOffsetPlayerSpriteAngle::clone() const {
	return std::make_shared<EMPAngleOffsetPlayerSpriteAngle>();
}

std::string EMPAngleOffsetPlayerSpriteAngle::format() const {
	return formatString("EMPAngleOffsetPlayerSpriteAngle") + formatTMObject(symbolTable);
}

void EMPAngleOffsetPlayerSpriteAngle::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	symbolTable.load(items.at(1));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EMPAngleOffsetPlayerSpriteAngle::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	// Always legal
	return std::make_pair(LEGAL_STATUS::LEGAL, std::vector<std::string>());
}

void EMPAngleOffsetPlayerSpriteAngle::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	// Nothing to be done
}

float EMPAngleOffsetPlayerSpriteAngle::evaluate(const entt::DefaultRegistry & registry, float xFrom, float yFrom) {
	return registry.get<SpriteComponent>(registry.attachee<PlayerTag>()).getInheritedRotationAngle();
}

float EMPAngleOffsetPlayerSpriteAngle::evaluate(float xFrom, float yFrom, float playerX, float playerY) {
	// The player's sprite angle cannot be determined, so just return 0
	return 0;
}

bool EMPAngleOffsetPlayerSpriteAngle::operator==(const EMPAAngleOffset& other) const {
	return true;
}

DetachFromParentEMPA::DetachFromParentEMPA() {
}

std::shared_ptr<LevelPackObject> DetachFromParentEMPA::clone() const {
	std::shared_ptr<LevelPackObject> copy = std::make_shared<DetachFromParentEMPA>();
	copy->load(format());
	return copy;
}

std::string DetachFromParentEMPA::format() const {
	return formatString("DetachFromParentEMPA") + formatTMObject(symbolTable);
}

void DetachFromParentEMPA::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	symbolTable.load(items.at(1));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> DetachFromParentEMPA::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	// Always legal
	return std::make_pair(LEGAL_STATUS::LEGAL, std::vector<std::string>());
}

void DetachFromParentEMPA::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	// Nothing to be done
}

std::string DetachFromParentEMPA::getGuiFormat() {
	return "Detach";
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

std::shared_ptr<MovablePoint> DetachFromParentEMPA::generateStandaloneMP(float x, float y, float playerX, float playerY) {
	return std::make_shared<StationaryMP>(sf::Vector2f(x, y), 0);
}

bool DetachFromParentEMPA::operator==(const EMPAction& other) const {
	return true;
}

StayStillAtLastPositionEMPA::StayStillAtLastPositionEMPA() {
}

StayStillAtLastPositionEMPA::StayStillAtLastPositionEMPA(float duration) 
	: duration(duration) {
}

std::shared_ptr<LevelPackObject> StayStillAtLastPositionEMPA::clone() const {
	std::shared_ptr<LevelPackObject> copy = std::make_shared<StayStillAtLastPositionEMPA>();
	copy->load(format());
	return copy;
}

std::string StayStillAtLastPositionEMPA::format() const {
	return formatString("StayStillAtLastPositionEMPA") + tos(duration) + formatTMObject(symbolTable);
}

void StayStillAtLastPositionEMPA::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	duration = std::stof(items.at(1));
	symbolTable.load(items.at(2));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> StayStillAtLastPositionEMPA::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	if (duration < 0) {
		status = std::max(status, LEGAL_STATUS::WARNING);
		messages.push_back("Duration is negative.");
	}

	return std::make_pair(status, messages);
}

void StayStillAtLastPositionEMPA::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	// Nothing to be done
}

std::string StayStillAtLastPositionEMPA::getGuiFormat() {
	return "Stay still";
}

std::shared_ptr<MovablePoint> StayStillAtLastPositionEMPA::execute(EntityCreationQueue& queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	// Last known global position
	auto& lastPos = registry.get<PositionComponent>(entity);
	// Queue creation of the reference entity
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, entity, timeLag, lastPos.getX(), lastPos.getY()));

	return std::make_shared<StationaryMP>(sf::Vector2f(0, 0), duration);
}

std::shared_ptr<MovablePoint> StayStillAtLastPositionEMPA::generateStandaloneMP(float x, float y, float playerX, float playerY) {
	return std::make_shared<StationaryMP>(sf::Vector2f(x, y), duration);
}

bool StayStillAtLastPositionEMPA::operator==(const EMPAction& other) const {
	const StayStillAtLastPositionEMPA& derived = dynamic_cast<const StayStillAtLastPositionEMPA&>(other);
	return duration == derived.duration;
}

MoveCustomPolarEMPA::MoveCustomPolarEMPA() {
}

MoveCustomPolarEMPA::MoveCustomPolarEMPA(std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle, float time) 
	: distance(distance), angle(angle), time(time), angleOffset(std::make_shared<EMPAAngleOffsetZero>()) {
}

MoveCustomPolarEMPA::MoveCustomPolarEMPA(std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle, float time, std::shared_ptr<EMPAAngleOffset> angleOffset) 
	: distance(distance), angle(angle), time(time), angleOffset(angleOffset) {
}

std::shared_ptr<LevelPackObject> MoveCustomPolarEMPA::clone() const {
	std::shared_ptr<LevelPackObject> copy = std::make_shared<MoveCustomPolarEMPA>();
	copy->load(format());
	return copy;
}

std::string MoveCustomPolarEMPA::format() const {
	return formatString("MoveCustomPolarEMPA") + formatTMObject(*distance) + formatTMObject(*angle) + tos(time) + formatTMObject(*angleOffset) + formatTMObject(symbolTable);
}

void MoveCustomPolarEMPA::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	distance = TFVFactory::create(items.at(1));
	angle = TFVFactory::create(items.at(2));
	time = std::stof(items.at(3));
	angleOffset = EMPAngleOffsetFactory::create(items.at(4));
	symbolTable.load(items.at(5));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> MoveCustomPolarEMPA::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	
	if (time < 0) {
		status = std::max(status, LEGAL_STATUS::WARNING);
		messages.push_back("Time is negative.");
	}

	auto offsetLegal = angleOffset->legal(levelPack, spriteLoader, symbolTables);
	if (offsetLegal.first != LEGAL_STATUS::LEGAL) {
		status = std::max(status, offsetLegal.first);
		tabEveryLine(offsetLegal.second);
		messages.push_back("Angle offset:");
		messages.insert(messages.end(), offsetLegal.second.begin(), offsetLegal.second.end());
	}

	return std::make_pair(status, messages);
}

void MoveCustomPolarEMPA::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE

	angleOffset->compileExpressions(symbolTables);
}

std::string MoveCustomPolarEMPA::getGuiFormat() {
	return "Polar movement";
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

std::shared_ptr<MovablePoint> MoveCustomPolarEMPA::generateStandaloneMP(float x, float y, float playerX, float playerY) {
	if (angleOffset == nullptr) {
		return std::make_shared<PolarMP>(time, distance, angle);
	} else {
		// Create a new TFV with the offset added
		std::shared_ptr<TFV> angleWithOffset = std::make_shared<TranslationWrapperTFV>(angle, angleOffset->evaluate(x, y, playerX, playerY));

		return std::make_shared<PolarMP>(time, distance, angleWithOffset);
	}
}

bool MoveCustomPolarEMPA::operator==(const EMPAction& other) const {
	const MoveCustomPolarEMPA& derived = dynamic_cast<const MoveCustomPolarEMPA&>(other);
	return *distance == *derived.distance && *angle == *derived.angle && time == derived.time && ((!angleOffset && !derived.angleOffset) || *angleOffset == *derived.angleOffset);
}

MoveCustomBezierEMPA::MoveCustomBezierEMPA() {
}

MoveCustomBezierEMPA::MoveCustomBezierEMPA(std::vector<sf::Vector2f> unrotatedControlPoints, float time) 
	: time(time), rotationAngle(std::make_shared<EMPAAngleOffsetZero>()) {
	setUnrotatedControlPoints(unrotatedControlPoints);
}

MoveCustomBezierEMPA::MoveCustomBezierEMPA(std::vector<sf::Vector2f> unrotatedControlPoints, float time, std::shared_ptr<EMPAAngleOffset> rotationAngle) 
	: time(time), rotationAngle(rotationAngle) {
	setUnrotatedControlPoints(unrotatedControlPoints);
}

std::shared_ptr<LevelPackObject> MoveCustomBezierEMPA::clone() const {
	std::shared_ptr<LevelPackObject> copy = std::make_shared<MoveCustomBezierEMPA>();
	copy->load(format());
	return copy;
}

std::string MoveCustomBezierEMPA::format() const {
	std::string ret = formatString("MoveCustomBezierEMPA");
	ret += tos(time) + formatTMObject(*rotationAngle) + formatTMObject(symbolTable);
	for (auto p : unrotatedControlPoints) {
		ret += tos(p.x) + tos(p.y);
	}
	return ret;
}

void MoveCustomBezierEMPA::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	time = std::stof(items.at(1));
	rotationAngle = EMPAngleOffsetFactory::create(items.at(2));
	symbolTable.load(items.at(3));
	unrotatedControlPoints.clear();
	int i;
	for (i = 4; i < items.size(); i += 2) {
		unrotatedControlPoints.push_back(sf::Vector2f(std::stof(items.at(i)), std::stof(items.at(i + 1))));
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> MoveCustomBezierEMPA::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK

	if (time < 0) {
		status = std::max(status, LEGAL_STATUS::WARNING);
		messages.push_back("Time is negative.");
	}

	auto offsetLegal = rotationAngle->legal(levelPack, spriteLoader, symbolTables);
	if (offsetLegal.first != LEGAL_STATUS::LEGAL) {
		status = std::max(status, offsetLegal.first);
		tabEveryLine(offsetLegal.second);
		messages.push_back("Angle offset:");
		messages.insert(messages.end(), offsetLegal.second.begin(), offsetLegal.second.end());
	}

	return std::make_pair(status, messages);
}

void MoveCustomBezierEMPA::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE

	rotationAngle->compileExpressions(symbolTables);
}

std::string MoveCustomBezierEMPA::getGuiFormat() {
	return "Bezier movement";
}

std::shared_ptr<MovablePoint> MoveCustomBezierEMPA::execute(EntityCreationQueue & queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	assert(unrotatedControlPoints[0] == sf::Vector2f(0, 0) && "Bezier curves must start at (0, 0)");

	auto& lastPos = registry.get<PositionComponent>(entity);
	// Queue creation of the reference entity
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, entity, timeLag, lastPos.getX(), lastPos.getY()));

	if (rotationAngle) {
		// Rotate all control points around (0, 0)
		float radians = rotationAngle->evaluate(registry, lastPos.getX(), lastPos.getY());
		float cos = std::cos(radians);
		float sin = std::sin(radians);

		std::vector<sf::Vector2f> controlPoints;
		controlPoints.push_back(sf::Vector2f(0, 0));
		// Skip the first control point since it's always going to be (0, 0)
		for (int i = 1; i < unrotatedControlPoints.size(); i++) {
			controlPoints.push_back(sf::Vector2f(unrotatedControlPoints[i].x * cos - unrotatedControlPoints[i].y * sin,
				unrotatedControlPoints[i].x * sin + unrotatedControlPoints[i].y * cos));
		}

		return std::make_shared<BezierMP>(time, controlPoints);
	} else {
		return std::make_shared<BezierMP>(time, unrotatedControlPoints);
	}
}

std::shared_ptr<MovablePoint> MoveCustomBezierEMPA::generateStandaloneMP(float x, float y, float playerX, float playerY) {
	if (rotationAngle) {
		// Rotate all control points around (0, 0)
		float radians = rotationAngle->evaluate(x, y, playerX, playerY);
		float cos = std::cos(radians);
		float sin = std::sin(radians);

		std::vector<sf::Vector2f> controlPoints;
		controlPoints.push_back(sf::Vector2f(0, 0));
		// Skip the first control point since it's always going to be (0, 0)
		for (int i = 1; i < unrotatedControlPoints.size(); i++) {
			controlPoints.push_back(sf::Vector2f(unrotatedControlPoints[i].x * cos - unrotatedControlPoints[i].y * sin,
				unrotatedControlPoints[i].x * sin + unrotatedControlPoints[i].y * cos));
		}

		return std::make_shared<BezierMP>(time, controlPoints);
	} else {
		return std::make_shared<BezierMP>(time, unrotatedControlPoints);
	}
}

bool MoveCustomBezierEMPA::operator==(const EMPAction& other) const {
	const MoveCustomBezierEMPA& derived = dynamic_cast<const MoveCustomBezierEMPA&>(other);
	return time == derived.time && ((!rotationAngle && !derived.rotationAngle) || *rotationAngle == *derived.rotationAngle)
		&& unrotatedControlPoints.size() == derived.unrotatedControlPoints.size()
		&& std::equal(unrotatedControlPoints.begin(), unrotatedControlPoints.end(), derived.unrotatedControlPoints.begin());
}

MovePlayerHomingEMPA::MovePlayerHomingEMPA() {
}

MovePlayerHomingEMPA::MovePlayerHomingEMPA(std::shared_ptr<TFV> homingStrength, std::shared_ptr<TFV> speed, float time) 
	: homingStrength(homingStrength), speed(speed), time(time) {
}

std::shared_ptr<LevelPackObject> MovePlayerHomingEMPA::clone() const {
	std::shared_ptr<LevelPackObject> copy = std::make_shared<MovePlayerHomingEMPA>();
	copy->load(format());
	return copy;
}

std::string MovePlayerHomingEMPA::format() const {
	return formatString("MovePlayerHomingEMPA") + formatTMObject(*homingStrength) + formatTMObject(*speed) + tos(time) + formatTMObject(symbolTable);
}

void MovePlayerHomingEMPA::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	homingStrength = TFVFactory::create(items.at(1));
	speed = TFVFactory::create(items.at(2));
	time = std::stof(items.at(3));
	symbolTable.load(items.at(4));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> MovePlayerHomingEMPA::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK

	if (time < 0) {
		status = std::max(status, LEGAL_STATUS::WARNING);
		messages.push_back("Time is negative.");
	}

	return std::make_pair(status, messages);
}

void MovePlayerHomingEMPA::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	// Nothing to be done
}

std::string MovePlayerHomingEMPA::getGuiFormat() {
	return "Player-homing";
}

std::shared_ptr<MovablePoint> MovePlayerHomingEMPA::execute(EntityCreationQueue & queue, entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	// Queue creation of the reference entity
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, entity, timeLag, 0, 0));

	return std::make_shared<HomingMP>(time, speed, homingStrength, entity, registry.attachee<PlayerTag>(), registry);
}

std::shared_ptr<MovablePoint> MovePlayerHomingEMPA::generateStandaloneMP(float x, float y, float playerX, float playerY) {
	return std::make_shared<HomingMP>(time, speed, homingStrength, x, y, playerX, playerY);
}

bool MovePlayerHomingEMPA::operator==(const EMPAction& other) const {
	const MovePlayerHomingEMPA& derived = dynamic_cast<const MovePlayerHomingEMPA&>(other);
	return *homingStrength == *derived.homingStrength && *speed == *derived.speed && time == derived.time;
}

MoveGlobalHomingEMPA::MoveGlobalHomingEMPA() {
}

MoveGlobalHomingEMPA::MoveGlobalHomingEMPA(std::shared_ptr<TFV> homingStrength, std::shared_ptr<TFV> speed, std::string targetX, std::string targetY, float time)
	: homingStrength(homingStrength), speed(speed), targetX(targetX), targetY(targetY), time(time) {
}

std::shared_ptr<LevelPackObject> MoveGlobalHomingEMPA::clone() const {
	std::shared_ptr<LevelPackObject> copy = std::make_shared<MoveGlobalHomingEMPA>();
	copy->load(format());
	return copy;
}

std::string MoveGlobalHomingEMPA::format() const {
	return formatString("MoveGlobalHomingEMPA") + formatString(targetX) + formatString(targetY) + formatTMObject(*homingStrength) + formatTMObject(*speed) + tos(time) + formatTMObject(symbolTable);
}

void MoveGlobalHomingEMPA::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	targetX = items.at(1);
	targetY = items.at(2);
	homingStrength = TFVFactory::create(items.at(3));
	speed = TFVFactory::create(items.at(4));
	time = std::stof(items.at(5));
	symbolTable.load(items.at(6));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> MoveGlobalHomingEMPA::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(targetX, x-target)
	LEGAL_CHECK_EXPRESSION(targetY, y-target)

	return std::make_pair(status, messages);
}

void MoveGlobalHomingEMPA::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(targetX)
	COMPILE_EXPRESSION_FOR_FLOAT(targetY)
}

std::string MoveGlobalHomingEMPA::getGuiFormat() {
	return "Map-homing";
}

std::shared_ptr<MovablePoint> MoveGlobalHomingEMPA::execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) {
	// Queue creation of the reference entity
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, entity, timeLag, 0, 0));
	const PositionComponent& curPos = registry.get<PositionComponent>(entity);

	return std::make_shared<HomingMP>(time, speed, homingStrength, entity, targetXExprCompiledValue, targetYExprCompiledValue, registry);
}

std::shared_ptr<MovablePoint> MoveGlobalHomingEMPA::generateStandaloneMP(float x, float y, float targetX, float targetY) {
	return std::make_shared<HomingMP>(time, speed, homingStrength, x, y, targetX, targetY);
}

bool MoveGlobalHomingEMPA::operator==(const EMPAction& other) const {
	const MoveGlobalHomingEMPA& derived = dynamic_cast<const MoveGlobalHomingEMPA&>(other);
	return *homingStrength == *derived.homingStrength && *speed == *derived.speed && time == derived.time && targetX == derived.targetX && targetY == derived.targetY;
}

std::shared_ptr<EMPAction> EMPActionFactory::create(std::string formattedString) {
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
	std::shared_ptr<EMPAction> ptr;
	if (name == "DetachFromParentEMPA") {
		ptr = std::make_shared<DetachFromParentEMPA>();
	} else if (name == "StayStillAtLastPositionEMPA") {
		ptr = std::make_shared<StayStillAtLastPositionEMPA>();
	} else if (name == "MoveCustomPolarEMPA") {
		ptr = std::make_shared<MoveCustomPolarEMPA>();
	} else if (name == "MoveCustomBezierEMPA") {
		ptr = std::make_shared<MoveCustomBezierEMPA>();
	} else if (name == "MovePlayerHomingEMPA") {
		ptr = std::make_shared<MovePlayerHomingEMPA>();
	} else if (name == "MoveGlobalHomingEMPA") {
		ptr = std::make_shared<MoveGlobalHomingEMPA>();
	}
	ptr->load(formattedString);
	return ptr;
}

std::shared_ptr<EMPAAngleOffset> EMPAngleOffsetFactory::create(std::string formattedString) {
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
	std::shared_ptr<EMPAAngleOffset> ptr;
	if (name == "EMPAAngleOffsetToPlayer") {
		ptr = std::make_shared<EMPAAngleOffsetToPlayer>();
	} else if (name == "EMPAAngleOffsetToGlobalPosition") {
		ptr = std::make_shared<EMPAAngleOffsetToGlobalPosition>();
	} else if (name == "EMPAAngleOffsetZero") {
		ptr = std::make_shared<EMPAAngleOffsetZero>();
	} else if (name == "EMPAngleOffsetPlayerSpriteAngle") {
		ptr = std::make_shared<EMPAngleOffsetPlayerSpriteAngle>();
	} else if (name == "EMPAAngleOffsetConstant") {
		ptr = std::make_shared<EMPAAngleOffsetConstant>();
	}
	ptr->load(formattedString);
	return ptr;
}