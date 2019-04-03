#pragma once

#define PI 3.14159265358979323846f
#define PI2 6.28318530717958647692

// The slowest time between each physics update. If the program takes longer than this time,
// physics is simulated at 1/MAX_PHYSICS_DELTA_TIME FPS.
const static float MAX_PHYSICS_DELTA_TIME = 1 / 30.0f;
// Time between each frame render; render FPS = 1/RENDER_INTERVAL
const static float RENDER_INTERVAL = 1 / 60.0f;

const static int MAP_WIDTH = 600;
const static int MAP_HEIGHT = 700;

const static int PLAYER_SPAWN_X = MAP_WIDTH / 2;
const static int PLAYER_SPAWN_Y = 100;

const static float SHADOW_TRAIL_MAX_OPACITY = 0.75f;

// Amount of entities to reserve space for on level start (1,000,000 entities ~ 700Mb RAM)
const static int INITIAL_ENTITY_RESERVATION = 500000;
//TODO: implement this
// Additional amount of entities to reserve space for when current limit is exceeded
const static int ENTITY_RESERVATION_INCREMENT = 50000;