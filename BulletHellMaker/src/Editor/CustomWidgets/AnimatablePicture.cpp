#include <Editor/CustomWidgets/AnimatablePicture.h>

AnimatablePicture::AnimatablePicture() {
	connect("SizeChanged", [&]() {
		resizeCurSpriteToFitWidget();
	});
}

AnimatablePicture::AnimatablePicture(const AnimatablePicture& other) {
}

bool AnimatablePicture::update(sf::Time elapsedTime) {
	bool ret = tgui::Widget::update(elapsedTime);

	if (animation) {
		std::shared_ptr<sf::Sprite> prevSprite = curSprite;
		curSprite = animation->update(elapsedTime.asSeconds());
		if (curSprite != prevSprite) {
			resizeCurSpriteToFitWidget();
			return true;
		}
	}

	return ret;
}

void AnimatablePicture::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (curSprite) {
		if (spriteScaledToFitHorizontal) {
			curSprite->setPosition(getPosition().x + getSize().x / 2, getPosition().y + curSprite->getOrigin().y * curSprite->getScale().y);
		} else {
			curSprite->setPosition(getPosition().x + curSprite->getOrigin().x * curSprite->getScale().x, getPosition().y + getSize().y / 2);
		}
		target.draw(*curSprite, states);
	}
}

tgui::Widget::Ptr AnimatablePicture::clone() const {
	return std::make_shared<AnimatablePicture>(*this);
}

bool AnimatablePicture::mouseOnWidget(tgui::Vector2f pos) const {
	if (tgui::FloatRect{ 0, 0, getSize().x, getSize().y }.contains(pos)) {
		return true;
	}
	return false;
}

void AnimatablePicture::setAnimation(SpriteLoader& spriteLoader, const std::string& animationName, const std::string& spriteSheetName) {
	std::unique_ptr<Animation> animation = spriteLoader.getAnimation(animationName, spriteSheetName, true);
	if (animation) {
		this->animation = std::move(animation);
	} else {
		setSpriteToMissingSprite(spriteLoader);
	}
}

void AnimatablePicture::setSprite(SpriteLoader& spriteLoader, const std::string& spriteName, const std::string& spriteSheetName) {
	curSprite = spriteLoader.getSprite(spriteName, spriteSheetName);
	animation = nullptr;
	resizeCurSpriteToFitWidget();
}

void AnimatablePicture::setSpriteToMissingSprite(SpriteLoader& spriteLoader) {
	curSprite = spriteLoader.getMissingSprite();
	animation = nullptr;
	resizeCurSpriteToFitWidget();

}

void AnimatablePicture::resizeCurSpriteToFitWidget() {
	if (!curSprite) {
		return;
	}

	float width = curSprite->getLocalBounds().width * curSprite->getScale().x;
	float height = curSprite->getLocalBounds().height * curSprite->getScale().y;
	float scaleAmount;
	if (width > height) {
		scaleAmount = getSize().x / width;
		spriteScaledToFitHorizontal = true;
	} else {
		scaleAmount = getSize().y / height;
		spriteScaledToFitHorizontal = false;
	}
	curSprite->scale(scaleAmount, scaleAmount);
}