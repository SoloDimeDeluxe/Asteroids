#include "entities/ufo.h"

#include <cmath>

#include "utils/config.h"
#include "utils/random_utils.h"

static const char* const TEXTURE_PATH = "res/textures/ufo.png";
static const float DRAW_WIDTH = 64.0f;
static const float DRAW_HEIGHT = 36.0f;

// Movimiento
static const float SPEED = 110.0f;               // Pixeles por segundo (horizontal)
static const float ZIGZAG_AMPLITUDE = 70.0f;     // Cuanto sube y baja
static const float ZIGZAG_SPEED = 2.0f;          // Radianes por segundo
static const float SPAWN_HEIGHT_MARGIN = 150.0f; // Distancia minima a los bordes de arriba y abajo
static const float OUTSIDE_MARGIN = DRAW_WIDTH;  // Aparece y desaparece afuera de la pantalla

// Inclinacion mientras zigzaguea (solo visual)
static const float MAX_TILT_DEGREES = 10.0f;

// Disparo
static const float FIRST_SHOT_DELAY = 1.5f;      // Segundos antes del primer disparo
static const float SHOOT_INTERVAL = 1.8f;        // Segundos entre disparos
static const float MAX_AIM_ERROR = 12.0f * DEG2RAD; // Error maximo al apuntar

static Texture2D ufoTexture = {};

static bool HasLeftScreen(const Ufo& ufo)
{
	const float screenWidth = static_cast<float>(SCREEN_WIDTH);

	if (ufo.directionX > 0.0f)
	{
		return ufo.position.x > screenWidth + OUTSIDE_MARGIN;
	}

	return ufo.position.x < -OUTSIDE_MARGIN;
}

void LoadUfoTexture()
{
	ufoTexture = LoadTexture(TEXTURE_PATH);
}

Ufo CreateUfo()
{
	Ufo ufo = {};
	ufo.isActive = false;

	return ufo;
}

void SpawnUfo(Ufo& ufo)
{
	const float screenWidth = static_cast<float>(SCREEN_WIDTH);
	const float screenHeight = static_cast<float>(SCREEN_HEIGHT);

	// Mitad de las veces entra por la izquierda y mitad por la derecha
	const bool entersFromLeft = (GetRandomValue(0, 1) == 0);

	ufo.directionX = entersFromLeft ? 1.0f : -1.0f;
	ufo.baseY = RandomFloat(SPAWN_HEIGHT_MARGIN, screenHeight - SPAWN_HEIGHT_MARGIN);
	ufo.position.x = entersFromLeft ? -OUTSIDE_MARGIN : screenWidth + OUTSIDE_MARGIN;
	ufo.position.y = ufo.baseY;
	ufo.aliveTime = 0.0f;
	ufo.shootTimer = FIRST_SHOT_DELAY;
	ufo.isActive = true;
}

void UpdateUfo(Ufo& ufo)
{
	if (!ufo.isActive)
	{
		return;
	}

	const float deltaTime = GetFrameTime();

	ufo.aliveTime += deltaTime;
	ufo.shootTimer -= deltaTime;

	// Avanza de costado y sube/baja siguiendo una onda (seno)
	ufo.position.x += ufo.directionX * SPEED * deltaTime;
	ufo.position.y = ufo.baseY + std::sin(ufo.aliveTime * ZIGZAG_SPEED) * ZIGZAG_AMPLITUDE;

	// No da la vuelta como los asteroides: cruza una vez y se va
	if (HasLeftScreen(ufo))
	{
		ufo.isActive = false;
	}
}

bool ShouldUfoShoot(Ufo& ufo)
{
	if (!ufo.isActive || ufo.shootTimer > 0.0f)
	{
		return false;
	}

	ufo.shootTimer = SHOOT_INTERVAL;
	return true;
}

float GetUfoShotDirection(const Ufo& ufo, Vector2 targetPosition)
{
	// Angulo hacia el objetivo (igual que la nave hacia el mouse) mas un error al azar
	const float aimDirection = std::atan2(targetPosition.y - ufo.position.y, targetPosition.x - ufo.position.x);

	return aimDirection + RandomFloat(-MAX_AIM_ERROR, MAX_AIM_ERROR);
}

void DrawUfo(const Ufo& ufo)
{
	if (!ufo.isActive)
	{
		return;
	}

	// Se inclina para el lado al que se mueve en el zigzag
	const float tilt = std::cos(ufo.aliveTime * ZIGZAG_SPEED) * MAX_TILT_DEGREES * ufo.directionX;

	const Rectangle source = { 0.0f, 0.0f, static_cast<float>(ufoTexture.width), static_cast<float>(ufoTexture.height) };
	const Rectangle destination = { ufo.position.x, ufo.position.y, DRAW_WIDTH, DRAW_HEIGHT };
	const Vector2 origin = { DRAW_WIDTH / 2.0f, DRAW_HEIGHT / 2.0f };

	DrawTexturePro(ufoTexture, source, destination, origin, tilt, WHITE);
}

void UnloadUfoTexture()
{
	UnloadTexture(ufoTexture);
}
