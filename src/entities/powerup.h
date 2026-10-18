#pragma once

#include "raylib.h"

namespace ReyesCuadra::powerup
{
	// Radio del circulo para agarrarlo con la nave
	constexpr float COLLISION_RADIUS = 18.0f;

	// Power-up de bomba: al agarrarlo destruye todos los asteroides de la pantalla
	struct PowerUp
	{
		Vector2 position;
		float timeLeft;   // Segundos que le quedan en pantalla
		bool isActive;    // false = no esta en pantalla
	};

	// Carga la imagen (despues de InitWindow)
	void LoadTexture();

	// Crea el power-up sin mostrar (inactivo)
	PowerUp Create();

	// Lo hace aparecer en un lugar al azar, lejos de "avoidPosition" (la nave)
	void Spawn(PowerUp& powerUp, Vector2 avoidPosition);

	void Update(PowerUp& powerUp);
	void Draw(const PowerUp& powerUp);

	// Libera la imagen (antes de CloseWindow)
	void UnloadTexture();
}
