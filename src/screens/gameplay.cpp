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

		// Vidas
		constexpr int INITIAL_LIVES = 3;
		constexpr int HUD_FONT_SIZE = 25;
		constexpr int HUD_MARGIN = 15;
		constexpr int GAME_OVER_FONT_SIZE = 60;
		constexpr int GAME_OVER_HINT_FONT_SIZE = 20;

		// Aparicion de asteroides
		constexpr int INITIAL_ASTEROIDS = 4;
		constexpr float SPAWN_INTERVAL = 5.0f;      // Segundos entre cada asteroide nuevo
		constexpr int MAX_ACTIVE_FOR_SPAWN = 8;     // No aparecen mas si ya hay estos

		bool isPaused = false;
		bool isGameOver = false;
		int lives = INITIAL_LIVES;
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

		// Si la nave toca un asteroide pierde una vida (salvo que sea invulnerable)
		void CheckShipAsteroidCollisions()
		{
			if (ship::IsInvulnerable(playerShip))
			{
				return;
			}

			for (int asteroidIndex = 0; asteroidIndex < asteroid::MAX_ASTEROIDS; asteroidIndex++)
			{
				if (!asteroids[asteroidIndex].isActive)
				{
					continue;
				}

				if (collision::AreCirclesColliding(playerShip.position, ship::COLLISION_RADIUS,
					asteroids[asteroidIndex].position, asteroids[asteroidIndex].radius))
				{
					// El asteroide tambien se rompe con el choque
					asteroid::Destroy(asteroids, asteroidIndex);
					lives--;

					if (lives > 0)
					{
						ship::Respawn(playerShip);
					}
					else
					{
						isGameOver = true;
					}

					// Un solo choque por frame
					return;
				}
			}
		}

		// Aca va la logica de la partida (nave, asteroides, balas)
		void UpdateMatch()
		{
			// Sin vidas la nave ya no se mueve ni dispara, pero los asteroides siguen
			if (!isGameOver)
			{
				ship::Update(playerShip);

				if (IsFirePressed())
				{
					bullet::Fire(bullets, ship::GetNosePosition(playerShip), playerShip.rotation);
				}
			}

			bullet::UpdateAll(bullets);

			UpdateAsteroidSpawn();
			asteroid::UpdateAll(asteroids);

			CheckBulletAsteroidCollisions();

			if (!isGameOver)
			{
				CheckShipAsteroidCollisions();
			}
		}

		void DrawHud()
		{
			DrawText(TextFormat("VIDAS: %d", lives), HUD_MARGIN, HUD_MARGIN, HUD_FONT_SIZE, RAYWHITE);
		}

		// Provisorio hasta la pantalla de derrota (v0.24)
		void DrawGameOver()
		{
			const int centerY = config::SCREEN_HEIGHT / 2;

			ui::DrawCenteredText("GAME OVER", centerY - GAME_OVER_FONT_SIZE, GAME_OVER_FONT_SIZE, RED);
			ui::DrawCenteredText("Usa PAUSA > MENU para volver",
				centerY + GAME_OVER_HINT_FONT_SIZE, GAME_OVER_HINT_FONT_SIZE, LIGHTGRAY);
		}

		// Aca se dibuja la partida
		void DrawMatch()
		{
			asteroid::DrawAll(asteroids);
			bullet::DrawAll(bullets);

			if (!isGameOver)
			{
				ship::Draw(playerShip);
			}

			DrawHud();
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
		isGameOver = false;
		lives = INITIAL_LIVES;
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

		if (isGameOver && !isPaused)
		{
			DrawGameOver();
		}

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
