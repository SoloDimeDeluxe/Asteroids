#pragma once

#include "raylib.h"

// Lugar para todos los asteroides (incluye los que salen al dividirse)
const int MAX_ASTEROIDS = 40;

enum class AsteroidSize
{
	Large,
	Medium,
	Small
};

struct Asteroid
{
	Vector2 position;
	Vector2 velocity;     // En pixeles por segundo
	AsteroidSize size;
	float radius;
	float rotation;       // En grados (solo visual)
	float rotationSpeed;  // Grados por segundo
	bool isActive;        // false = lugar libre en el arreglo
};

// Carga las imagenes de los 3 tamaños (despues de InitWindow)
void LoadAsteroidTextures();

// Deja todos los asteroides inactivos
void InitAsteroids(Asteroid asteroids[]);

// Crea un asteroide grande desde afuera de un borde, yendo hacia la pantalla
void SpawnAsteroidFromEdge(Asteroid asteroids[]);

// Rompe el asteroide: el grande se divide en 2 medianos,
// el mediano en 2 chicos y el chico desaparece
void DestroyAsteroid(Asteroid asteroids[], int index);

// Destruye todos los asteroides de una (sin dividirlos). Devuelve cuantos habia
int DestroyAllAsteroids(Asteroid asteroids[]);

int CountActiveAsteroids(const Asteroid asteroids[]);

void UpdateAsteroids(Asteroid asteroids[]);
void DrawAsteroids(const Asteroid asteroids[]);

// Libera las imagenes (antes de CloseWindow)
void UnloadAsteroidTextures();
