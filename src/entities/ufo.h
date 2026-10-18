#pragma once

#include "raylib.h"

namespace ReyesCuadra::ufo
{
	// Radio del circulo de colision (el platillo es mas ancho que alto)
	constexpr float COLLISION_RADIUS = 24.0f;

	// Enemigo que cruza la pantalla en zigzag y le dispara a la nave
	struct Ufo
	{
		Vector2 position;
		float directionX;   // 1 = va a la derecha, -1 = a la izquierda
		float baseY;        // Altura central del zigzag
		float aliveTime;    // Segundos desde que aparecio (para el zigzag)
		float shootTimer;   // Segundos que faltan para el proximo disparo
		bool isActive;      // false = no esta en pantalla
	};

	// Carga la imagen (despues de InitWindow)
	void LoadTexture();

	// Crea el OVNI sin mostrar (inactivo)
	Ufo Create();

	// Lo hace entrar por un costado al azar, a una altura al azar
	void Spawn(Ufo& ufo);

	// Lo mueve en zigzag; al salir por el otro lado se desactiva
	void Update(Ufo& ufo);

	// true cuando le toca disparar (y reinicia el tiempo para el proximo)
	bool ShouldShoot(Ufo& ufo);

	// Direccion (en radianes) para dispararle al objetivo, con un poco de error
	float GetShotDirection(const Ufo& ufo, Vector2 targetPosition);

	void Draw(const Ufo& ufo);

	// Libera la imagen (antes de CloseWindow)
	void UnloadTexture();
}
