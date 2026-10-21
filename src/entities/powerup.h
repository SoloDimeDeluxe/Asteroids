#pragma once

#include "raylib.h"

namespace ReyesCuadra
{
	// Radio del circulo para agarrarlo con la nave
	const float POWERUP_COLLISION_RADIUS = 18.0f;

	// Power-up de bomba: al agarrarlo destruye todos los asteroides de la pantalla
	struct PowerUp
	{
		Vector2 position;
		float timeLeft;   // Segundos que le quedan en pantalla
		bool isActive;    // false = no esta en pantalla
	};

	// Carga la imagen (despues de InitWindow)
	void LoadPowerUpTexture();

	// Crea el power-up sin mostrar (inactivo)
	PowerUp CreatePowerUp();

	// Lo hace aparecer en un lugar al azar, lejos de "avoidPosition" (la nave)
	void SpawnPowerUp(PowerUp& powerUp, Vector2 avoidPosition);

	void UpdatePowerUp(PowerUp& powerUp);
	void DrawPowerUp(const PowerUp& powerUp);

	// Libera la imagen (antes de CloseWindow)
	void UnloadPowerUpTexture();
}
