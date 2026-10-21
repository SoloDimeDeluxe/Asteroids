#include "entities/powerup.h"

#include <cmath>

#include "utils/config.h"
#include "utils/random_utils.h"

static const char* const TEXTURE_PATH = "res/textures/powerup_bomb.png";
static const float DRAW_SIZE = 40.0f;

static const float LIFE_TIME = 8.0f;            // Segundos en pantalla antes de desaparecer
static const float BLINK_START_TIME = 2.0f;     // Ultimos segundos: parpadea para avisar
static const float BLINK_INTERVAL = 0.15f;

// Late (se agranda y achica) para llamar la atencion
static const float PULSE_SPEED = 5.0f;          // Radianes por segundo
static const float PULSE_AMOUNT = 0.12f;        // 12% mas grande o mas chico

// Donde puede aparecer
static const float SPAWN_MARGIN = 80.0f;        // Distancia minima a los bordes
static const float MIN_DISTANCE_FROM_SHIP = 200.0f;
static const int MAX_SPAWN_ATTEMPTS = 20;       // Intentos para encontrar un lugar lejos de la nave

static Texture2D powerUpTexture = {};

static Vector2 GetRandomPosition()
{
	const float screenWidth = static_cast<float>(SCREEN_WIDTH);
	const float screenHeight = static_cast<float>(SCREEN_HEIGHT);

	Vector2 position = {};
	position.x = RandomFloat(SPAWN_MARGIN, screenWidth - SPAWN_MARGIN);
	position.y = RandomFloat(SPAWN_MARGIN, screenHeight - SPAWN_MARGIN);

	return position;
}

static bool IsFarEnough(Vector2 position, Vector2 avoidPosition)
{
	// Distancias al cuadrado, sin raiz (igual que en las colisiones)
	const float distanceX = position.x - avoidPosition.x;
	const float distanceY = position.y - avoidPosition.y;

	return distanceX * distanceX + distanceY * distanceY >= MIN_DISTANCE_FROM_SHIP * MIN_DISTANCE_FROM_SHIP;
}

void LoadPowerUpTexture()
{
	powerUpTexture = LoadTexture(TEXTURE_PATH);
}

PowerUp CreatePowerUp()
{
	PowerUp powerUp = {};
	powerUp.isActive = false;

	return powerUp;
}

void SpawnPowerUp(PowerUp& powerUp, Vector2 avoidPosition)
{
	Vector2 position = GetRandomPosition();

	// Busca un lugar lejos de la nave (si no lo encuentra, usa el ultimo)
	for (int attempt = 0; attempt < MAX_SPAWN_ATTEMPTS && !IsFarEnough(position, avoidPosition); attempt++)
	{
		position = GetRandomPosition();
	}

	powerUp.position = position;
	powerUp.timeLeft = LIFE_TIME;
	powerUp.isActive = true;
}

void UpdatePowerUp(PowerUp& powerUp)
{
	if (!powerUp.isActive)
	{
		return;
	}

	powerUp.timeLeft -= GetFrameTime();

	if (powerUp.timeLeft <= 0.0f)
	{
		powerUp.isActive = false;
	}
}

void DrawPowerUp(const PowerUp& powerUp)
{
	if (!powerUp.isActive)
	{
		return;
	}

	// Cuando le queda poco tiempo parpadea: un intervalo se dibuja y otro no
	if (powerUp.timeLeft < BLINK_START_TIME)
	{
		const int blinkStep = static_cast<int>(powerUp.timeLeft / BLINK_INTERVAL);

		if (blinkStep % 2 == 1)
		{
			return;
		}
	}

	const float scale = 1.0f + std::sin(powerUp.timeLeft * PULSE_SPEED) * PULSE_AMOUNT;
	const float size = DRAW_SIZE * scale;

	const Rectangle source = { 0.0f, 0.0f, static_cast<float>(powerUpTexture.width), static_cast<float>(powerUpTexture.height) };
	const Rectangle destination = { powerUp.position.x, powerUp.position.y, size, size };
	const Vector2 origin = { size / 2.0f, size / 2.0f }; // Centrado en su posicion

	DrawTexturePro(powerUpTexture, source, destination, origin, 0.0f, WHITE);
}

void UnloadPowerUpTexture()
{
	UnloadTexture(powerUpTexture);
}
