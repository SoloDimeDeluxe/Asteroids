#include "screens/gameplay.h"

#include "raylib.h"

#include "entities/asteroid.h"
#include "entities/bullet.h"
#include "entities/ship.h"
#include "utils/collision.h"
#include "utils/config.h"
#include "utils/ui.h"

namespace ReyesCuadra::gameplay
{
	namespace
	{
		constexpr int PAUSE_FONT_SIZE = 60;
		constexpr int PAUSE_TITLE_POSITION_Y = 220;

		// Boton de pausa (arriba a la derecha)
		constexpr float PAUSE_BUTTON_WIDTH = 140.0f;
		constexpr float PAUSE_BUTTON_HEIGHT = 45.0f;
		constexpr float PAUSE_BUTTON_MARGIN = 15.0f;

		// Botones del menu de pausa
		constexpr float MENU_BUTTON_WIDTH = 260.0f;
		constexpr float MENU_BUTTON_HEIGHT = 60.0f;
		constexpr float RESUME_BUTTON_POSITION_Y = 340.0f;
		constexpr float BUTTON_SPACING = 90.0f;

		// Aparicion de asteroides
		constexpr int INITIAL_ASTEROIDS = 4;
		constexpr float SPAWN_INTERVAL = 5.0f;      // Segundos entre cada asteroide nuevo
		constexpr int MAX_ACTIVE_FOR_SPAWN = 8;     // No aparecen mas si ya hay estos

		bool isPaused = false;
		ship::Ship playerShip = {};
		bullet::Bullet bullets[bullet::MAX_BULLETS] = {};
		asteroid::Asteroid asteroids[asteroid::MAX_ASTEROIDS] = {};
		float spawnTimer = 0.0f;

		ui::Button pauseButton = {};
		ui::Button resumeButton = {};
		ui::Button menuButton = {};

		// Clic izquierdo (o espacio) dispara.
		// Si el mouse esta sobre el boton de pausa no dispara,
		// asi el clic para pausar no tira una bala.
		bool IsFirePressed()
		{
			const bool clickedToFire = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ui::IsButtonHovered(pauseButton);

			return clickedToFire || IsKeyPressed(KEY_SPACE);
		}

		// Cada SPAWN_INTERVAL segundos aparece un asteroide grande
		void UpdateAsteroidSpawn()
		{
			spawnTimer += GetFrameTime();

			if (spawnTimer >= SPAWN_INTERVAL)
			{
				spawnTimer = 0.0f;

				if (asteroid::CountActive(asteroids) < MAX_ACTIVE_FOR_SPAWN)
				{
					asteroid::SpawnFromEdge(asteroids);
				}
			}
		}

		// Si una bala toca un asteroide, se apagan los dos (y el asteroide se divide)
		void CheckBulletAsteroidCollisions()
		{
			for (int bulletIndex = 0; bulletIndex < bullet::MAX_BULLETS; bulletIndex++)
			{
				if (!bullets[bulletIndex].isActive)
				{
					continue;
				}

				for (int asteroidIndex = 0; asteroidIndex < asteroid::MAX_ASTEROIDS; asteroidIndex++)
				{
					if (!asteroids[asteroidIndex].isActive)
					{
						continue;
					}

					if (collision::AreCirclesColliding(bullets[bulletIndex].position, bullet::RADIUS,
						asteroids[asteroidIndex].position, asteroids[asteroidIndex].radius))
					{
						bullets[bulletIndex].isActive = false;
						asteroid::Destroy(asteroids, asteroidIndex);

						// Esta bala ya se gasto, pasa a la siguiente
						break;
					}
				}
			}
		}

		// Aca va la logica de la partida (nave, asteroides, balas)
		void UpdateMatch()
		{
			ship::Update(playerShip);

			if (IsFirePressed())
			{
				bullet::Fire(bullets, ship::GetNosePosition(playerShip), playerShip.rotation);
			}

			bullet::UpdateAll(bullets);

			UpdateAsteroidSpawn();
			asteroid::UpdateAll(asteroids);

			CheckBulletAsteroidCollisions();
		}

		// Aca se dibuja la partida
		void DrawMatch()
		{
			asteroid::DrawAll(asteroids);
			bullet::DrawAll(bullets);
			ship::Draw(playerShip);
		}

		void DrawPauseOverlay()
		{
			// Capa oscura sobre la partida
			DrawRectangle(0, 0, config::SCREEN_WIDTH, config::SCREEN_HEIGHT, Fade(BLACK, 0.7f));

			ui::DrawCenteredText("PAUSA", PAUSE_TITLE_POSITION_Y, PAUSE_FONT_SIZE, RAYWHITE);

			ui::DrawButton(resumeButton);
			ui::DrawButton(menuButton);
		}
	}

	void Init()
	{
		isPaused = false;
		playerShip = ship::Create();
		bullet::InitAll(bullets);

		asteroid::InitAll(asteroids);
		spawnTimer = 0.0f;

		for (int i = 0; i < INITIAL_ASTEROIDS; i++)
		{
			asteroid::SpawnFromEdge(asteroids);
		}

		const float pauseButtonX = static_cast<float>(config::SCREEN_WIDTH) - PAUSE_BUTTON_WIDTH - PAUSE_BUTTON_MARGIN;
		pauseButton = ui::CreateButton(pauseButtonX, PAUSE_BUTTON_MARGIN, PAUSE_BUTTON_WIDTH, PAUSE_BUTTON_HEIGHT, "PAUSA");

		resumeButton = ui::CreateCenteredButton(RESUME_BUTTON_POSITION_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "CONTINUAR");
		menuButton = ui::CreateCenteredButton(RESUME_BUTTON_POSITION_Y + BUTTON_SPACING, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "MENU");
	}

	game::Screen Update()
	{
		// P o ESC tambien pausan/reanudan (opcional, por teclado)
		const bool pauseKeyPressed = IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE);

		if (isPaused)
		{
			if (ui::IsButtonClicked(menuButton))
			{
				return game::Screen::Menu;
			}

			if (ui::IsButtonClicked(resumeButton) || pauseKeyPressed)
			{
				isPaused = false;
			}
		}
		else
		{
			if (ui::IsButtonClicked(pauseButton) || pauseKeyPressed)
			{
				isPaused = true;
			}
			else
			{
				UpdateMatch();
			}
		}

		return game::Screen::Gameplay;
	}

	void Draw()
	{
		DrawMatch();

		if (isPaused)
		{
			DrawPauseOverlay();
		}
		else
		{
			ui::DrawButton(pauseButton);
		}
	}
}
