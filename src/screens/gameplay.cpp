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

static const int TITLE_FONT_SIZE = 60;
static const int TITLE_POSITION_Y = 200;

// Boton de pausa (arriba a la derecha)
static const float PAUSE_BUTTON_WIDTH = 140.0f;
static const float PAUSE_BUTTON_HEIGHT = 45.0f;
static const float PAUSE_BUTTON_MARGIN = 15.0f;

// Botones de los menus de pausa y derrota
static const float MENU_BUTTON_WIDTH = 320.0f;
static const float MENU_BUTTON_HEIGHT = 60.0f;
static const float FIRST_BUTTON_POSITION_Y = 380.0f;
static const float BUTTON_SPACING = 90.0f;

// Capa oscura de pausa y derrota (0 = transparente, 1 = negro total)
static const float DARK_LAYER_OPACITY = 0.7f;

// Pantalla de derrota
static const int FINAL_SCORE_FONT_SIZE = 35;
static const int FINAL_SCORE_POSITION_Y = 275;
static const int FINAL_DETAIL_FONT_SIZE = 22;
static const int FINAL_DETAIL_POSITION_Y = 322;

// Vidas y puntaje (HUD)
static const int INITIAL_LIVES = 3;
static const int HUD_FONT_SIZE = 25;
static const int HUD_MARGIN = 15;
static const int HUD_LINE_SPACING = 30;

// Ayuda de controles al empezar la partida
static const float CONTROLS_HINT_DURATION = 6.0f;  // Segundos que se muestra
static const float CONTROLS_HINT_FADE_TIME = 1.0f; // Ultimo segundo: se desvanece
static const int CONTROLS_HINT_FONT_SIZE = 20;
static const int CONTROLS_HINT_BOTTOM_MARGIN = 40;

// Segundos que se ve la nave destruida antes de mostrar el GAME OVER
static const float GAME_OVER_DELAY = 2.0f;

// Power-up de bomba
static const float POWERUP_SPAWN_INTERVAL = 15.0f; // Segundos sin power-up hasta que aparece uno
static const float BOMB_FLASH_DURATION = 0.5f;     // Destello blanco al explotar
static const float BOMB_FLASH_MAX_OPACITY = 0.8f;

// OVNI enemigo
static const float UFO_SPAWN_INTERVAL = 20.0f;     // Segundos sin OVNI hasta que aparece uno
static const int UFO_POINTS = 5;                   // Puntos por destruir un OVNI (un asteroide da 1)

// Tamaño de las explosiones
static const float ASTEROID_EXPLOSION_SCALE = 2.4f; // Respecto del radio del asteroide
static const float UFO_EXPLOSION_SIZE = 90.0f;
static const float SHIP_EXPLOSION_SIZE = 100.0f;

// Aparicion de asteroides
static const int INITIAL_ASTEROIDS = 4;
static const float SPAWN_INTERVAL = 5.0f;      // Segundos entre cada asteroide nuevo
static const int MAX_ACTIVE_FOR_SPAWN = 8;     // No aparecen mas si ya hay estos

static bool isPaused = false;
static bool isShipDestroyed = false;               // Perdio la ultima vida
static bool isGameOver = false;                    // Se esta mostrando la pantalla de derrota
static float gameOverDelayLeft = 0.0f;
static float controlsHintTimeLeft = 0.0f;
static int lives = INITIAL_LIVES;
static int asteroidsDestroyed = 0;                 // Con balas o con la bomba
static int ufosDestroyed = 0;

static Ship playerShip = {};
static Bullet playerBullets[MAX_BULLETS] = {};
static Bullet enemyBullets[MAX_BULLETS] = {};
static Asteroid asteroids[MAX_ASTEROIDS] = {};
static float spawnTimer = 0.0f;

static PowerUp bombPowerUp = {};
static float powerUpSpawnTimer = 0.0f;
static float bombFlashTimeLeft = 0.0f;

static Explosion explosions[MAX_EXPLOSIONS] = {};

static Ufo enemyUfo = {};
static float ufoSpawnTimer = 0.0f;

static Button pauseButton = {};

// Menu de pausa
static Button resumeButton = {};
static Button pauseMenuButton = {};

// Menu de derrota
static Button retryButton = {};
static Button gameOverMenuButton = {};

// Cada asteroide da 1 punto y cada OVNI da UFO_POINTS
static int GetScore()
{
	return asteroidsDestroyed + ufosDestroyed * UFO_POINTS;
}

