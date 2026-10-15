#pragma once

#include "raylib.h"

namespace ReyesCuadra::asteroid
{
	// Lugar para todos los asteroides (incluye los que salen al dividirse)
	constexpr int MAX_ASTEROIDS = 40;

	enum class Size
	{
		Large,
		Medium,
		Small
	};

	struct Asteroid
	{
		Vector2 position;
		Vector2 velocity;     // En pixeles por segundo
		Size size;
		float radius;
		float rotation;       // En grados (solo visual)
		float rotationSpeed;  // Grados por segundo
		bool isActive;        // false = lugar libre en el arreglo
	};

	// Deja todos los asteroides inactivos
	void InitAll(Asteroid asteroids[]);

	// Activa un asteroide libre con esos datos (si hay alguno libre)
	void Spawn(Asteroid asteroids[], Vector2 position, float direction, Size size);

	// Crea un asteroide grande desde afuera de un borde, yendo hacia la pantalla
	void SpawnFromEdge(Asteroid asteroids[]);

	int CountActive(const Asteroid asteroids[]);

	void UpdateAll(Asteroid asteroids[]);
	void DrawAll(const Asteroid asteroids[]);
}
