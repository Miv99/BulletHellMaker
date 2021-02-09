#include <LevelPack/DeathAction.h>

#include <Game/Components/EnemyComponent.h>
#include <Game/Components/PositionComponent.h>
#include <LevelPack/LevelPack.h>
#include <LevelPack/Attack.h>
#include <LevelPack/Enemy.h>
#include <Game/EntityCreationQueue.h>

PlayAnimatableDeathAction::PlayAnimatableDeathAction() {
}

PlayAnimatableDeathAction::PlayAnimatableDeathAction(Animatable animatable, DEATH_ANIMATION_EFFECT effect, std::string duration) 
	: animatable(animatable), effect(effect), duration(duration) {
}

std::shared_ptr<LevelPackObject> PlayAnimatableDeathAction::clone() const {
	auto clone = std::make_shared<PlayAnimatableDeathAction>();
	clone->load(format());
	return clone;
}

std::string PlayAnimatableDeathAction::format() const {
	return formatString("PlayAnimatableDeathAction") + formatTMObject(animatable) + formatString(duration) + tos(static_cast<int>(effect)) + formatTMObject(symbolTable);
}

void PlayAnimatableDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	animatable.load(items.at(1));
	duration = items.at(2);
	effect = static_cast<DEATH_ANIMATION_EFFECT>(std::stoi(items.at(3)));
	symbolTable.load(items.at(4));
}

nlohmann::json PlayAnimatableDeathAction::toJson() {
	return {
		{"className", "PlayAnimatableDeathAction"},
		{"valueSymbolTable", symbolTable.toJson()},
		{"animatable", animatable.toJson()},
		{"effect", effect},
		{"duration", duration}
	};
}

void PlayAnimatableDeathAction::load(const nlohmann::json& j) {
	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	if (j.contains("animatable")) {
		animatable.load(j.at("animatable"));
	} else {
		animatable = Animatable();
	}

	j.at("effect").get_to(effect);
	j.at("duration").get_to(duration);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> PlayAnimatableDeathAction::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(duration, duration)

	// TODO: legal check animatable
	return std::make_pair(status, messages);
}

void PlayAnimatableDeathAction::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(duration)
}

void PlayAnimatableDeathAction::execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	queue.pushBack(std::make_unique<PlayDeathAnimatableCommand>(registry, spriteLoader, entity, animatable, effect, durationExprCompiledValue));
}

Animatable PlayAnimatableDeathAction::getAnimatable() {
	return animatable;
}

PlayAnimatableDeathAction::DEATH_ANIMATION_EFFECT PlayAnimatableDeathAction::getEffect() {
	return effect;
}

float PlayAnimatableDeathAction::getDuration() {
	return durationExprCompiledValue;
}

void PlayAnimatableDeathAction::setAnimatable(Animatable animatable) {
	this->animatable = animatable;
}

void PlayAnimatableDeathAction::setDuration(float duration) {
	this->duration = duration;
}

void PlayAnimatableDeathAction::setEffect(DEATH_ANIMATION_EFFECT effect) {
	this->effect = effect;
}

PlaySoundDeathAction::PlaySoundDeathAction() {
}

PlaySoundDeathAction::PlaySoundDeathAction(SoundSettings soundSettings) 
	: soundSettings(soundSettings) {
}

std::shared_ptr<LevelPackObject> PlaySoundDeathAction::clone() const {
	auto clone = std::make_shared<PlaySoundDeathAction>();
	clone->load(format());
	return clone;
}

std::string PlaySoundDeathAction::format() const {
	return formatString("PlaySoundDeathAction") + formatTMObject(soundSettings);
}

void PlaySoundDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	soundSettings.load(items.at(1));
}

nlohmann::json PlaySoundDeathAction::toJson() {
	return {
		{"className", "PlaySoundDeathAction"},
		{"soundSettings", soundSettings.toJson()},
	};
}

void PlaySoundDeathAction::load(const nlohmann::json& j) {
	if (j.contains("soundSettings")) {
		soundSettings.load(j.at("soundSettings"));
	} else {
		soundSettings = SoundSettings();
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> PlaySoundDeathAction::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	// TODO: legal check sound settings
	return std::make_pair(status, messages);
}

void PlaySoundDeathAction::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	// Nothing to be done
}

void PlaySoundDeathAction::execute(LevelPack& levelPack, EntityCreationQueue & queue, entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	levelPack.playSound(soundSettings);
}

SoundSettings PlaySoundDeathAction::getSoundSettings() {
	return soundSettings;
}

void PlaySoundDeathAction::setSoundSettings(SoundSettings soundSettings) {
	this->soundSettings = soundSettings;
}

