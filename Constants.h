#pragma once

#define PI 3.14159265358979323846f
#define PI2 6.28318530717958647692

// The slowest time between each physics update. If the program takes longer than this time,
// physics is simulated at 1/MAX_PHYSICS_DELTA_TIME FPS.
const static float MAX_PHYSICS_DELTA_TIME = 1 / 30.0f;
// Time between each frame render; render FPS = 1/RENDER_INTERVAL
const static float RENDER_INTERVAL = 1 / 60.0f;

// Epsilon for checking float equality
const static float EPSILON = MAX_PHYSICS_DELTA_TIME / 10.0f;

const static int MAP_WIDTH = 550;
const static int MAP_HEIGHT = 700;

const static int PLAYER_SPAWN_X = MAP_WIDTH / 2;
const static int PLAYER_SPAWN_Y = 100;

const static float SHADOW_TRAIL_MAX_OPACITY = 0.75f;

// Amount of entities to reserve space for on level start (1,000,000 entities ~ 700Mb RAM)
const static int INITIAL_ENTITY_RESERVATION = 500000;
// Amount of entities to reserve space for when editing a LevelPack
const static int INITIAL_EDITOR_ENTITY_RESERVATION = 5000;
// Additional amount of entities to reserve space for when current limit is exceeded
const static int ENTITY_RESERVATION_INCREMENT = 50000;

// Time before an item despawns
const static float ITEM_DESPAWN_TIME = 11.0f;

const static int HEALTH_PER_HEALTH_PACK = 1;
const static int POINTS_PER_POINTS_PACK = 100;
const static int POWER_PER_POWER_PACK = 1;

// Points for every power received after reaching the maximum
const static int POINTS_PER_EXTRA_POWER = POINTS_PER_POINTS_PACK;
// Points for every bomb received after reaching the maximum
const static int POINTS_PER_EXTRA_BOMB = POINTS_PER_POINTS_PACK * 20;

const static int POWER_PER_POWER_TIER = 50;

/*
Render layers

Highest layer is drawn on top.
Must go from 0 to some number without skipping any.
*/
const static int SHADOW_LAYER = 0;
const static int PARTICLE_LAYER = 1;
const static int PLAYER_BULLET_LAYER = 2;
const static int ENEMY_LAYER = 3;
const static int ENEMY_BOSS_LAYER = 4;
const static int PLAYER_LAYER = 5;
const static int ITEM_LAYER = 6;
const static int ENEMY_BULLET_LAYER = 7;

const static int HIGHEST_RENDER_LAYER = ENEMY_BULLET_LAYER;

// ----------------- GUI stuff -----------------------
// Height of any tgui::Widget that involves text, excluding buttons
const static float TEXT_BOX_HEIGHT = 16;
// Height of any button that involves text
const static float TEXT_BUTTON_HEIGHT = 16;
// Height of any slider
const static float SLIDER_HEIGHT = 15;
const static int TEXT_SIZE = 12;
const static float CHECKBOX_SIZE = 15;
const static float SMALL_BUTTON_SIZE = 20;
// Width of every tab in a tguI::Tabs button
const static float TAB_WIDTH = 100.0f;
// Padding around widgets, when applicable
const static float GUI_PADDING_X = 20;
const static float GUI_PADDING_Y = 10;
// Y padding inbetween labels and the widget they're explaining
const static float GUI_LABEL_PADDING_Y = 5;