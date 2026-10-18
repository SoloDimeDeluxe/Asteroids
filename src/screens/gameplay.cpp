#include "screens/gameplay.h"

#include "raylib.h"

#include "entities/asteroid.h"
#include "entities/bullet.h"
#include "entities/explosion.h"
#include "entities/powerup.h"
#include "entities/ship.h"
#include "entities/ufo.h"
#include "game/audio.h"
#include "utils/collision.h"
#include "utils/config.h"
#include "utils/ui.h"

namespace ReyesCuadra::gameplay
{
	static constexpr int TITLE_FONT_SIZE = 60;
	static constexpr int TITLE_POSITION_Y = 200;

	// Boton de pausa (arriba a la derecha)
	static constexpr float PAUSE_BUTTON_WIDTH = 140.0f;
	static constexpr float PAUSE_BUTTON_HEIGHT = 45.0f;
	static constexpr float PAUSE_BUTTON_MARGIN = 15.0f;

	// Botones de los menus de pausa y derrota
	static constexpr float MENU_BUTTON_WIDTH = 320.0f;
	static constexpr float MENU_BUTTON_HEIGHT = 60.0f;
	static constexpr float FIRST_BUTTON_POSITION_Y = 380.0f;
	static constexpr float BUTTON_SPACING = 90.0f;

	// Capa oscura de pausa y derrota (0 = transparente, 1 = negro total)
	static constexpr float DARK_LAYER_OPACITY = 0.7f;

	// Pantalla de derrota
	static constexpr int FINAL_SCORE_FONT_SIZE = 35;
	static constexpr int FINAL_SCORE_POSITION_Y = 275;
	static constexpr int FINAL_DETAIL_FONT_SIZE = 22;
	static constexpr int FINAL_DETAIL_POSITION_Y = 322;

	// Vidas y puntaje (HUD)
	static constexpr int INITIAL_LIVES = 3;
	static constexpr int HUD_FONT_SIZE = 25;
	static constexpr int HUD_MARGIN = 15;
	static constexpr int HUD_LINE_SPACING = 30;

	// Ayuda de controles al empezar la partida
	static constexpr float CONTROLS_HINT_DURATION = 6.0f;  // Segundos que se muestra
	static constexpr float CONTROLS_HINT_FADE_TIME = 1.0f; // Ultimo segundo: se desvanece
	static constexpr int CONTROLS_HINT_FONT_SIZE = 20;
	static constexpr int CONTROLS_HINT_BOTTOM_MARGIN = 40;

	// Segundos que se ve la nave destruida antes de mostrar el GAME OVER
	static constexpr float GAME_OVER_DELAY = 2.0f;

	// Power-up de bomba
	static constexpr float POWERUP_SPAWN_INTERVAL = 15.0f; // Segundos sin power-up hasta que aparece uno
	static constexpr float BOMB_FLASH_DURATION = 0.5f;     // Destello blanco al explotar
	static constexpr float BOMB_FLASH_MAX_OPACITY = 0.8f;

	// OVNI enemigo
	static constexpr float UFO_SPAWN_INTERVAL = 20.0f;     // Segundos sin OVNI hasta que aparece uno
	static constexpr int UFO_POINTS = 5;                   // Puntos por destruir un OVNI (un asteroide da 1)

	// Tamaño de las explosiones
	static constexpr float ASTEROID_EXPLOSION_SCALE = 2.4f; // Respecto del radio del asteroide
	static constexpr float UFO_EXPLOSION_SIZE = 90.0f;
	static constexpr float SHIP_EXPLOSION_SIZE = 100.0f;

	// Aparicion de asteroides
	static constexpr int INITIAL_ASTEROIDS = 4;
	static constexpr float SPAWN_INTERVAL = 5.0f;      // Segundos entre cada asteroide nuevo
	static constexpr int MAX_ACTIVE_FOR_SPAWN = 8;     // No aparecen mas si ya hay estos