ExecuteAttacksDeathAction::ExecuteAttacksDeathAction() {
}

ExecuteAttacksDeathAction::ExecuteAttacksDeathAction(std::vector<std::pair<int, ExprSymbolTable>> attackIDs) 
	: attackIDs(attackIDs) {
}

std::shared_ptr<LevelPackObject> ExecuteAttacksDeathAction::clone() const {
	auto clone = std::make_shared<ExecuteAttacksDeathAction>();
	clone->load(format());
	return clone;
}

std::string ExecuteAttacksDeathAction::format() const {
	std::string ret = formatString("ExecuteAttacksDeathAction") + formatTMObject(symbolTable) + tos(attackIDs.size());
	for (auto p : attackIDs) {
		ret += tos(p.first) + formatTMObject(p.second);
	}
	return ret;
}

void ExecuteAttacksDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);

	symbolTable.load(items.at(1));

	attackIDs.clear();
	for (int i = 3; i < std::stoi(items.at(2)) + 3; i += 2) {
		ExprSymbolTable definer;
		definer.load(items.at(i + 1));
		attackIDs.push_back(std::make_pair(std::stoi(items.at(i)), definer));
	}
}

nlohmann::json ExecuteAttacksDeathAction::toJson() {
	nlohmann::json j = {
		{"className", "ExecuteAttacksDeathAction"},
		{"valueSymbolTable", symbolTable.toJson()}
	};

	nlohmann::json attackIDsJson;
	for (std::pair<int, ExprSymbolTable> p : attackIDs) {
		attackIDsJson.push_back(nlohmann::json{ {"attackID", p.first}, {"exprSymbolTable", p.second.toJson()} });
	}
	j["attackIDs"] = attackIDsJson;

	return j;
}

void ExecuteAttacksDeathAction::load(const nlohmann::json& j) {
	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	attackIDs.clear();
	if (j.contains("attackIDs")) {
		for (const nlohmann::json& item : j.at("attackIDs")) {
			int attackID;
			ExprSymbolTable exprSymbolTable;

			item.at("attackID").get_to(attackID);
			if (item.contains("exprSymbolTable")) {
				exprSymbolTable.load(item.at("exprSymbolTable"));
			}

			attackIDs.push_back(std::make_pair(attackID, exprSymbolTable));
		}
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> ExecuteAttacksDeathAction::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;

	int i = 0;
	for (auto p : attackIDs) {
		if (!levelPack.hasAttack(p.first)) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back("Attack index " + std::to_string(i) + " uses a non-existent attack ID " + std::to_string(p.first));
		}

		i++;
	}

	// TODO: legal check sound settings
	return std::make_pair(status, messages);
}

void ExecuteAttacksDeathAction::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE

	compiledAttackIDs.clear();
	for (auto p : attackIDs) {
		compiledAttackIDs.push_back(std::make_pair(p.first, p.second.toLowerLevelSymbolTable(expr)));
	}
}

void ExecuteAttacksDeathAction::execute(LevelPack & levelPack, EntityCreationQueue & queue, entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	// Bullets from death actions have an attack pattern origin id of -1.
	// Enemy bullets also have an enemy phase origin id of -1.

	if (registry.has<PlayerTag>(entity)) {
		// Execute attacks as player
		for (auto p : compiledAttackIDs) {
			levelPack.getGameplayAttack(p.first, p.second)->executeAsPlayer(queue, spriteLoader, registry, entity, 0, -1);
		}
	} else {
		// Execute attacks as enemy
		int enemyID = -1;
		if (registry.has<EnemyComponent>(entity)) {
			enemyID = registry.get<EnemyComponent>(entity).getEnemyData()->getID();
		}
		for (auto p : compiledAttackIDs) {
			levelPack.getGameplayAttack(p.first, p.second)->executeAsEnemy(queue, spriteLoader, registry, entity, 0, -1, enemyID, -1);
		}
	}
}

std::vector<std::pair<int, ExprSymbolTable>> ExecuteAttacksDeathAction::getAttackIDs() {
	return attackIDs;
}

void ExecuteAttacksDeathAction::setAttackIDs(std::vector<std::pair<int, ExprSymbolTable>> attackIDs) {
	this->attackIDs = attackIDs;
}

ParticleExplosionDeathAction::ParticleExplosionDeathAction() {
}

ParticleExplosionDeathAction::ParticleExplosionDeathAction(PARTICLE_EFFECT effect, Animatable animatable, bool loopAnimatable, sf::Color color, std::string minParticles, 
	std::string maxParticles, std::string minDistance, std::string maxDistance, std::string minLifespan, std::string maxLifespan) 
	: effect(effect), animatable(animatable), color(color), minParticles(minParticles), maxParticles(maxParticles), minDistance(minDistance), 
	maxDistance(maxDistance), minLifespan(minLifespan), maxLifespan(maxLifespan) {
}


