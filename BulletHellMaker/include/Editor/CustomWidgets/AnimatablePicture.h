#pragma once
#include <TGUI/TGUI.hpp>

#include <LevelPack/Animation.h>
#include <DataStructs/SpriteLoader.h>

/*
A widget used to display Animatables.
*/
class AnimatablePicture : public tgui::Widget {
public:
	AnimatablePicture();
	AnimatablePicture(const AnimatablePicture& other);
	static std::shared_ptr<AnimatablePicture> create() {
		return std::make_shared<AnimatablePicture>();
	}

	bool update(sf::Time elapsedTime) override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual tgui::Widget::Ptr clone() const override;
	virtual bool mouseOnWidget(tgui::Vector2f pos) const override;

	void setAnimation(SpriteLoader& spriteLoader, const std::string& animationName, const std::string& spriteSheetName);
	void setSprite(SpriteLoader& spriteLoader, const std::string& spriteName, const std::string& spriteSheetName);
	void setSpriteToMissingSprite(SpriteLoader& spriteLoader);

private:
	std::unique_ptr<Animation> animation;
	std::shared_ptr<sf::Sprite> curSprite;

	bool spriteScaledToFitHorizontal;

	void resizeCurSpriteToFitWidget();
};