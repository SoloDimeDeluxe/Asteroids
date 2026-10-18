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

	// Carga las imagenes de los 3 tamaños (despues de InitWindow)
	void LoadTextures();

	// Deja todos los asteroides inactivos
	void InitAll(Asteroid asteroids[]);

	// Crea un asteroide grande desde afuera de un borde, yendo hacia la pantalla
	void SpawnFromEdge(Asteroid asteroids[]);

	// Rompe el asteroide: el grande se divide en 2 medianos,
	// el mediano en 2 chicos y el chico desaparece
	void Destroy(Asteroid asteroids[], int index);

	int CountActive(const Asteroid asteroids[]);

	void UpdateAll(Asteroid asteroids[]);
	void DrawAll(const Asteroid asteroids[]);

	// Libera las imagenes (antes de CloseWindow)
	void UnloadTextures();
}