	static bool isPaused = false;
	static bool isShipDestroyed = false;               // Perdio la ultima vida
	static bool isGameOver = false;                    // Se esta mostrando la pantalla de derrota
	static float gameOverDelayLeft = 0.0f;
	static float controlsHintTimeLeft = 0.0f;
	static int lives = INITIAL_LIVES;
	static int asteroidsDestroyed = 0;                 // Con balas o con la bomba
	static int ufosDestroyed = 0;

	static ship::Ship playerShip = {};
	static bullet::Bullet playerBullets[bullet::MAX_BULLETS] = {};
	static bullet::Bullet enemyBullets[bullet::MAX_BULLETS] = {};
	static asteroid::Asteroid asteroids[asteroid::MAX_ASTEROIDS] = {};
	static float spawnTimer = 0.0f;

	static powerup::PowerUp bombPowerUp = {};
	static float powerUpSpawnTimer = 0.0f;
	static float bombFlashTimeLeft = 0.0f;

	static explosion::Explosion explosions[explosion::MAX_EXPLOSIONS] = {};

	static ufo::Ufo enemyUfo = {};
	static float ufoSpawnTimer = 0.0f;

	static ui::Button pauseButton = {};

	// Menu de pausa
	static ui::Button resumeButton = {};
	static ui::Button pauseMenuButton = {};

	// Menu de derrota
	static ui::Button retryButton = {};
	static ui::Button gameOverMenuButton = {};

	// Cada asteroide da 1 punto y cada OVNI da UFO_POINTS
	static int GetScore()
	{
		return asteroidsDestroyed + ufosDestroyed * UFO_POINTS;
	}

	// Explosion del tamaño del asteroide, en su posicion
	static void ExplodeAsteroid(int asteroidIndex)
	{
		const float size = asteroids[asteroidIndex].radius * ASTEROID_EXPLOSION_SCALE;
		explosion::Spawn(explosions, asteroids[asteroidIndex].position, size);
	}

	// Explosion en la posicion del OVNI, que deja de estar en pantalla
	static void ExplodeUfo()
	{
		explosion::Spawn(explosions, enemyUfo.position, UFO_EXPLOSION_SIZE);
		enemyUfo.isActive = false;
	}

	// Clic izquierdo (o espacio) dispara.
	// Si el mouse esta sobre el boton de pausa no dispara,
	// asi el clic para pausar no tira una bala.
	static bool IsFirePressed()
	{
		const bool clickedToFire = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ui::IsButtonHovered(pauseButton);

		return clickedToFire || IsKeyPressed(KEY_SPACE);
	}

	// Cada SPAWN_INTERVAL segundos aparece un asteroide grande
	static void UpdateAsteroidSpawn()
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
	static void CheckBulletAsteroidCollisions()
	{
		for (int bulletIndex = 0; bulletIndex < bullet::MAX_BULLETS; bulletIndex++)
		{
			if (!playerBullets[bulletIndex].isActive)
			{
				continue;
			}

			for (int asteroidIndex = 0; asteroidIndex < asteroid::MAX_ASTEROIDS; asteroidIndex++)
			{
				if (!asteroids[asteroidIndex].isActive)
				{
					continue;
				}

				if (collision::AreCirclesColliding(playerBullets[bulletIndex].position, bullet::RADIUS,
					asteroids[asteroidIndex].position, asteroids[asteroidIndex].radius))
				{
					playerBullets[bulletIndex].isActive = false;
					ExplodeAsteroid(asteroidIndex);
					asteroid::Destroy(asteroids, asteroidIndex);
					audio::PlaySoundEffect(audio::SoundEffect::AsteroidExplosion);
					asteroidsDestroyed++;

					// Esta bala ya se gasto, pasa a la siguiente
					break;
				}
			}
		}
	}

	// Pierde una vida: reaparece en el centro, o si era la ultima, la nave queda destruida
	static void LoseLife()
	{
		explosion::Spawn(explosions, playerShip.position, SHIP_EXPLOSION_SIZE);
		audio::PlaySoundEffect(audio::SoundEffect::ShipExplosion);
		lives--;

		if (lives > 0)
		{
			ship::Respawn(playerShip);
		}
		else
		{
			// No muestra el GAME OVER de golpe: primero deja ver el choque
			isShipDestroyed = true;
			gameOverDelayLeft = GAME_OVER_DELAY;
		}
	}

