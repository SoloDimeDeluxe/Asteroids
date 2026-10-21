#pragma once

#include "raylib.h"

// Si la posicion sale por un borde, la pasa al borde contrario.
// margin: cuanto puede salir antes de pasar (el radio del objeto),
// asi desaparece del todo antes de aparecer del otro lado.
void WrapPosition(Vector2& position, float margin);
