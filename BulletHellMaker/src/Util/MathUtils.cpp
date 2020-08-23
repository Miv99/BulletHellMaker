#include <Util/MathUtils.h>

#include <cmath>

#include <Game/Components/HitboxComponent.h>
#include <Game/Components/PositionComponent.h>

// TODO: make a lookup table
float binom(int n, int k) {
    return 1.0f / ((n + 1) * std::beta(n - k + 1, k + 1));
}

float distance(float x1, float y1, float x2, float y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, const HitboxComponent& h2) {
    return distance(p1.getX() + h1.getX(), p1.getY() + h1.getY(), p2.getX() + h2.getX(), p2.getY() + h2.getY()) <= (h1.getRadius() + h2.getRadius());
}

bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, float h2x, float h2y, float h2radius) {
    return distance(p1.getX() + h1.getX(), p1.getY() + h1.getY(), p2.getX() + h2x, p2.getY() + h2y) <= (h1.getRadius() + h2radius);
}
