#include "EditorInstance.h"

EditorInstance::EditorInstance(std::string levelPackName) {
	audioPlayer = std::make_shared<AudioPlayer>();
	levelPack = std::make_shared<LevelPack>(*audioPlayer, levelPackName);

	spriteLoader = levelPack->createSpriteLoader();
	spriteLoader->preloadTextures();

	// testing
	attackEditor = std::make_shared<AttackEditor>(levelPack, spriteLoader);
	attackEditor->start();
}