	// Si la nave toca un asteroide pierde una vida (salvo que sea invulnerable)
	static void CheckShipAsteroidCollisions()
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
				ExplodeAsteroid(asteroidIndex);
				asteroid::Destroy(asteroids, asteroidIndex);
				LoseLife();

				// Un solo choque por frame
				return;
			}
		}
	}

	// Si no hay OVNI, cada UFO_SPAWN_INTERVAL segundos aparece uno. Si hay, se mueve y dispara
	static void UpdateUfo()
	{
		if (enemyUfo.isActive)
		{
			ufo::Update(enemyUfo);

			// Le dispara a la nave (si sigue viva)
			if (!isShipDestroyed && ufo::ShouldShoot(enemyUfo))
			{
				const float shotDirection = ufo::GetShotDirection(enemyUfo, playerShip.position);

				if (bullet::Fire(enemyBullets, enemyUfo.position, shotDirection, bullet::BulletType::Enemy))
				{
					audio::PlaySoundEffect(audio::SoundEffect::UfoShoot);
				}
			}
		}
		else
		{
			ufoSpawnTimer += GetFrameTime();

			if (ufoSpawnTimer >= UFO_SPAWN_INTERVAL)
			{
				ufoSpawnTimer = 0.0f;
				ufo::Spawn(enemyUfo);
			}
		}

		audio::UpdateUfoHum(enemyUfo.isActive);
	}

	// Si una bala de la nave toca al OVNI, se destruyen los dos
	static void CheckBulletUfoCollisions()
	{
		if (!enemyUfo.isActive)
		{
			return;
		}

		for (int bulletIndex = 0; bulletIndex < bullet::MAX_BULLETS; bulletIndex++)
		{
			if (playerBullets[bulletIndex].isActive &&
				collision::AreCirclesColliding(playerBullets[bulletIndex].position, bullet::RADIUS,
					enemyUfo.position, ufo::COLLISION_RADIUS))
			{
				playerBullets[bulletIndex].isActive = false;
				ExplodeUfo();
				ufosDestroyed++;
				audio::PlaySoundEffect(audio::SoundEffect::AsteroidExplosion);
				return;
			}
		}
	}

	// Si la nave choca con el OVNI o la toca una bala del OVNI, pierde una vida
	static void CheckShipUfoCollisions()
	{
		if (ship::IsInvulnerable(playerShip))
		{
			return;
		}

		// Choque con el OVNI: se destruye tambien (pero no suma puntos)
		if (enemyUfo.isActive &&
			collision::AreCirclesColliding(playerShip.position, ship::COLLISION_RADIUS,
				enemyUfo.position, ufo::COLLISION_RADIUS))
		{
			ExplodeUfo();
			LoseLife();
			return;
		}

		for (int bulletIndex = 0; bulletIndex < bullet::MAX_BULLETS; bulletIndex++)
		{
			if (enemyBullets[bulletIndex].isActive &&
				collision::AreCirclesColliding(playerShip.position, ship::COLLISION_RADIUS,
					enemyBullets[bulletIndex].position, bullet::RADIUS))
			{
				enemyBullets[bulletIndex].isActive = false;
				LoseLife();

				// Un solo golpe por frame
				return;
			}
		}
	}

	// Si no hay power-up en pantalla, cada POWERUP_SPAWN_INTERVAL segundos aparece uno
	static void UpdatePowerUp()
	{
		powerup::Update(bombPowerUp);

		if (bombPowerUp.isActive)
		{
			return;
		}

		powerUpSpawnTimer += GetFrameTime();

		if (powerUpSpawnTimer >= POWERUP_SPAWN_INTERVAL)
		{
			powerUpSpawnTimer = 0.0f;
			powerup::Spawn(bombPowerUp, playerShip.position);
		}
	}

	// Si la nave agarra la bomba, destruye todo lo que hay en pantalla (asteroides, OVNI y sus balas)
	static void CheckShipPowerUpCollision()
	{
		if (!bombPowerUp.isActive)
		{
			return;
		}

		if (collision::AreCirclesColliding(playerShip.position, ship::COLLISION_RADIUS,
			bombPowerUp.position, powerup::COLLISION_RADIUS))
		{
			bombPowerUp.isActive = false;

			// Todos los asteroides explotan a la vez
			for (int asteroidIndex = 0; asteroidIndex < asteroid::MAX_ASTEROIDS; asteroidIndex++)
			{
				if (asteroids[asteroidIndex].isActive)
				{
					ExplodeAsteroid(asteroidIndex);
				}
			}

			asteroidsDestroyed += asteroid::DestroyAll(asteroids);

			if (enemyUfo.isActive)
			{
				ExplodeUfo();
				ufosDestroyed++;
			}

			bullet::InitAll(enemyBullets);
			bombFlashTimeLeft = BOMB_FLASH_DURATION;
			audio::PlaySoundEffect(audio::SoundEffect::Bomb);
		}
	}

	// Aca va la logica de la partida (nave, asteroides, OVNI, balas y power-up)
	static void UpdateMatch()
	{
		// Sin vidas la nave ya no se mueve ni dispara, pero los asteroides siguen de fondo
		if (!isShipDestroyed)
		{
			ship::Update(playerShip);

			if (IsFirePressed())
			{
				// Solo suena si de verdad salio una bala
				if (bullet::Fire(playerBullets, ship::GetNosePosition(playerShip), playerShip.rotation, bullet::BulletType::Player))
				{
					audio::PlaySoundEffect(audio::SoundEffect::Shoot);
				}
			}
		}

		// El motor suena solo mientras la nave (viva) acelera
		audio::UpdateThrust(!isShipDestroyed && playerShip.isAccelerating);

		bullet::UpdateAll(playerBullets);
		bullet::UpdateAll(enemyBullets);

		UpdateAsteroidSpawn();
		asteroid::UpdateAll(asteroids);
		UpdateUfo();

		CheckBulletAsteroidCollisions();
		CheckBulletUfoCollisions();

		// Por separado: si el primer choque fue el ultimo, la nave ya no puede volver a chocar
		if (!isShipDestroyed)
		{
			CheckShipAsteroidCollisions();
		}

		if (!isShipDestroyed)
		{
			CheckShipUfoCollisions();
		}

		UpdatePowerUp();

		if (!isShipDestroyed)
		{
			CheckShipPowerUpCollision();
		}

		explosion::UpdateAll(explosions);

		if (bombFlashTimeLeft > 0.0f)
		{
			bombFlashTimeLeft -= GetFrameTime();
		}

		if (controlsHintTimeLeft > 0.0f)
		{
			controlsHintTimeLeft -= GetFrameTime();
		}
	}

	// Despues del ultimo choque: espera unos segundos y muestra el GAME OVER
	static void UpdateShipDestroyed()
	{
		UpdateMatch();

		gameOverDelayLeft -= GetFrameTime();

		if (gameOverDelayLeft <= 0.0f)
		{
			isGameOver = true;
			audio::PlaySoundEffect(audio::SoundEffect::GameOver);
		}
	}

	static void DrawHud()
	{
		DrawText(TextFormat("VIDAS: %d", lives), HUD_MARGIN, HUD_MARGIN, HUD_FONT_SIZE, RAYWHITE);
		DrawText(TextFormat("PUNTAJE: %d", GetScore()), HUD_MARGIN, HUD_MARGIN + HUD_LINE_SPACING, HUD_FONT_SIZE, RAYWHITE);
	}

	// Controles abajo de la pantalla durante los primeros segundos
	static void DrawControlsHint()
	{
		if (controlsHintTimeLeft <= 0.0f)
		{
			return;
		}

		// Opaco hasta el ultimo segundo, ahi se va desvaneciendo
		float opacity = controlsHintTimeLeft / CONTROLS_HINT_FADE_TIME;

		if (opacity > 1.0f)
		{
			opacity = 1.0f;
		}

		const int positionY = config::SCREEN_HEIGHT - CONTROLS_HINT_BOTTOM_MARGIN;

		ui::DrawCenteredText("Mouse: apuntar  -  Clic derecho: acelerar  -  Clic izquierdo: disparar",
			positionY, CONTROLS_HINT_FONT_SIZE, Fade(LIGHTGRAY, opacity));
	}

	// Destello blanco que se desvanece al explotar la bomba
	static void DrawBombFlash()
	{
		if (bombFlashTimeLeft <= 0.0f)
		{
			return;
		}

		const float opacity = bombFlashTimeLeft / BOMB_FLASH_DURATION * BOMB_FLASH_MAX_OPACITY;
		DrawRectangle(0, 0, config::SCREEN_WIDTH, config::SCREEN_HEIGHT, Fade(WHITE, opacity));
	}

	// Aca se dibuja la partida
	static void DrawMatch()
	{
		asteroid::DrawAll(asteroids);
		powerup::Draw(bombPowerUp);
		ufo::Draw(enemyUfo);
		bullet::DrawAll(enemyBullets);
		bullet::DrawAll(playerBullets);

		if (isShipDestroyed)
		{
			ship::DrawDestroyed(playerShip);
		}
		else
		{
			ship::Draw(playerShip);
		}

		explosion::DrawAll(explosions);
		DrawBombFlash();
		DrawHud();
		DrawControlsHint();
	}

	// Capa oscura sobre la partida (para pausa y derrota)
	static void DrawDarkLayer()
	{
		DrawRectangle(0, 0, config::SCREEN_WIDTH, config::SCREEN_HEIGHT, Fade(BLACK, DARK_LAYER_OPACITY));
	}

	static void DrawPauseOverlay()
	{
		DrawDarkLayer();

		ui::DrawCenteredText("PAUSA", TITLE_POSITION_Y, TITLE_FONT_SIZE, RAYWHITE);

		ui::DrawButton(resumeButton);
		ui::DrawButton(pauseMenuButton);
	}

	static void DrawGameOverOverlay()
	{
		DrawDarkLayer();

		ui::DrawCenteredText("GAME OVER", TITLE_POSITION_Y, TITLE_FONT_SIZE, RED);
		ui::DrawCenteredText(TextFormat("PUNTAJE: %d", GetScore()),
			FINAL_SCORE_POSITION_Y, FINAL_SCORE_FONT_SIZE, RAYWHITE);
		ui::DrawCenteredText(TextFormat("Asteroides destruidos: %d   -   OVNIs destruidos: %d", asteroidsDestroyed, ufosDestroyed),
			FINAL_DETAIL_POSITION_Y, FINAL_DETAIL_FONT_SIZE, LIGHTGRAY);

		ui::DrawButton(retryButton);
		ui::DrawButton(gameOverMenuButton);
	}

	// Menu de derrota: volver a jugar o ir al menu
	static game::Screen UpdateGameOver()
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
	static void UpdatePause(bool pauseKeyPressed, game::Screen& nextScreen)
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

	void Init()
	{
		isPaused = false;
		isShipDestroyed = false;
		isGameOver = false;
		gameOverDelayLeft = 0.0f;
		controlsHintTimeLeft = CONTROLS_HINT_DURATION;
		lives = INITIAL_LIVES;
		asteroidsDestroyed = 0;
		ufosDestroyed = 0;

		// Arranca en el centro e invulnerable (parpadeando) unos segundos,
		// para que el jugador se acostumbre a los controles
		ship::Respawn(playerShip);
		bullet::InitAll(playerBullets);
		bullet::InitAll(enemyBullets);
		explosion::InitAll(explosions);

		asteroid::InitAll(asteroids);
		spawnTimer = 0.0f;

		bombPowerUp = powerup::Create();
		powerUpSpawnTimer = 0.0f;
		bombFlashTimeLeft = 0.0f;

		enemyUfo = ufo::Create();
		ufoSpawnTimer = 0.0f;

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

		// Mientras se ve la nave destruida no se puede pausar
		if (isShipDestroyed)
		{
			UpdateShipDestroyed();
			return game::Screen::Gameplay;
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

			// En pausa el motor y el OVNI no suenan
			audio::UpdateThrust(false);
			audio::UpdateUfoHum(false);
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
		else if (!isShipDestroyed)
		{
			ui::DrawButton(pauseButton);
		}
	}
}
