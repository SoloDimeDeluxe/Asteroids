#include "entities/explosion.h"

#include "utils/random_utils.h"

namespace ReyesCuadra::explosion
{
	static constexpr const char* TEXTURE_PATH = "res/textures/explosion.png";

	// La hoja de sprites tiene los cuadros uno al lado del otro
	static constexpr int FRAME_COUNT = 8;
	static constexpr float FRAME_TIME = 0.06f;  // Segundos que dura cada cuadro
	static constexpr float DURATION = FRAME_COUNT * FRAME_TIME;
	static constexpr float FULL_TURN_DEGREES = 360.0f;

	static Texture2D explosionTexture = {};

	void LoadTexture()
	{
		// "::" para usar la funcion de raylib y no esta misma
		explosionTexture = ::LoadTexture(TEXTURE_PATH);
		SetTextureFilter(explosionTexture, TEXTURE_FILTER_BILINEAR);
	}

	void InitAll(Explosion explosions[])
	{
		for (int i = 0; i < MAX_EXPLOSIONS; i++)
		{
			explosions[i] = {};
			explosions[i].isActive = false;
		}
	}

	void Spawn(Explosion explosions[], Vector2 position, float size)
	{
		// Busca el primer lugar libre
		for (int i = 0; i < MAX_EXPLOSIONS; i++)
		{
			if (!explosions[i].isActive)
			{
				explosions[i].position = position;
				explosions[i].size = size;
				explosions[i].rotation = random_utils::RandomFloat(0.0f, FULL_TURN_DEGREES);
				explosions[i].elapsedTime = 0.0f;
				explosions[i].isActive = true;

				return;
			}
		}

		// Si no hay lugar libre, esa explosion no se ve
	}

	void UpdateAll(Explosion explosions[])
	{
		const float deltaTime = GetFrameTime();

		for (int i = 0; i < MAX_EXPLOSIONS; i++)
		{
			if (!explosions[i].isActive)
			{
				continue;
			}

			// Avanza con el tiempo real (no por frame), asi dura lo mismo a cualquier FPS
			explosions[i].elapsedTime += deltaTime;

			if (explosions[i].elapsedTime >= DURATION)
			{
				explosions[i].isActive = false;
			}
		}
	}

	void DrawAll(const Explosion explosions[])
	{
		const float frameWidth = static_cast<float>(explosionTexture.width) / static_cast<float>(FRAME_COUNT);
		const float frameHeight = static_cast<float>(explosionTexture.height);

		for (int i = 0; i < MAX_EXPLOSIONS; i++)
		{
			if (!explosions[i].isActive)
			{
				continue;
			}

			// Cuadro actual segun el tiempo que paso
			int frame = static_cast<int>(explosions[i].elapsedTime / FRAME_TIME);

			if (frame >= FRAME_COUNT)
			{
				frame = FRAME_COUNT - 1;
			}

			// Recorta de la hoja solo el cuadro que toca
			const Rectangle source = { frameWidth * static_cast<float>(frame), 0.0f, frameWidth, frameHeight };
			const Rectangle destination = { explosions[i].position.x, explosions[i].position.y, explosions[i].size, explosions[i].size };
			const Vector2 origin = { explosions[i].size / 2.0f, explosions[i].size / 2.0f };

			DrawTexturePro(explosionTexture, source, destination, origin, explosions[i].rotation, WHITE);
		}
	}

	void UnloadTexture()
	{
		::UnloadTexture(explosionTexture);
	}
}
