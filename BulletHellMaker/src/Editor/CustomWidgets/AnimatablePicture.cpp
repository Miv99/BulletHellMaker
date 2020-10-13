#include <Editor/CustomWidgets/AnimatablePicture.h>

#include <Editor/Util/TguiUtils.h>

AnimatablePicture::AnimatablePicture() {
	onSizeChange.connect([this]() {
		resizeCurSpriteToFitWidget();
	});
}

AnimatablePicture::AnimatablePicture(const AnimatablePicture& other) {
}

bool AnimatablePicture::updateTime(tgui::Duration elapsedTime) {
	bool ret = tgui::Widget::updateTime(elapsedTime);

	if (animation) {
		std::shared_ptr<sf::Sprite> prevSprite = curSprite;
		setCurSprite(animation->update(elapsedTime.asSeconds()));
		if (curSprite != prevSprite) {
			resizeCurSpriteToFitWidget();
			return true;
		}
	}

	return ret;
}

void AnimatablePicture::draw(tgui::BackendRenderTargetBase& target, tgui::RenderStates states) const {
	if (curSprite) {
		tgui::BackendRenderTargetSFML& sfmlTarget = dynamic_cast<tgui::BackendRenderTargetSFML&>(target);
		sf::RenderTarget& renderTarget = *sfmlTarget.getTarget();
		sf::RenderStates sfmlStates = toSFMLRenderStates(states);

		curSprite->setPosition(getPosition().x + curSprite->getOrigin().x * curSprite->getScale().x, getPosition().y + curSprite->getOrigin().y * curSprite->getScale().y);
		renderTarget.draw(*curSprite, sfmlStates);
	}
}

tgui::Widget::Ptr AnimatablePicture::clone() const {
	return std::make_shared<AnimatablePicture>(*this);
}

bool AnimatablePicture::isMouseOnWidget(tgui::Vector2f pos) const {
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
	setCurSprite(spriteLoader.getSprite(spriteName, spriteSheetName));
	animation = nullptr;
	resizeCurSpriteToFitWidget();
}

void AnimatablePicture::setSpriteToMissingSprite(SpriteLoader& spriteLoader) {
	setCurSprite(spriteLoader.getMissingSprite());
	animation = nullptr;
	resizeCurSpriteToFitWidget();

}

void AnimatablePicture::setEmptyAnimatable() {
	setCurSprite(nullptr);
	animation = nullptr;
}

void AnimatablePicture::setCurSprite(std::shared_ptr<sf::Sprite> curSprite) {
	this->curSprite = curSprite;

	if (curSprite) {
		curSpriteOriginalScale = curSprite->getScale();
	}
}

void AnimatablePicture::resizeCurSpriteToFitWidget() {
	if (!curSprite) {
		return;
	}

	float width = curSprite->getLocalBounds().width * curSpriteOriginalScale.x;
	float height = curSprite->getLocalBounds().height * curSpriteOriginalScale.y;
	float scaleAmount;
	if (getSize().x/getSize().y < width/height) {
		scaleAmount = getSize().x / width;
		spriteScaledToFitHorizontal = true;
	} else {
		scaleAmount = getSize().y / height;
		spriteScaledToFitHorizontal = false;
	}
	curSprite->setScale(curSpriteOriginalScale.x * scaleAmount, curSpriteOriginalScale.y * scaleAmount);
}