#pragma once

class PositionComponent;
class HitboxComponent;

#define PI 3.14159265358979323846f
#define PI2 6.28318530717958647692f

/*
Returns the value of n choose k.
*/
float binom(int n, int k);

/*
Returns the distance between two points.
*/
float distance(float x1, float y1, float x2, float y2);

/*
Returns whether an entity with PositionComponent p1 and HitboxComponent h1
is colliding with an entity with PositionComponent p2 and HitboxComponent h2.
*/
bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, const HitboxComponent& h2);
bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, float h2x, float h2y, float h2radius);