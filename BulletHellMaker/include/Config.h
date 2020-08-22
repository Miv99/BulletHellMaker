#pragma once

/*
Project/Non-gameplay related configurations.
*/

// Max path length for Windows
static const int MAX_PATH_LENGTH = 261;
// Location of python relative to the .exe
static const char* RELATIVE_PYTHON_PATH = "\\packages\\Python27";

// ----------------- Level pack ----------------
// The %s in the strings in this section represent the level pack name
const static std::string LEVEL_PACK_SOUND_FOLDER_PATH = "Level Packs\\%s\\Sounds";