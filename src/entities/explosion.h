#pragma once

#include "raylib.h"

// Explosiones que pueden verse al mismo tiempo (la bomba hace explotar muchas juntas)
const int MAX_EXPLOSIONS = 40;

// Animacion de explosion: recorre los cuadros de la hoja de sprites y se apaga sola
struct Explosion
{
	Vector2 position;
	float size;          // Ancho y alto en pantalla
	float rotation;      // En grados, al azar para que no se vean todas iguales
	float elapsedTime;   // Segundos desde que empezo
	bool isActive;       // false = lugar libre en el arreglo
};

// Carga la hoja de sprites (despues de InitWindow)
void LoadExplosionTexture();

// Deja todas las explosiones inactivas
void InitExplosions(Explosion explosions[]);

// Empieza una explosion en esa posicion (si hay lugar libre)
void SpawnExplosion(Explosion explosions[], Vector2 position, float size);

void UpdateExplosions(Explosion explosions[]);
void DrawExplosions(const Explosion explosions[]);

// Libera la imagen (antes de CloseWindow)
void UnloadExplosionTexture();
