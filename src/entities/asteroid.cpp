#include "entities/asteroid.h"

#include <cmath>

#include "utils/config.h"
#include "utils/screen_wrap.h"

namespace ReyesCuadra::asteroid
{
	namespace
	{
		// Radio de cada tamaño
		constexpr float LARGE_RADIUS = 45.0f;
		constexpr float MEDIUM_RADIUS = 25.0f;
		constexpr float SMALL_RADIUS = 12.0f;

		// Velocidad minima y maxima de cada tamaño (los chicos son mas rapidos)
		constexpr float LARGE_MIN_SPEED = 50.0f;
		constexpr float LARGE_MAX_SPEED = 90.0f;
		constexpr float MEDIUM_MIN_SPEED = 90.0f;
		constexpr float MEDIUM_MAX_SPEED = 140.0f;
		constexpr float SMALL_MIN_SPEED = 140.0f;
		constexpr float SMALL_MAX_SPEED = 200.0f;

		constexpr float MAX_ROTATION_SPEED = 60.0f; // Grados por segundo

		// Zona central a la que apuntan al aparecer (para que entren a la pantalla)
		constexpr float TARGET_AREA_MARGIN = 200.0f;

		// Los dos pedazos salen abriendose hacia los costados de la direccion original
		constexpr float SPLIT_ANGLE = 35.0f * DEG2RAD;

		// Sprites (uno por tamaño)
		constexpr const char* LARGE_TEXTURE_PATH = "res/textures/asteroid_large.png";
		constexpr const char* MEDIUM_TEXTURE_PATH = "res/textures/asteroid_medium.png";
		constexpr const char* SMALL_TEXTURE_PATH = "res/textures/asteroid_small.png";

		// La roca ocupa el 92% del ancho de la imagen (el resto es transparente)
		constexpr float TEXTURE_FILL = 0.92f;

		Texture2D largeTexture = {};
		Texture2D mediumTexture = {};
		Texture2D smallTexture = {};

		const Texture2D& GetTexture(Size size)
		{
			switch (size)
			{
			case Size::Large:
				return largeTexture;
			case Size::Medium:
				return mediumTexture;
			case Size::Small:
				return smallTexture;
			}

			return smallTexture;
		}

		// Numero al azar entre min y max (con decimales)
		float RandomFloat(float min, float max)
		{
			constexpr int PRECISION = 10000;
			const float randomPercent = static_cast<float>(GetRandomValue(0, PRECISION)) / static_cast<float>(PRECISION);

			return min + (max - min) * randomPercent;
		}

		float GetRadius(Size size)
		{
			switch (size)
			{
			case Size::Large:
				return LARGE_RADIUS;
			case Size::Medium:
				return MEDIUM_RADIUS;
			case Size::Small:
				return SMALL_RADIUS;
			}

			return SMALL_RADIUS;
		}

		float GetRandomSpeed(Size size)
		{
			switch (size)
			{
			case Size::Large:
				return RandomFloat(LARGE_MIN_SPEED, LARGE_MAX_SPEED);
			case Size::Medium:
				return RandomFloat(MEDIUM_MIN_SPEED, MEDIUM_MAX_SPEED);
			case Size::Small:
				return RandomFloat(SMALL_MIN_SPEED, SMALL_MAX_SPEED);
			}

			return SMALL_MIN_SPEED;
		}
	}

	void LoadTextures()
	{
		largeTexture = LoadTexture(LARGE_TEXTURE_PATH);
		mediumTexture = LoadTexture(MEDIUM_TEXTURE_PATH);
		smallTexture = LoadTexture(SMALL_TEXTURE_PATH);

		SetTextureFilter(largeTexture, TEXTURE_FILTER_BILINEAR);
		SetTextureFilter(mediumTexture, TEXTURE_FILTER_BILINEAR);
		SetTextureFilter(smallTexture, TEXTURE_FILTER_BILINEAR);
	}

	void UnloadTextures()
	{
		UnloadTexture(largeTexture);
		UnloadTexture(mediumTexture);
		UnloadTexture(smallTexture);
	}

