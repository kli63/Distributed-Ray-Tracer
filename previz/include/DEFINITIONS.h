#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <memory>
#include <iostream>

#include "SETTINGS.h"

using namespace std;
using POINT3 = VEC3;

VEC3 rotateY(VEC3& v, double angle)
{
    double rad = angle * (M_PI / 180.0);
    MATRIX3 My;
    My <<   cos(rad), 0.0, sin(rad),
            0.0, 1.0, 0.0,
            -sin(rad), 0.0, cos(rad);

    return My * v;
}

float clamp(float value)
{
  if (value < 0.0)      return 0.0;
  else if (value > 1.0) return 1.0;
  return value;
}

VEC3 random_in_unit_disk() {
    VEC3 p;
    do {
        // Generate random x and y values in the range [-1, 1]
        float x = 2.0f * float(rand()) / RAND_MAX - 1.0f;
        float y = 2.0f * float(rand()) / RAND_MAX - 1.0f;
        p = VEC3(x, y, 0);

    } while (p.dot(p) >= 1.0); // Repeat if the point is outside the unit disk

    return p;
}

inline double random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min) * random_double();
}


#endif