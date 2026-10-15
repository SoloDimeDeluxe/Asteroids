#include "entities/bullet.h"

#include <cmath>

#include "utils/screen_wrap.h"

namespace ReyesCuadra::bullet
{
	namespace
	{
		constexpr float SPEED = 600.0f;    // Pixeles por segundo
		constexpr float LIFE_TIME = 1.0f;  // Segundos
		constexpr float RADIUS = 2.5f;

		const Color BULLET_COLOR = RAYWHITE;
	}

	void InitAll(Bullet bullets[])
	{
		for (int i = 0; i < MAX_BULLETS; i++)
		{
			bullets[i] = {};
			bullets[i].isActive = false;
		}
	}

	void Fire(Bullet bullets[], Vector2 position, float rotation)
	{
		// Busca el primer lugar libre
		for (int i = 0; i < MAX_BULLETS; i++)
		{
			if (!bullets[i].isActive)
			{
				bullets[i].position = position;

				// Misma direccion que la nave
				bullets[i].velocity.x = std::cos(rotation) * SPEED;
				bullets[i].velocity.y = std::sin(rotation) * SPEED;

				bullets[i].timeLeft = LIFE_TIME;
				bullets[i].isActive = true;

				return;
			}
		}

		// Si no hay lugar libre, no dispara
	}

	void UpdateAll(Bullet bullets[])
	{
		const float deltaTime = GetFrameTime();

		for (int i = 0; i < MAX_BULLETS; i++)
		{
			if (!bullets[i].isActive)
			{
				continue;
			}

			bullets[i].position.x += bullets[i].velocity.x * deltaTime;
			bullets[i].position.y += bullets[i].velocity.y * deltaTime;

			// Igual que la nave, aparece del otro lado
			screen_wrap::WrapPosition(bullets[i].position, RADIUS);

			// Se apaga cuando se le acaba el tiempo
			bullets[i].timeLeft -= deltaTime;

			if (bullets[i].timeLeft <= 0.0f)
			{
				bullets[i].isActive = false;
			}
		}
	}

	void DrawAll(const Bullet bullets[])
	{
		for (int i = 0; i < MAX_BULLETS; i++)
		{
			if (bullets[i].isActive)
			{
				DrawCircleV(bullets[i].position, RADIUS, BULLET_COLOR);
			}
		}
	}
}
