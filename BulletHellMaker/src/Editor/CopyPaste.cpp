#include <Editor/CopyPaste.h>

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

CopyOperationResult Clipboard::copy(std::shared_ptr<CopyPasteable> source) {
	return copy(source.get());
}

PasteOperationResult Clipboard::paste(std::shared_ptr<CopyPasteable> target) {
	return paste(target.get());
}

PasteOperationResult Clipboard::paste2(std::shared_ptr<CopyPasteable> target) {
	return paste2(target.get());
}

CopyOperationResult Clipboard::copy(CopyPasteable * source) {
	CopyOperationResult result = source->copyFrom();
	copied = result.copiedObject;
	if (result.description != "") {
		onCopy->publish(result.description);
	}
	return result;
}

PasteOperationResult Clipboard::paste(CopyPasteable * target) {
	if (copied && copied->getCopiedFromID() == target->getID()) {
		PasteOperationResult result = target->pasteInto(copied);
		if (result.description != "") {
			onPaste->publish(result.description);
		}
		return result;
	}
	return PasteOperationResult(false, "");
}

PasteOperationResult Clipboard::paste2(CopyPasteable * target) {
	if (copied && copied->getCopiedFromID() == target->getID()) {
		PasteOperationResult result = target->paste2Into(copied);
		if (result.description != "") {
			onPaste->publish(result.description);
		}
		return result;
	}
	return PasteOperationResult(false, "");
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

CopiedLayerRootLevelPackObject::CopiedLayerRootLevelPackObject(std::string copiedFromID, std::vector<std::shared_ptr<LayerRootLevelPackObject>> objs) 
	: CopiedObject(copiedFromID) {

	// Deep copy every object
	for (auto obj : objs) {
		this->objs.push_back(std::dynamic_pointer_cast<LayerRootLevelPackObject>(obj->clone()));
	}
}

std::vector<std::shared_ptr<LayerRootLevelPackObject>> CopiedLayerRootLevelPackObject::getLevelPackObjects() {
	// Return deep copies of the objects
	std::vector<std::shared_ptr<LayerRootLevelPackObject>> copies;
	for (auto obj : objs) {
		copies.push_back(std::dynamic_pointer_cast<LayerRootLevelPackObject>(obj->clone()));
	}
	return copies;
}

int CopiedLayerRootLevelPackObject::getLevelPackObjectsCount() {
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

CopiedAttackPatternToAttackUseRelationship::CopiedAttackPatternToAttackUseRelationship(std::string copiedFromID, 
	std::vector<std::tuple<std::string, int, ExprSymbolTable>> relationships) 
	: CopiedObject(copiedFromID), relationships(relationships) {
}

std::vector<std::tuple<std::string, int, ExprSymbolTable>> CopiedAttackPatternToAttackUseRelationship::getRelationships() {
	return relationships;
}

int CopiedAttackPatternToAttackUseRelationship::getRelationshipsCount() {
	return relationships.size();
}