std::shared_ptr<LevelPackObject> ParticleExplosionDeathAction::clone() const {
	auto clone = std::make_shared<ParticleExplosionDeathAction>();
	clone->load(format());
	return clone;
}

std::string ParticleExplosionDeathAction::format() const {
	return formatString("ParticleExplosionDeathAction") + tos(static_cast<int>(effect)) + formatTMObject(animatable) + formatBool(loopAnimatable)
		+ tos(color.r) + tos(color.g) + tos(color.b) + tos(color.a) + formatString(minParticles) + formatString(maxParticles) + formatString(minDistance)
		+ formatString(maxDistance) + formatString(minLifespan) + formatString(maxLifespan) + formatTMObject(symbolTable);
}

void ParticleExplosionDeathAction::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	effect = static_cast<PARTICLE_EFFECT>(std::stoi(items.at(1)));
	animatable.load(items.at(2));
	loopAnimatable = unformatBool(items.at(3));
	color.r = std::stoi(items.at(4));
	color.g = std::stoi(items.at(5));
	color.b = std::stoi(items.at(6));
	color.a = std::stoi(items.at(7));
	minParticles = items.at(8);
	maxParticles = items.at(9);
	minDistance = items.at(10);
	maxDistance = items.at(11);
	minLifespan = items.at(12);
	minLifespan = items.at(13);
	symbolTable.load(items.at(14));
}

nlohmann::json ParticleExplosionDeathAction::toJson() {
	return {
		{"className", "ParticleExplosionDeathAction"},
		{"effect", effect},
		{"animatable", animatable.toJson()},
		{"loopAnimatable", loopAnimatable},
		{"color", nlohmann::json{{"r", color.r}, {"g", color.g}, {"b", color.b}, {"a", color.a}}},
		{"minParticles", minParticles},
		{"maxParticles", maxParticles},
		{"minDistance", minDistance},
		{"maxDistance", maxDistance},
		{"minLifespan", minLifespan},
		{"maxLifespan",maxLifespan}
	};
}

void ParticleExplosionDeathAction::load(const nlohmann::json& j) {
	j.at("effect").get_to(effect);

	if (j.contains("animatable")) {
		animatable.load(j.at("animatable"));
	} else {
		animatable = Animatable();
	}

	j.at("loopAnimatable").get_to(loopAnimatable);

	if (j.contains("color")) {
		nlohmann::json colorJson = j.at("color");
		colorJson.at("r").get_to(color.r);
		colorJson.at("g").get_to(color.g);
		colorJson.at("b").get_to(color.b);
		colorJson.at("a").get_to(color.a);
	} else {
		color = sf::Color::Red;
	}

	j.at("minParticles").get_to(minParticles);
	j.at("maxParticles").get_to(maxParticles);
	j.at("minDistance").get_to(minDistance);
	j.at("maxDistance").get_to(maxDistance);
	j.at("minLifespan").get_to(minLifespan);
	j.at("maxLifespan").get_to(maxLifespan);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> ParticleExplosionDeathAction::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	//TODO: legal check
	return std::pair<LEGAL_STATUS, std::vector<std::string>>();
}

void ParticleExplosionDeathAction::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_INT(minParticles)
	COMPILE_EXPRESSION_FOR_INT(maxParticles)
	COMPILE_EXPRESSION_FOR_FLOAT(minDistance)
	COMPILE_EXPRESSION_FOR_FLOAT(maxDistance)
	COMPILE_EXPRESSION_FOR_FLOAT(minLifespan)
	COMPILE_EXPRESSION_FOR_FLOAT(maxLifespan)
}

void ParticleExplosionDeathAction::execute(LevelPack & levelPack, EntityCreationQueue & queue, entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t entity) {
	auto& pos = registry.get<PositionComponent>(entity);
	queue.pushBack(std::make_unique<ParticleExplosionCommand>(registry, spriteLoader, pos.getX(), pos.getY(), animatable, loopAnimatable, effect, color, 
		minParticlesExprCompiledValue, maxParticlesExprCompiledValue, minDistanceExprCompiledValue, maxDistanceExprCompiledValue, minLifespanExprCompiledValue, maxLifespanExprCompiledValue));
}

ParticleExplosionDeathAction::PARTICLE_EFFECT ParticleExplosionDeathAction::getEffect() {
	return effect;
}

sf::Color ParticleExplosionDeathAction::getColor() {
	return color;
}

