#include "entities/bullet.h"

#include <cmath>

#include "utils/screen_wrap.h"

namespace ReyesCuadra::bullet
{
	// Balas de la nave: rapidas y cortas
	static constexpr float PLAYER_SPEED = 600.0f;     // Pixeles por segundo
	static constexpr float PLAYER_LIFE_TIME = 1.0f;   // Segundos
	static constexpr float PLAYER_DRAW_SIZE = 12.0f;  // Mas grande que el radio: incluye el brillo

	// Balas del OVNI: mas lentas (para poder esquivarlas) y duran mas
	static constexpr float ENEMY_SPEED = 300.0f;
	static constexpr float ENEMY_LIFE_TIME = 2.2f;
	static constexpr float ENEMY_DRAW_SIZE = 14.0f;

	static constexpr const char* PLAYER_TEXTURE_PATH = "res/textures/bullet.png";
	static constexpr const char* ENEMY_TEXTURE_PATH = "res/textures/enemy_bullet.png";

	static Texture2D playerTexture = {};
	static Texture2D enemyTexture = {};

	static float GetSpeed(BulletType type)
	{
		return (type == BulletType::Player) ? PLAYER_SPEED : ENEMY_SPEED;
	}

	static float GetLifeTime(BulletType type)
	{
		return (type == BulletType::Player) ? PLAYER_LIFE_TIME : ENEMY_LIFE_TIME;
	}

	void LoadTextures()
	{
		playerTexture = LoadTexture(PLAYER_TEXTURE_PATH);
		enemyTexture = LoadTexture(ENEMY_TEXTURE_PATH);

		SetTextureFilter(playerTexture, TEXTURE_FILTER_BILINEAR);
		SetTextureFilter(enemyTexture, TEXTURE_FILTER_BILINEAR);
	}

	void InitAll(Bullet bullets[])
	{
		for (int i = 0; i < MAX_BULLETS; i++)
		{
			bullets[i] = {};
			bullets[i].isActive = false;
		}
	}

	bool Fire(Bullet bullets[], Vector2 position, float rotation, BulletType type)
	{
		// Busca el primer lugar libre
		for (int i = 0; i < MAX_BULLETS; i++)
		{
			if (!bullets[i].isActive)
			{
				const float speed = GetSpeed(type);

				bullets[i].position = position;
				bullets[i].velocity.x = std::cos(rotation) * speed;
				bullets[i].velocity.y = std::sin(rotation) * speed;
				bullets[i].timeLeft = GetLifeTime(type);
				bullets[i].type = type;
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
			if (!bullets[i].isActive)
			{
				continue;
			}

			const bool isPlayerBullet = (bullets[i].type == BulletType::Player);
			const Texture2D& texture = isPlayerBullet ? playerTexture : enemyTexture;
			const float drawSize = isPlayerBullet ? PLAYER_DRAW_SIZE : ENEMY_DRAW_SIZE;

			// Centrada en la posicion de la bala
			const Rectangle source = { 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) };
			const Rectangle destination = { bullets[i].position.x, bullets[i].position.y, drawSize, drawSize };
			const Vector2 origin = { drawSize / 2.0f, drawSize / 2.0f };

			DrawTexturePro(texture, source, destination, origin, 0.0f, WHITE);
		}
	}

	void UnloadTextures()
	{
		UnloadTexture(playerTexture);
		UnloadTexture(enemyTexture);
	}
}
