#include "entities/bullet.h"

#include <cmath>

#include "utils/screen_wrap.h"

namespace ReyesCuadra::bullet
{
	namespace
	{
		constexpr float SPEED = 600.0f;    // Pixeles por segundo
		constexpr float LIFE_TIME = 1.0f;  // Segundos

		constexpr const char* TEXTURE_PATH = "res/textures/bullet.png";
		constexpr float DRAW_SIZE = 12.0f; // Mas grande que el radio: incluye el brillo

		Texture2D bulletTexture = {};
	}

	void LoadTexture()
	{
		// "::" para usar la funcion de raylib y no esta misma
		bulletTexture = ::LoadTexture(TEXTURE_PATH);
		SetTextureFilter(bulletTexture, TEXTURE_FILTER_BILINEAR);
	}

	void UnloadTexture()
	{
		::UnloadTexture(bulletTexture);
	}

	void InitAll(Bullet bullets[])
	{
		for (int i = 0; i < MAX_BULLETS; i++)
		{
			bullets[i] = {};
			bullets[i].isActive = false;
		}
	}

	bool Fire(Bullet bullets[], Vector2 position, float rotation)
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

				return true;
			}
		}

		// Si no hay lugar libre, no dispara
		return false;
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
				// Centrada en la posicion de la bala
				const Vector2 topLeft = { bullets[i].position.x - DRAW_SIZE / 2.0f, bullets[i].position.y - DRAW_SIZE / 2.0f };
				const Rectangle source = { 0.0f, 0.0f, static_cast<float>(bulletTexture.width), static_cast<float>(bulletTexture.height) };
				const Rectangle destination = { topLeft.x, topLeft.y, DRAW_SIZE, DRAW_SIZE };

				DrawTexturePro(bulletTexture, source, destination, { 0.0f, 0.0f }, 0.0f, WHITE);
			}
		}
	}
}
