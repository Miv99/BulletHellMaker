#pragma once
#include <thread>
#include <memory>
#include <string>
#include "AttackEditor.h"
#include "LevelPack.h"
#include "AudioPlayer.h"
#include "SpriteLoader.h"

class EditorInstance {
public:
	//TODO: make a window that's like a main menu of editors; all editor windows are closed when EditorInstance is closed
	EditorInstance(std::string levelPackName);

private:
	std::shared_ptr<LevelPack> levelPack;
	std::shared_ptr<AudioPlayer> audioPlayer;
	std::shared_ptr<SpriteLoader> spriteLoader;

	std::shared_ptr<AttackEditor> attackEditor;
};