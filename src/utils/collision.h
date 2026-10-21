#pragma once

#include "raylib.h"

// true si los dos circulos se tocan.
// Compara distancias al cuadrado para no usar sqrt (mas eficiente).
bool AreCirclesColliding(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB);
