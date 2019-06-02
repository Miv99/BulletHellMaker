#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <entt/entt.hpp>
#include "Components.h"
#include "SpriteLoader.h"
#include "TextFileParser.h"
#include <boost/chrono.hpp>
#include "CollisionSystem.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "GameInstance.h"
#include "Constants.h"
// testing
#include "TGUI/TGUI.hpp"
#include "TextMarshallable.h"
#include "EditorInstance.h"

int main() {
	// testing
	GameInstance game("test pack");
	game.loadLevel(0);
	game.start();
	//EditorInstance a;

	std::system("pause");
	return 0;
}