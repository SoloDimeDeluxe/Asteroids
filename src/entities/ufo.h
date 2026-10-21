#pragma once

#include "raylib.h"

// Radio del circulo de colision (el platillo es mas ancho que alto)
const float UFO_COLLISION_RADIUS = 24.0f;

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
void LoadUfoTexture();

// Crea el OVNI sin mostrar (inactivo)
Ufo CreateUfo();

// Lo hace entrar por un costado al azar, a una altura al azar
void SpawnUfo(Ufo& ufo);

// Lo mueve en zigzag; al salir por el otro lado se desactiva
void UpdateUfo(Ufo& ufo);

// true cuando le toca disparar (y reinicia el tiempo para el proximo)
bool ShouldUfoShoot(Ufo& ufo);

// Direccion (en radianes) para dispararle al objetivo, con un poco de error
float GetUfoShotDirection(const Ufo& ufo, Vector2 targetPosition);

void DrawUfo(const Ufo& ufo);

// Libera la imagen (antes de CloseWindow)
void UnloadUfoTexture();
