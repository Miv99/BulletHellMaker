#include "EditorUtilities.h"
#include <map>

std::shared_ptr<tgui::Label> createToolTip(std::string text) {
	auto tooltip = tgui::Label::create();
	tooltip->setMaximumTextWidth(300);
	tooltip->setText(text);
	tooltip->setTextSize(12);
	tooltip->getRenderer()->setBackgroundColor(tgui::Color::White);

	return tooltip;
}

AnimatableChooser::AnimatableChooser(SpriteLoader& spriteLoader, bool forceSprite) : spriteLoader(spriteLoader), forceSprite(forceSprite) {
	const std::map<std::string, std::shared_ptr<SpriteSheet>> sheets = spriteLoader.getSpriteSheets();
	for (auto it = sheets.begin(); it != sheets.end(); it++) {
		const std::map<std::string, std::shared_ptr<SpriteData>> spriteData = it->second->getSpriteData();
		addItem(it->first);
		for (auto it2 = spriteData.begin(); it2 != spriteData.end(); it2++) {
			addItem("[S]" + it2->first, it->first);
		}

		if (!forceSprite) {
			const std::map<std::string, std::shared_ptr<AnimationData>> animationData = it->second->getAnimationData();
			for (auto it2 = animationData.begin(); it2 != animationData.end(); it2++) {
				addItem("[A]" + it2->first, it->first);
			}
		}
	}

	setChangeItemOnScroll(false);
	setExpandDirection(tgui::ComboBox::ExpandDirection::Down);
}

void AnimatableChooser::calculateItemsToDisplay() {
	if (!getParent()) {
		return;
	}

	if (getExpandDirection() == tgui::ComboBox::ExpandDirection::Down) {
		float availableSpace = getParent()->getSize().y - getPosition().y - getSize().y;
		// Each item in the ComboBox has height equal to the ComboBox's height
		setItemsToDisplay((int)(availableSpace / getSize().y));
	}
}

std::shared_ptr<AnimatablePicture> AnimatableChooser::getAnimatablePicture() {
	if (!animatablePicture) {
		animatablePicture = std::make_shared<AnimatablePicture>();
		connect("ItemSelected", [&](std::string itemText, std::string spriteSheetName) {
			// The only items without an ID are sprite sheet name indicators, so if ID is empty, this item shouldn't be selectable
			if (spriteSheetName == "") {
				deselectItem();
			} else {
				// Item text is in format "[S]spriteName" or "[A]animationName"
				if (itemText[1] == 'S') {
					animatablePicture->setSprite(spriteLoader, itemText.substr(3), spriteSheetName);
				} else {
					animatablePicture->setAnimation(spriteLoader, itemText.substr(3), spriteSheetName);
				}
			}
		});
	}
	return animatablePicture;
}

void AnimatablePicture::update(float deltaTime) {
	if (animation) {
		std::shared_ptr<sf::Sprite> sprite = animation->update(deltaTime);
		getRenderer()->setTexture(*sprite->getTexture());
	}
}

void AnimatablePicture::setAnimation(SpriteLoader& spriteLoader, const std::string& animationName, const std::string& spriteSheetName) {
	this->animation = spriteLoader.getAnimation(animationName, spriteSheetName, true);
}

void AnimatablePicture::setSprite(SpriteLoader& spriteLoader, const std::string& spriteName, const std::string& spriteSheetName) {
	getRenderer()->setTexture(*spriteLoader.getSprite(spriteName, spriteSheetName)->getTexture());
	animation = nullptr;
}

SliderWithEditBox::SliderWithEditBox() {
	editBox = tgui::EditBox::create();
	//TODO: connect() such that both change each other (for editbox: do on return key press to prevent infinite looping)
}
