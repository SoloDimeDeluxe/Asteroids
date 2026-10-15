#pragma once

#include "raylib.h"

namespace ReyesCuadra::ship
{
	// Radio del circulo que se usa para chocar (aprox. el tamaño del triangulo)
	constexpr float COLLISION_RADIUS = 14.0f;

	struct Ship
	{
		Vector2 position;
		Vector2 velocity;     // En pixeles por segundo
		float rotation;       // En radianes, 0 = mirando a la derecha
		bool isAccelerating;  // Para dibujar el fuego del motor
		float invulnerableTimeLeft; // Segundos que le quedan sin poder chocar
	};

	// Crea la nave quieta en el centro de la pantalla
	Ship Create();

	void Update(Ship& ship);
	void Draw(const Ship& ship);

	// Punta de la nave (de donde salen las balas)
	Vector2 GetNosePosition(const Ship& ship);

	// Vuelve al centro, quieta e invulnerable por unos segundos (al perder una vida)
	void Respawn(Ship& ship);

	bool IsInvulnerable(const Ship& ship);
}
