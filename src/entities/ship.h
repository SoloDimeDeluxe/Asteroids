#pragma once

#include "raylib.h"

namespace ReyesCuadra::ship
{
	struct Ship
	{
		Vector2 position;
		float rotation; // En radianes, 0 = mirando a la derecha
	};

	// Crea la nave en el centro de la pantalla
	Ship Create();

	void Update(Ship& ship);
	void Draw(const Ship& ship);
}
