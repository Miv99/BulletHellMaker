#include "EditorInstance.h"

EditorInstance::EditorInstance(std::string levelPackName) {
	audioPlayer = std::make_shared<AudioPlayer>();
	levelPack = std::make_shared<LevelPack>(*audioPlayer, levelPackName);

	// testing
	attackEditor = std::make_shared<AttackEditor>(*levelPack);
	attackEditor->start();
}