	void InitAll(Asteroid asteroids[])
	{
		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			asteroids[i] = {};
			asteroids[i].isActive = false;
		}
	}

	void Spawn(Asteroid asteroids[], Vector2 position, float direction, Size size)
	{
		// Busca el primer lugar libre
		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (!asteroids[i].isActive)
			{
				const float speed = GetRandomSpeed(size);

				asteroids[i].position = position;
				asteroids[i].velocity.x = std::cos(direction) * speed;
				asteroids[i].velocity.y = std::sin(direction) * speed;
				asteroids[i].size = size;
				asteroids[i].radius = GetRadius(size);
				asteroids[i].rotation = RandomFloat(0.0f, 360.0f);
				asteroids[i].rotationSpeed = RandomFloat(-MAX_ROTATION_SPEED, MAX_ROTATION_SPEED);
				asteroids[i].isActive = true;

				return;
			}
		}

		// Si no hay lugar libre, no aparece
	}

	void SpawnFromEdge(Asteroid asteroids[])
	{
		const float screenWidth = static_cast<float>(config::SCREEN_WIDTH);
		const float screenHeight = static_cast<float>(config::SCREEN_HEIGHT);

		// Justo afuera de la pantalla, para que no aparezca de golpe
		const float outside = LARGE_RADIUS;

		Vector2 position = {};

		// Elige un borde al azar: 0 arriba, 1 derecha, 2 abajo, 3 izquierda
		switch (GetRandomValue(0, 3))
		{
		case 0:
			position = { RandomFloat(0.0f, screenWidth), -outside };
			break;
		case 1:
			position = { screenWidth + outside, RandomFloat(0.0f, screenHeight) };
			break;
		case 2:
			position = { RandomFloat(0.0f, screenWidth), screenHeight + outside };
			break;
		default:
			position = { -outside, RandomFloat(0.0f, screenHeight) };
			break;
		}

		// Apunta a un punto al azar de la zona central
		const float targetX = RandomFloat(TARGET_AREA_MARGIN, screenWidth - TARGET_AREA_MARGIN);
		const float targetY = RandomFloat(TARGET_AREA_MARGIN, screenHeight - TARGET_AREA_MARGIN);
		const float direction = std::atan2(targetY - position.y, targetX - position.x);

		Spawn(asteroids, position, direction, Size::Large);
	}

	void Destroy(Asteroid asteroids[], int index)
	{
		// Copia los datos antes de liberar el lugar
		const Asteroid destroyed = asteroids[index];

		asteroids[index].isActive = false;

		// El chico desaparece sin dividirse
		if (destroyed.size == Size::Small)
		{
			return;
		}

		const Size smallerSize = (destroyed.size == Size::Large) ? Size::Medium : Size::Small;

		// Direccion en la que venia (angulo de su vector velocidad)
		const float direction = std::atan2(destroyed.velocity.y, destroyed.velocity.x);

		// Un pedazo para cada lado
		Spawn(asteroids, destroyed.position, direction + SPLIT_ANGLE, smallerSize);
		Spawn(asteroids, destroyed.position, direction - SPLIT_ANGLE, smallerSize);
	}

	int CountActive(const Asteroid asteroids[])
	{
		int activeCount = 0;

		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (asteroids[i].isActive)
			{
				activeCount++;
			}
		}

		return activeCount;
	}

	void UpdateAll(Asteroid asteroids[])
	{
		const float deltaTime = GetFrameTime();

		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (!asteroids[i].isActive)
			{
				continue;
			}

			asteroids[i].position.x += asteroids[i].velocity.x * deltaTime;
			asteroids[i].position.y += asteroids[i].velocity.y * deltaTime;
			asteroids[i].rotation += asteroids[i].rotationSpeed * deltaTime;

			// Igual que la nave, aparece del otro lado
			screen_wrap::WrapPosition(asteroids[i].position, asteroids[i].radius);
		}
	}

	void DrawAll(const Asteroid asteroids[])
	{
		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (!asteroids[i].isActive)
			{
				continue;
			}

			const Texture2D& texture = GetTexture(asteroids[i].size);

			// Tamaño en pantalla para que la roca coincida con el radio de colision
			const float drawSize = asteroids[i].radius * 2.0f / TEXTURE_FILL;

			const Rectangle source = { 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) };
			const Rectangle destination = { asteroids[i].position.x, asteroids[i].position.y, drawSize, drawSize };
			const Vector2 origin = { drawSize / 2.0f, drawSize / 2.0f }; // Gira sobre su centro

			// La rotacion del asteroide ya esta en grados
			DrawTexturePro(texture, source, destination, origin, asteroids[i].rotation, WHITE);
		}
	}
}
