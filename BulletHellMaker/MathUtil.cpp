#include "MathUtil.h"
#include <cmath>

// TODO: make a lookup table
float binom(int n, int k) {
    return 1.0f / ((n + 1) * std::beta(n - k + 1, k + 1));
}