// Explosion del tamaño del asteroide, en su posicion
static void ExplodeAsteroid(int asteroidIndex)
{
	const float size = asteroids[asteroidIndex].radius * ASTEROID_EXPLOSION_SCALE;
	SpawnExplosion(explosions, asteroids[asteroidIndex].position, size);
}

// Explosion en la posicion del OVNI, que deja de estar en pantalla
static void ExplodeUfo()
{
	SpawnExplosion(explosions, enemyUfo.position, UFO_EXPLOSION_SIZE);
	enemyUfo.isActive = false;
}

// Clic izquierdo (o espacio) dispara.
// Si el mouse esta sobre el boton de pausa no dispara,
// asi el clic para pausar no tira una bala.
static bool IsFirePressed()
{
	const bool clickedToFire = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsButtonHovered(pauseButton);

	return clickedToFire || IsKeyPressed(KEY_SPACE);
}

// Cada SPAWN_INTERVAL segundos aparece un asteroide grande
static void UpdateAsteroidSpawn()
{
	spawnTimer += GetFrameTime();

	if (spawnTimer >= SPAWN_INTERVAL)
	{
		spawnTimer = 0.0f;

		if (CountActiveAsteroids(asteroids) < MAX_ACTIVE_FOR_SPAWN)
		{
			SpawnAsteroidFromEdge(asteroids);
		}
	}
}

