#pragma once

#include "raylib.h"

namespace ReyesCuadra::explosion
{
	// Explosiones que pueden verse al mismo tiempo (la bomba hace explotar muchas juntas)
	constexpr int MAX_EXPLOSIONS = 40;

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
	void LoadTexture();

	// Deja todas las explosiones inactivas
	void InitAll(Explosion explosions[]);

	// Empieza una explosion en esa posicion (si hay lugar libre)
	void Spawn(Explosion explosions[], Vector2 position, float size);

	void UpdateAll(Explosion explosions[]);
	void DrawAll(const Explosion explosions[]);

	// Libera la imagen (antes de CloseWindow)
	void UnloadTexture();
}
