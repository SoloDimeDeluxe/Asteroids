#pragma once

#include "raylib.h"

// Radio del circulo que se usa para chocar (aprox. el tamaño del triangulo)
const float SHIP_COLLISION_RADIUS = 16.0f;

struct Ship
{
	Vector2 position;
	Vector2 velocity;     // En pixeles por segundo
	float rotation;       // En radianes, 0 = mirando a la derecha
	bool isAccelerating;  // Para dibujar el fuego del motor
	float flameAnimationTime; // Segundos acelerando (elige el cuadro del fuego)
	float invulnerableTimeLeft; // Segundos que le quedan sin poder chocar
};

// Carga las imagenes de la nave (despues de InitWindow)
void LoadShipTextures();

// Crea la nave quieta en el centro de la pantalla
Ship CreateShip();

void UpdateShip(Ship& ship);
void DrawShip(const Ship& ship);

// Dibuja la nave en rojo y quieta (cuando la destruye un asteroide)
void DrawDestroyedShip(const Ship& ship);

// Punta de la nave (de donde salen las balas)
Vector2 GetShipNosePosition(const Ship& ship);

// Vuelve al centro, quieta e invulnerable por unos segundos (al perder una vida)
void RespawnShip(Ship& ship);

bool IsShipInvulnerable(const Ship& ship);

// Libera las imagenes de la nave (antes de CloseWindow)
void UnloadShipTextures();
