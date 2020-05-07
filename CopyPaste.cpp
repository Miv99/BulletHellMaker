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
	copied = source->copyFrom();
}

void Clipboard::paste(std::shared_ptr<CopyPasteable> target) {
	if (copied && copied->getCopiedFromID() == target->getID()) {
		target->pasteInto(copied);
	}
}

void Clipboard::paste2(std::shared_ptr<CopyPasteable> target) {
	if (copied && copied->getCopiedFromID() == target->getID()) {
		target->paste2Into(copied);
	}
}

void Clipboard::clear() {
	copied = nullptr;
}
