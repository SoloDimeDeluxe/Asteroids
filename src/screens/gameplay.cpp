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
		constexpr int TITLE_FONT_SIZE = 60;
		constexpr int TITLE_POSITION_Y = 200;

		// Boton de pausa (arriba a la derecha)
		constexpr float PAUSE_BUTTON_WIDTH = 140.0f;
		constexpr float PAUSE_BUTTON_HEIGHT = 45.0f;
		constexpr float PAUSE_BUTTON_MARGIN = 15.0f;

		// Botones de los menus de pausa y derrota
		constexpr float MENU_BUTTON_WIDTH = 320.0f;
		constexpr float MENU_BUTTON_HEIGHT = 60.0f;
		constexpr float FIRST_BUTTON_POSITION_Y = 380.0f;
		constexpr float BUTTON_SPACING = 90.0f;

		// Pantalla de derrota
		constexpr int FINAL_SCORE_FONT_SIZE = 35;
		constexpr int FINAL_SCORE_POSITION_Y = 290;

		// Vidas y puntaje (HUD)
		constexpr int INITIAL_LIVES = 3;
		constexpr int HUD_FONT_SIZE = 25;
		constexpr int HUD_MARGIN = 15;
		constexpr int HUD_LINE_SPACING = 30;

		// Aparicion de asteroides
		constexpr int INITIAL_ASTEROIDS = 4;
		constexpr float SPAWN_INTERVAL = 5.0f;      // Segundos entre cada asteroide nuevo
		constexpr int MAX_ACTIVE_FOR_SPAWN = 8;     // No aparecen mas si ya hay estos

		bool isPaused = false;
		bool isGameOver = false;
		int lives = INITIAL_LIVES;
		int score = 0;                              // Asteroides destruidos con balas

		ship::Ship playerShip = {};
		bullet::Bullet bullets[bullet::MAX_BULLETS] = {};
		asteroid::Asteroid asteroids[asteroid::MAX_ASTEROIDS] = {};
		float spawnTimer = 0.0f;

		ui::Button pauseButton = {};

		// Menu de pausa
		ui::Button resumeButton = {};
		ui::Button pauseMenuButton = {};

		// Menu de derrota
		ui::Button retryButton = {};
		ui::Button gameOverMenuButton = {};

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

						// Un punto por cada asteroide destruido
						score++;

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
					// El asteroide tambien se rompe con el choque (pero no suma puntos)
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
			// Sin vidas la nave ya no se mueve ni dispara, pero los asteroides siguen de fondo
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
			DrawText(TextFormat("PUNTAJE: %d", score), HUD_MARGIN, HUD_MARGIN + HUD_LINE_SPACING, HUD_FONT_SIZE, RAYWHITE);
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

		// Capa oscura sobre la partida (para pausa y derrota)
		void DrawDarkLayer()
		{
			DrawRectangle(0, 0, config::SCREEN_WIDTH, config::SCREEN_HEIGHT, Fade(BLACK, 0.7f));
		}

		void DrawPauseOverlay()
		{
			DrawDarkLayer();

			ui::DrawCenteredText("PAUSA", TITLE_POSITION_Y, TITLE_FONT_SIZE, RAYWHITE);

			ui::DrawButton(resumeButton);
			ui::DrawButton(pauseMenuButton);
		}

		void DrawGameOverOverlay()
		{
			DrawDarkLayer();

			ui::DrawCenteredText("GAME OVER", TITLE_POSITION_Y, TITLE_FONT_SIZE, RED);
			ui::DrawCenteredText(TextFormat("Asteroides destruidos: %d", score),
				FINAL_SCORE_POSITION_Y, FINAL_SCORE_FONT_SIZE, RAYWHITE);

			ui::DrawButton(retryButton);
			ui::DrawButton(gameOverMenuButton);
		}

		// Menu de derrota: volver a jugar o ir al menu
		game::Screen UpdateGameOver()
		{
			if (ui::IsButtonClicked(gameOverMenuButton) || IsKeyPressed(KEY_ESCAPE))
			{
				return game::Screen::Menu;
			}

			if (ui::IsButtonClicked(retryButton))
			{
				// Reinicia todo: vidas, puntaje, nave y asteroides
				Init();
				return game::Screen::Gameplay;
			}

			// Los asteroides siguen moviendose de fondo
			UpdateMatch();

			return game::Screen::Gameplay;
		}

		// Menu de pausa: continuar o ir al menu
		void UpdatePause(bool pauseKeyPressed, game::Screen& nextScreen)
		{
			if (ui::IsButtonClicked(pauseMenuButton))
			{
				nextScreen = game::Screen::Menu;
				return;
			}

			if (ui::IsButtonClicked(resumeButton) || pauseKeyPressed)
			{
				isPaused = false;
			}
		}
	}

	void Init()
	{
		isPaused = false;
		isGameOver = false;
		lives = INITIAL_LIVES;
		score = 0;

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

		const float secondButtonPositionY = FIRST_BUTTON_POSITION_Y + BUTTON_SPACING;

		resumeButton = ui::CreateCenteredButton(FIRST_BUTTON_POSITION_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "CONTINUAR");
		pauseMenuButton = ui::CreateCenteredButton(secondButtonPositionY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "MENU");

		retryButton = ui::CreateCenteredButton(FIRST_BUTTON_POSITION_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "VOLVER A JUGAR");
		gameOverMenuButton = ui::CreateCenteredButton(secondButtonPositionY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "MENU");
	}

	game::Screen Update()
	{
		if (isGameOver)
		{
			return UpdateGameOver();
		}

		// P o ESC tambien pausan/reanudan (opcional, por teclado)
		const bool pauseKeyPressed = IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE);

		game::Screen nextScreen = game::Screen::Gameplay;

		if (isPaused)
		{
			UpdatePause(pauseKeyPressed, nextScreen);
		}
		else if (ui::IsButtonClicked(pauseButton) || pauseKeyPressed)
		{
			isPaused = true;
		}
		else
		{
			UpdateMatch();
		}

		return nextScreen;
	}

	void Draw()
	{
		DrawMatch();

		if (isGameOver)
		{
			DrawGameOverOverlay();
		}
		else if (isPaused)
		{
			DrawPauseOverlay();
		}
		else
		{
			ui::DrawButton(pauseButton);
		}
	}
}
