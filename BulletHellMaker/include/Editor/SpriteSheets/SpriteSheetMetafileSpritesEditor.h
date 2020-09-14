#pragma once
#include <TGUI/TGUI.hpp>

#include <Editor/EventCapturable.h>
#include <Editor/ViewController.h>

class MainEditorWindow;

/*
A panel that can load sprite sheets and edit the sprites of a sprite sheet's metafile.
*/
class SpriteSheetMetafileSpritesEditor : public tgui::Panel, public EventCapturable {
public:
	SpriteSheetMetafileSpritesEditor(MainEditorWindow& mainEditorWindow);
	static std::shared_ptr<SpriteSheetMetafileSpritesEditor> create(MainEditorWindow& mainEditorWindow) {
		return std::make_shared<SpriteSheetMetafileSpritesEditor>(mainEditorWindow);
	}
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	bool handleEvent(sf::Event event) override;

	void loadImage(std::string fileName);

	void resetCamera();
	
private:
	MainEditorWindow& mainEditorWindow;

	sf::View viewFromViewController;
	sf::FloatRect viewportFloatRect, viewFloatRect;

	std::unique_ptr<ViewController> viewController;
};