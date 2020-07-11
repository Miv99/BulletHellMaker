#include "CopyPaste.h"

CopyPasteable::CopyPasteable(std::string id) : id(id) {
}

std::string CopyPasteable::getID() {
	return id;
}

CopiedObject::CopiedObject(std::string copiedFromID) : copiedFromID(copiedFromID) {
}

std::string CopiedObject::getCopiedFromID() {
	return copiedFromID;
}

Clipboard::Clipboard() {
	onCopy = std::make_shared<entt::SigH<void(std::string)>>();
	onPaste = std::make_shared<entt::SigH<void(std::string)>>();
}

void Clipboard::copy(std::shared_ptr<CopyPasteable> source) {
	copy(source.get());
}

void Clipboard::paste(std::shared_ptr<CopyPasteable> target) {
	paste(target.get());
}

void Clipboard::paste2(std::shared_ptr<CopyPasteable> target) {
	paste2(target.get());
}

void Clipboard::copy(CopyPasteable * source) {
	auto result = source->copyFrom();
	copied = result.first;
	if (result.second != "") {
		onCopy->publish(result.second);
	}
}

void Clipboard::paste(CopyPasteable * target) {
	if (copied && copied->getCopiedFromID() == target->getID()) {
		std::string result = target->pasteInto(copied);
		if (result != "") {
			onPaste->publish(result);
		}
	}
}

void Clipboard::paste2(CopyPasteable * target) {
	if (copied && copied->getCopiedFromID() == target->getID()) {
		std::string result = target->paste2Into(copied);
		if (result != "") {
			onPaste->publish(result);
		}
	}
}

void Clipboard::clear() {
	copied = nullptr;
}

std::shared_ptr<entt::SigH<void(std::string)>> Clipboard::getOnCopy() {
	return onCopy;
}

std::shared_ptr<entt::SigH<void(std::string)>> Clipboard::getOnPaste() {
	return onPaste;
}

CopiedEditorMovablePoint::CopiedEditorMovablePoint(std::string copiedFromID, std::shared_ptr<EditorMovablePoint> emp) : CopiedObject(copiedFromID) {
	this->emp = std::make_shared<EditorMovablePoint>(emp);
}

std::shared_ptr<EditorMovablePoint> CopiedEditorMovablePoint::getEMP() {
	// Return a deep copy of the EMP
	return std::make_shared<EditorMovablePoint>(emp);
}

CopiedPiecewiseTFVSegment::CopiedPiecewiseTFVSegment(std::string copiedFromID, std::pair<float, std::shared_ptr<TFV>> segment) : CopiedObject(copiedFromID) {
	this->segment = std::make_pair(segment.first, segment.second->clone());
}

std::pair<float, std::shared_ptr<TFV>> CopiedPiecewiseTFVSegment::getSegment() {
	return std::make_pair(segment.first, segment.second->clone());
}

CopiedLevelPackObject::CopiedLevelPackObject(std::string copiedFromID, std::vector<std::shared_ptr<LevelPackObject>> objs) : CopiedObject(copiedFromID) {
	// Deep copy every object
	for (auto obj : objs) {
		this->objs.push_back(obj->clone());
	}
}

std::vector<std::shared_ptr<LevelPackObject>> CopiedLevelPackObject::getLevelPackObjects() {
	// Return deep copies of the objects
	std::vector<std::shared_ptr<LevelPackObject>> copies;
	for (auto obj : objs) {
		copies.push_back(obj->clone());
	}
	return copies;
}

int CopiedLevelPackObject::getLevelPackObjectsCount() {
	return objs.size();
}

CopiedEMPActions::CopiedEMPActions(std::string copiedFromID, std::vector<std::shared_ptr<EMPAction>> actions) : CopiedObject(copiedFromID) {
	// Deep copy every action
	for (auto action : actions) {
		this->actions.push_back(std::dynamic_pointer_cast<EMPAction>(action->clone()));
	}
}

std::vector<std::shared_ptr<EMPAction>> CopiedEMPActions::getActions() {
	// Return deep copies of the actions
	std::vector<std::shared_ptr<EMPAction>> copies;
	for (auto action : actions) {
		copies.push_back(std::dynamic_pointer_cast<EMPAction>(action->clone()));
	}
	return copies;
}

int CopiedEMPActions::getActionsCount() {
	return actions.size();
}

CopiedMarker::CopiedMarker(std::string copiedFromID, sf::CircleShape marker) : CopiedObject(copiedFromID), marker(marker) {
}

sf::CircleShape CopiedMarker::getMarker() {
	return marker;
}