// Si una bala toca un asteroide, se apagan los dos (y el asteroide se divide)
static void CheckBulletAsteroidCollisions()
{
	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		if (!playerBullets[bulletIndex].isActive)
		{
			continue;
		}

		for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
		{
			if (!asteroids[asteroidIndex].isActive)
			{
				continue;
			}

			if (AreCirclesColliding(playerBullets[bulletIndex].position, BULLET_RADIUS,
				asteroids[asteroidIndex].position, asteroids[asteroidIndex].radius))
			{
				playerBullets[bulletIndex].isActive = false;
				ExplodeAsteroid(asteroidIndex);
				DestroyAsteroid(asteroids, asteroidIndex);
				PlaySoundEffect(SoundEffect::AsteroidExplosion);
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
	SpawnExplosion(explosions, playerShip.position, SHIP_EXPLOSION_SIZE);
	PlaySoundEffect(SoundEffect::ShipExplosion);
	lives--;

	if (lives > 0)
	{
		RespawnShip(playerShip);
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
	if (IsShipInvulnerable(playerShip))
	{
		return;
	}

	for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
	{
		if (!asteroids[asteroidIndex].isActive)
		{
			continue;
		}

		if (AreCirclesColliding(playerShip.position, SHIP_COLLISION_RADIUS,
			asteroids[asteroidIndex].position, asteroids[asteroidIndex].radius))
		{
			// El asteroide tambien se rompe con el choque (pero no suma puntos)
			ExplodeAsteroid(asteroidIndex);
			DestroyAsteroid(asteroids, asteroidIndex);
			LoseLife();

			// Un solo choque por frame
			return;
		}
	}
}

// Si no hay OVNI, cada UFO_SPAWN_INTERVAL segundos aparece uno. Si hay, se mueve y dispara
static void UpdateEnemyUfo()
{
	if (enemyUfo.isActive)
	{
		UpdateUfo(enemyUfo);

		// Le dispara a la nave (si sigue viva)
		if (!isShipDestroyed && ShouldUfoShoot(enemyUfo))
		{
			const float shotDirection = GetUfoShotDirection(enemyUfo, playerShip.position);

			if (FireBullet(enemyBullets, enemyUfo.position, shotDirection, BulletType::Enemy))
			{
				PlaySoundEffect(SoundEffect::UfoShoot);
			}
		}
	}
	else
	{
		ufoSpawnTimer += GetFrameTime();

		if (ufoSpawnTimer >= UFO_SPAWN_INTERVAL)
		{
			ufoSpawnTimer = 0.0f;
			SpawnUfo(enemyUfo);
		}
	}

	UpdateUfoHumSound(enemyUfo.isActive);
}

// Si una bala de la nave toca al OVNI, se destruyen los dos
static void CheckBulletUfoCollisions()
{
	if (!enemyUfo.isActive)
	{
		return;
	}

	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		if (playerBullets[bulletIndex].isActive &&
			AreCirclesColliding(playerBullets[bulletIndex].position, BULLET_RADIUS,
				enemyUfo.position, UFO_COLLISION_RADIUS))
		{
			playerBullets[bulletIndex].isActive = false;
			ExplodeUfo();
			ufosDestroyed++;
			PlaySoundEffect(SoundEffect::AsteroidExplosion);
			return;
		}
	}
}

// Si la nave choca con el OVNI o la toca una bala del OVNI, pierde una vida
static void CheckShipUfoCollisions()
{
	if (IsShipInvulnerable(playerShip))
	{
		return;
	}

	// Choque con el OVNI: se destruye tambien (pero no suma puntos)
	if (enemyUfo.isActive &&
		AreCirclesColliding(playerShip.position, SHIP_COLLISION_RADIUS,
			enemyUfo.position, UFO_COLLISION_RADIUS))
	{
		ExplodeUfo();
		LoseLife();
		return;
	}

	for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++)
	{
		if (enemyBullets[bulletIndex].isActive &&
			AreCirclesColliding(playerShip.position, SHIP_COLLISION_RADIUS,
				enemyBullets[bulletIndex].position, BULLET_RADIUS))
		{
			enemyBullets[bulletIndex].isActive = false;
			LoseLife();

			// Un solo golpe por frame
			return;
		}
	}
}

// Si no hay power-up en pantalla, cada POWERUP_SPAWN_INTERVAL segundos aparece uno
static void UpdateBombPowerUp()
{
	UpdatePowerUp(bombPowerUp);

	if (bombPowerUp.isActive)
	{
		return;
	}

	powerUpSpawnTimer += GetFrameTime();

	if (powerUpSpawnTimer >= POWERUP_SPAWN_INTERVAL)
	{
		powerUpSpawnTimer = 0.0f;
		SpawnPowerUp(bombPowerUp, playerShip.position);
	}
}

// Si la nave agarra la bomba, destruye todo lo que hay en pantalla (asteroides, OVNI y sus balas)
static void CheckShipPowerUpCollision()
{
	if (!bombPowerUp.isActive)
	{
		return;
	}

	if (AreCirclesColliding(playerShip.position, SHIP_COLLISION_RADIUS,
		bombPowerUp.position, POWERUP_COLLISION_RADIUS))
	{
		bombPowerUp.isActive = false;

		// Todos los asteroides explotan a la vez
		for (int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++)
		{
			if (asteroids[asteroidIndex].isActive)
			{
				ExplodeAsteroid(asteroidIndex);
			}
		}

		asteroidsDestroyed += DestroyAllAsteroids(asteroids);

		if (enemyUfo.isActive)
		{
			ExplodeUfo();
			ufosDestroyed++;
		}

		InitBullets(enemyBullets);
		bombFlashTimeLeft = BOMB_FLASH_DURATION;
		PlaySoundEffect(SoundEffect::Bomb);
	}
}

// Aca va la logica de la partida (nave, asteroides, OVNI, balas y power-up)
static void UpdateMatch()
{
	// Sin vidas la nave ya no se mueve ni dispara, pero los asteroides siguen de fondo
	if (!isShipDestroyed)
	{
		UpdateShip(playerShip);

		if (IsFirePressed())
		{
			// Solo suena si de verdad salio una bala
			if (FireBullet(playerBullets, GetShipNosePosition(playerShip), playerShip.rotation, BulletType::Player))
			{
				PlaySoundEffect(SoundEffect::Shoot);
			}
		}
	}

	// El motor suena solo mientras la nave (viva) acelera
	UpdateThrustSound(!isShipDestroyed && playerShip.isAccelerating);

	UpdateBullets(playerBullets);
	UpdateBullets(enemyBullets);

	UpdateAsteroidSpawn();
	UpdateAsteroids(asteroids);
	UpdateEnemyUfo();

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

	UpdateBombPowerUp();

	if (!isShipDestroyed)
	{
		CheckShipPowerUpCollision();
	}

	UpdateExplosions(explosions);

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
		PlaySoundEffect(SoundEffect::GameOver);
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

	const int positionY = SCREEN_HEIGHT - CONTROLS_HINT_BOTTOM_MARGIN;

	DrawCenteredText("Mouse: apuntar  -  Clic derecho: acelerar  -  Clic izquierdo: disparar",
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
	DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, opacity));
}

// Aca se dibuja la partida
static void DrawMatch()
{
	DrawAsteroids(asteroids);
	DrawPowerUp(bombPowerUp);
	DrawUfo(enemyUfo);
	DrawBullets(enemyBullets);
	DrawBullets(playerBullets);

	if (isShipDestroyed)
	{
		DrawDestroyedShip(playerShip);
	}
	else
	{
		DrawShip(playerShip);
	}

	DrawExplosions(explosions);
	DrawBombFlash();
	DrawHud();
	DrawControlsHint();
}