Animatable ParticleExplosionDeathAction::getAnimatable() {
	return animatable;
}

bool ParticleExplosionDeathAction::getLoopAnimatable() {
	return loopAnimatable;
}

int ParticleExplosionDeathAction::getMinParticles() {
	return minParticlesExprCompiledValue;
}

int ParticleExplosionDeathAction::getMaxParticles() {
	return maxParticlesExprCompiledValue;
}

float ParticleExplosionDeathAction::getMinDistance() {
	return minDistanceExprCompiledValue;
}

float ParticleExplosionDeathAction::getMaxDistance() {
	return maxDistanceExprCompiledValue;
}

float ParticleExplosionDeathAction::getMinLifespan() {
	return minLifespanExprCompiledValue;
}

float ParticleExplosionDeathAction::getMaxLifespan() {
	return maxLifespanExprCompiledValue;
}

void ParticleExplosionDeathAction::setEffect(PARTICLE_EFFECT effect) {
	this->effect = effect;
}

void ParticleExplosionDeathAction::setColor(sf::Color color) {
	this->color = color;
}

void ParticleExplosionDeathAction::setAnimatable(Animatable animatable) {
	this->animatable = animatable;
}

void ParticleExplosionDeathAction::setLoopAnimatable(bool loopAnimatable) {
	this->loopAnimatable = loopAnimatable;
}

void ParticleExplosionDeathAction::setMinParticles(std::string minParticles) {
	this->minParticles = minParticles;
}

void ParticleExplosionDeathAction::setMaxParticles(std::string maxParticles) {
	this->maxParticles = maxParticles;
}

void ParticleExplosionDeathAction::setMinDistance(std::string minDistance) {
	this->minDistance = minDistance;
}

void ParticleExplosionDeathAction::setMaxDistance(std::string maxDistance) {
	this->maxDistance = maxDistance;
}

void ParticleExplosionDeathAction::setMinLifespan(std::string minLifespan) {
	this->minLifespan = minLifespan;
}

void ParticleExplosionDeathAction::setMaxLifespan(std::string maxLifespan) {
	this->maxLifespan = maxLifespan;
}

std::shared_ptr<DeathAction> DeathActionFactory::create(std::string formattedString) {
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
	std::shared_ptr<DeathAction> ptr;
	if (name == "PlayAnimatableDeathAction") {
		ptr = std::make_shared<PlayAnimatableDeathAction>();
	} else if (name == "PlaySoundDeathAction") {
		ptr = std::make_shared<PlaySoundDeathAction>();
	} else if (name == "ExecuteAttacksDeathAction") {
		ptr = std::make_shared<ExecuteAttacksDeathAction>();
	} else if (name == "ParticleExplosionDeathAction") {
		ptr = std::make_shared<ParticleExplosionDeathAction>();
	} else if (name == "NullDeathAction") {
		ptr = std::make_shared<NullDeathAction>();
	}
	ptr->load(formattedString);
	return ptr;
}

std::shared_ptr<DeathAction> DeathActionFactory::create(const nlohmann::json& j) {
	if (j.contains("className")) {
		std::string name;
		j.at("className").get_to(name);

		std::shared_ptr<DeathAction> ptr;
		if (name == "PlayAnimatableDeathAction") {
			ptr = std::make_shared<PlayAnimatableDeathAction>();
		} else if (name == "PlaySoundDeathAction") {
			ptr = std::make_shared<PlaySoundDeathAction>();
		} else if (name == "ExecuteAttacksDeathAction") {
			ptr = std::make_shared<ExecuteAttacksDeathAction>();
		} else if (name == "ParticleExplosionDeathAction") {
			ptr = std::make_shared<ParticleExplosionDeathAction>();
		} else if (name == "NullDeathAction") {
			ptr = std::make_shared<NullDeathAction>();
		}
		ptr->load(j);
		return ptr;
	} else {
		return std::make_shared<NullDeathAction>();
	}
}

NullDeathAction::NullDeathAction() {
}

std::shared_ptr<LevelPackObject> NullDeathAction::clone() const {
	return std::make_shared<NullDeathAction>();
}

std::string NullDeathAction::format() const {
	return formatString("NullDeathAction");
}

void NullDeathAction::load(std::string formattedString) {
}

nlohmann::json NullDeathAction::toJson() {
	return {
		{"className", "NullDeathAction"}
	};
}

void NullDeathAction::load(const nlohmann::json& j) {
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> NullDeathAction::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	// Always legal
	return std::make_pair(LEGAL_STATUS::LEGAL, std::vector<std::string>());
}

void NullDeathAction::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
}

void NullDeathAction::execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) {
}
