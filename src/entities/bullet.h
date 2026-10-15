#pragma once

#include "raylib.h"

namespace ReyesCuadra::bullet
{
	// Cantidad maxima de balas en pantalla al mismo tiempo
	constexpr int MAX_BULLETS = 20;

	struct Bullet
	{
		Vector2 position;
		Vector2 velocity;    // En pixeles por segundo
		float timeLeft;      // Segundos que le quedan antes de desaparecer
		bool isActive;       // false = lugar libre en el arreglo
	};

	// Deja todas las balas inactivas
	void InitAll(Bullet bullets[]);

	// Activa una bala libre en esa posicion y direccion (si hay alguna libre)
	void Fire(Bullet bullets[], Vector2 position, float rotation);

	void UpdateAll(Bullet bullets[]);
	void DrawAll(const Bullet bullets[]);
}
