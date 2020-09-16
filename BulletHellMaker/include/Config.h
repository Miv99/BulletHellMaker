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
const static std::string RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\Sprite Sheets";
const static std::string RELATIVE_LEVEL_PACK_BACKGROUNDS_FOLDER_PATH = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\Backgrounds";
const static std::string RELATIVE_LEVEL_PACK_GUI_FOLDER_PATH = RELATIVE_LEVEL_PACKS_FOLDER_PATH + "\\%s\\GUI";