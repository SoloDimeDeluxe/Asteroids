#pragma once

#include "raylib.h"

namespace ReyesCuadra::ship
{
	struct Ship
	{
		Vector2 position;
		Vector2 velocity;     // En pixeles por segundo
		float rotation;       // En radianes, 0 = mirando a la derecha
		bool isAccelerating;  // Para dibujar el fuego del motor
	};

	// Crea la nave quieta en el centro de la pantalla
	Ship Create();

	void Update(Ship& ship);
	void Draw(const Ship& ship);
}
