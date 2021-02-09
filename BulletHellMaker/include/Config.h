#pragma once

/*
Project/Non-gameplay related configurations.
*/

// Location of python relative to the .exe
const static char* RELATIVE_PYTHON_PATH = "\\packages\\Python27";

// Location of log files
const static char* RELATIVE_LOGS_FOLDER_PATH = "Logs";
// Max number of log files kept in logs folder
const static int MAX_LOG_FILES = 30;

// ----------------- Level pack ----------------
const static std::string RELATIVE_LEVEL_PACKS_FOLDER_PATH = "Level Packs";
// The %s in the strings in this section represent the level pack name
const static std::string RELATIVE_LEVEL_PACK_SOUND_FOLDER_PATH = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\Sounds";
const static std::string RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\SpriteSheets";
const static std::string RELATIVE_LEVEL_PACK_BACKGROUNDS_FOLDER_PATH = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\Backgrounds";
const static std::string RELATIVE_LEVEL_PACK_GUI_FOLDER_PATH = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\GUI";
const static std::string RELATIVE_LEVEL_PACK_MUSIC_FOLDER_NAME = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\Music";
const static std::string RELATIVE_LEVEL_PACK_ATTACKS_FOLDER_NAME = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\Attacks";
const static std::string RELATIVE_LEVEL_PACK_ATTACK_PATTERNS_FOLDER_NAME = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\AttackPatterns";
const static std::string RELATIVE_LEVEL_PACK_BULLET_MODELS_FOLDER_NAME = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\BulletModels";
const static std::string RELATIVE_LEVEL_PACK_ENEMIES_FOLDER_NAME = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\Enemies";
const static std::string RELATIVE_LEVEL_PACK_ENEMY_PHASES_FOLDER_NAME = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\EnemyPhases";
const static std::string RELATIVE_LEVEL_PACK_LEVELS_FOLDER_NAME = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\Levels";