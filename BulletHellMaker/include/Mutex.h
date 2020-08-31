#pragma once
#include <mutex>

// Mutex used to make sure multiple TGUI widgets aren't being instantiated at the same time in different threads.
// tgui::Gui draw() calls also can't be done at the same time.
// Apparently TGUI gets super messed up with multithreading.
static std::recursive_mutex tguiMutex;