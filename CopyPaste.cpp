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
	copied = source->copyFrom();
}

void Clipboard::paste(CopyPasteable * target) {
	if (copied && copied->getCopiedFromID() == target->getID()) {
		target->pasteInto(copied);
	}
}

void Clipboard::paste2(CopyPasteable * target) {
	if (copied && copied->getCopiedFromID() == target->getID()) {
		target->paste2Into(copied);
	}
}

void Clipboard::clear() {
	copied = nullptr;
}

CopiedEditorMovablePoint::CopiedEditorMovablePoint(std::string copiedFromID, std::shared_ptr<EditorMovablePoint> emp) : CopiedObject(copiedFromID) {
	this->emp = std::make_shared<EditorMovablePoint>(emp);
}

std::shared_ptr<EditorMovablePoint> CopiedEditorMovablePoint::getEMP() {
	// Return a deep copy of the EMP
	return std::make_shared<EditorMovablePoint>(emp);
}

CopiedEditorAttack::CopiedEditorAttack(std::string copiedFromID, std::vector<std::shared_ptr<EditorAttack>> attacks) : CopiedObject(copiedFromID) {
	// Deep copy every attack
	for (auto attack : attacks) {
		this->attacks.push_back(std::make_shared<EditorAttack>(attack));
	}
}

std::vector<std::shared_ptr<EditorAttack>> CopiedEditorAttack::getAttacks() {
	// Return deep copies of the attacks
	std::vector<std::shared_ptr<EditorAttack>> copies;
	for (auto attack : attacks) {
		copies.push_back(std::make_shared<EditorAttack>(attack));
	}
	return copies;
}

int CopiedEditorAttack::getAttacksCount() {
	return attacks.size();
}

CopiedPiecewiseTFVSegment::CopiedPiecewiseTFVSegment(std::string copiedFromID, std::pair<float, std::shared_ptr<TFV>> segment) : CopiedObject(copiedFromID) {
	this->segment = std::make_pair(segment.first, segment.second->clone());
}

std::pair<float, std::shared_ptr<TFV>> CopiedPiecewiseTFVSegment::getSegment() {
	return std::make_pair(segment.first, segment.second->clone());
}
