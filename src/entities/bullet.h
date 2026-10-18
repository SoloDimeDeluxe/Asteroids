#pragma once

#include "raylib.h"

namespace ReyesCuadra::bullet
{
	// Cantidad maxima de balas de cada arreglo al mismo tiempo
	constexpr int MAX_BULLETS = 20;

	// Radio de cada bala (tambien se usa para las colisiones)
	constexpr float RADIUS = 2.5f;

	// Quien disparo la bala: cambia velocidad, duracion e imagen
	enum class BulletType
	{
		Player,
		Enemy
	};

	struct Bullet
	{
		Vector2 position;
		Vector2 velocity;    // En pixeles por segundo
		float timeLeft;      // Segundos que le quedan antes de desaparecer
		BulletType type;
		bool isActive;       // false = lugar libre en el arreglo
	};

	// Carga las imagenes de las balas (despues de InitWindow)
	void LoadTextures();

	// Deja todas las balas inactivas
	void InitAll(Bullet bullets[]);

	// Activa una bala libre en esa posicion y direccion.
	// Devuelve false si no habia lugar libre (no se disparo)
	bool Fire(Bullet bullets[], Vector2 position, float rotation, BulletType type);

	void UpdateAll(Bullet bullets[]);
	void DrawAll(const Bullet bullets[]);

	// Libera las imagenes (antes de CloseWindow)
	void UnloadTextures();
}