// Capa oscura sobre la partida (para pausa y derrota)
static void DrawDarkLayer()
{
	DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, DARK_LAYER_OPACITY));
}

static void DrawPauseOverlay()
{
	DrawDarkLayer();

	DrawCenteredText("PAUSA", TITLE_POSITION_Y, TITLE_FONT_SIZE, RAYWHITE);

	DrawButton(resumeButton);
	DrawButton(pauseMenuButton);
}

static void DrawGameOverOverlay()
{
	DrawDarkLayer();

	DrawCenteredText("GAME OVER", TITLE_POSITION_Y, TITLE_FONT_SIZE, RED);
	DrawCenteredText(TextFormat("PUNTAJE: %d", GetScore()),
		FINAL_SCORE_POSITION_Y, FINAL_SCORE_FONT_SIZE, RAYWHITE);
	DrawCenteredText(TextFormat("Asteroides destruidos: %d   -   OVNIs destruidos: %d", asteroidsDestroyed, ufosDestroyed),
		FINAL_DETAIL_POSITION_Y, FINAL_DETAIL_FONT_SIZE, LIGHTGRAY);

	DrawButton(retryButton);
	DrawButton(gameOverMenuButton);
}

// Menu de derrota: volver a jugar o ir al menu
static Screen UpdateGameOver()
{
	if (IsButtonClicked(gameOverMenuButton) || IsKeyPressed(KEY_ESCAPE))
	{
		return Screen::Menu;
	}

	if (IsButtonClicked(retryButton))
	{
		// Reinicia todo: vidas, puntaje, nave y asteroides
		InitGameplay();
		return Screen::Gameplay;
	}

	// Los asteroides siguen moviendose de fondo
	UpdateMatch();

	return Screen::Gameplay;
}

// Menu de pausa: continuar o ir al menu
static void UpdatePause(bool pauseKeyPressed, Screen& nextScreen)
{
	if (IsButtonClicked(pauseMenuButton))
	{
		nextScreen = Screen::Menu;
		return;
	}

	if (IsButtonClicked(resumeButton) || pauseKeyPressed)
	{
		isPaused = false;
	}
}

void InitGameplay()
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
	RespawnShip(playerShip);
	InitBullets(playerBullets);
	InitBullets(enemyBullets);
	InitExplosions(explosions);

	InitAsteroids(asteroids);
	spawnTimer = 0.0f;

	bombPowerUp = CreatePowerUp();
	powerUpSpawnTimer = 0.0f;
	bombFlashTimeLeft = 0.0f;

	enemyUfo = CreateUfo();
	ufoSpawnTimer = 0.0f;

	for (int i = 0; i < INITIAL_ASTEROIDS; i++)
	{
		SpawnAsteroidFromEdge(asteroids);
	}

	const float pauseButtonX = static_cast<float>(SCREEN_WIDTH) - PAUSE_BUTTON_WIDTH - PAUSE_BUTTON_MARGIN;
	pauseButton = CreateButton(pauseButtonX, PAUSE_BUTTON_MARGIN, PAUSE_BUTTON_WIDTH, PAUSE_BUTTON_HEIGHT, "PAUSA");

	const float secondButtonPositionY = FIRST_BUTTON_POSITION_Y + BUTTON_SPACING;

	resumeButton = CreateCenteredButton(FIRST_BUTTON_POSITION_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "CONTINUAR");
	pauseMenuButton = CreateCenteredButton(secondButtonPositionY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "MENU");

	retryButton = CreateCenteredButton(FIRST_BUTTON_POSITION_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "VOLVER A JUGAR");
	gameOverMenuButton = CreateCenteredButton(secondButtonPositionY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "MENU");
}

Screen UpdateGameplay()
{
	if (isGameOver)
	{
		return UpdateGameOver();
	}

	// Mientras se ve la nave destruida no se puede pausar
	if (isShipDestroyed)
	{
		UpdateShipDestroyed();
		return Screen::Gameplay;
	}

	// P o ESC tambien pausan/reanudan (opcional, por teclado)
	const bool pauseKeyPressed = IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE);

	Screen nextScreen = Screen::Gameplay;

	if (isPaused)
	{
		UpdatePause(pauseKeyPressed, nextScreen);
	}
	else if (IsButtonClicked(pauseButton) || pauseKeyPressed)
	{
		isPaused = true;

		// En pausa el motor y el OVNI no suenan
		UpdateThrustSound(false);
		UpdateUfoHumSound(false);
	}
	else
	{
		UpdateMatch();
	}

	return nextScreen;
}

void DrawGameplay()
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
		DrawButton(pauseButton);
	}
}
