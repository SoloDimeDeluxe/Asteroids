#include "entities/ship.h"

#include <cmath>

#include "utils/config.h"
#include "utils/screen_wrap.h"

// Sprites
static const char* const SHIP_TEXTURE_PATH = "res/textures/ship.png";
static const char* const FLAME_TEXTURE_PATH = "res/textures/ship_flame.png";

// Tamaños en pantalla (la imagen se escala a esto)
static const float SHIP_DRAW_SIZE = 48.0f;    // Ancho y alto del sprite
static const float FLAME_DRAW_WIDTH = 24.0f;
static const float FLAME_DRAW_HEIGHT = 12.0f;

// El fuego es una hoja de sprites con cuadros uno al lado del otro
static const int FLAME_FRAME_COUNT = 4;
static const float FLAME_FRAME_TIME = 0.05f;    // Segundos por cuadro

// Medidas desde el centro de la nave (segun el dibujo del sprite)
static const float NOSE_LENGTH = 22.0f;       // Del centro a la punta (de ahi salen las balas)
static const float TAIL_LENGTH = 17.0f;       // Del centro al motor (ahi empieza el fuego)
static const float WRAP_MARGIN = TAIL_LENGTH + FLAME_DRAW_WIDTH; // Lo mas lejos que se dibuja algo

// Movimiento
static const float ACCELERATION = 300.0f; // Pixeles por segundo, cada segundo
static const float MAX_SPEED = 450.0f;    // Pixeles por segundo

// Invulnerabilidad al reaparecer
static const float RESPAWN_INVULNERABLE_TIME = 3.0f; // Segundos
static const float BLINK_INTERVAL = 0.15f;           // Segundos entre parpadeos

static const Color DESTROYED_COLOR = RED;

static Texture2D shipTexture = {};
static Texture2D flameTexture = {};

// Rectangulo con la imagen entera (para texturas de un solo cuadro)
static Rectangle GetFullSource(const Texture2D& texture)
{
	return { 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) };
}

// Dibuja la parte "source" de una textura, rotada alrededor de "origin" y con ese punto ubicado en "position".
// "tint" tiñe la imagen (WHITE = colores originales)
static void DrawRotatedTexture(const Texture2D& texture, Rectangle source, Vector2 position, float width, float height,
	Vector2 origin, float rotationRadians, Color tint)
{
	const Rectangle destination = { position.x, position.y, width, height };

	// DrawTexturePro usa grados, nuestra rotacion esta en radianes
	DrawTexturePro(texture, source, destination, origin, rotationRadians * RAD2DEG, tint);
}

// Rota un punto relativo a la nave y lo pasa a coordenadas de pantalla
static Vector2 ToScreen(const Ship& ship, float localX, float localY)
{
	const float cosine = std::cos(ship.rotation);
	const float sine = std::sin(ship.rotation);

	Vector2 point = {};
	point.x = ship.position.x + localX * cosine - localY * sine;
	point.y = ship.position.y + localX * sine + localY * cosine;

	return point;
}

static void RotateTowardMouse(Ship& ship)
{
	// Vector de la nave al mouse
	const Vector2 mousePosition = GetMousePosition();
	const float distanceX = mousePosition.x - ship.position.x;
	const float distanceY = mousePosition.y - ship.position.y;

	// atan2 da el angulo de ese vector
	ship.rotation = std::atan2(distanceY, distanceX);
}

static void Accelerate(Ship& ship, float deltaTime)
{
	// Direccion de la nave (vector de largo 1)
	const float directionX = std::cos(ship.rotation);
	const float directionY = std::sin(ship.rotation);

	// La aceleracion se suma a la velocidad
	ship.velocity.x += directionX * ACCELERATION * deltaTime;
	ship.velocity.y += directionY * ACCELERATION * deltaTime;

	// Si supera la velocidad maxima, se achica sin cambiar la direccion
	const float speed = std::sqrt(ship.velocity.x * ship.velocity.x + ship.velocity.y * ship.velocity.y);

	if (speed > MAX_SPEED)
	{
		ship.velocity.x = ship.velocity.x / speed * MAX_SPEED;
		ship.velocity.y = ship.velocity.y / speed * MAX_SPEED;
	}
}

void LoadShipTextures()
{
	shipTexture = LoadTexture(SHIP_TEXTURE_PATH);
	flameTexture = LoadTexture(FLAME_TEXTURE_PATH);
}

Ship CreateShip()
{
	Ship ship = {};

	ship.position.x = static_cast<float>(SCREEN_WIDTH) / 2.0f;
	ship.position.y = static_cast<float>(SCREEN_HEIGHT) / 2.0f;
	ship.velocity = { 0.0f, 0.0f };
	ship.rotation = 0.0f;
	ship.isAccelerating = false;
	ship.flameAnimationTime = 0.0f;
	ship.invulnerableTimeLeft = 0.0f;

	return ship;
}

void UpdateShip(Ship& ship)
{
	// Tiempo del ultimo frame: el movimiento no depende de los FPS
	const float deltaTime = GetFrameTime();

	RotateTowardMouse(ship);

	// Clic derecho mantenido: acelera hacia donde mira
	ship.isAccelerating = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

	if (ship.isAccelerating)
	{
		Accelerate(ship, deltaTime);
		ship.flameAnimationTime += deltaTime;
	}

	// Gravedad cero: nunca frena sola, sigue con la misma velocidad
	ship.position.x += ship.velocity.x * deltaTime;
	ship.position.y += ship.velocity.y * deltaTime;

	// Si sale por un borde, aparece por el contrario
	WrapPosition(ship.position, WRAP_MARGIN);

	// Descuenta el tiempo de invulnerabilidad
	if (ship.invulnerableTimeLeft > 0.0f)
	{
		ship.invulnerableTimeLeft -= deltaTime;
	}
}

void DrawShip(const Ship& ship)
{
	// Mientras es invulnerable parpadea: se dibuja un intervalo si y otro no
	if (IsShipInvulnerable(ship))
	{
		const int blinkStep = static_cast<int>(ship.invulnerableTimeLeft / BLINK_INTERVAL);

		if (blinkStep % 2 == 1)
		{
			return;
		}
	}

	// Fuego del motor (atras de la nave), solo mientras acelera.
	// Su pivote queda a la derecha de la imagen, en el centro de la nave,
	// asi gira junto con ella y siempre queda pegado al motor.
	if (ship.isAccelerating)
	{
		// Cuadro de la animacion segun el tiempo acelerando (vuelve a empezar al terminar)
		const int frame = static_cast<int>(ship.flameAnimationTime / FLAME_FRAME_TIME) % FLAME_FRAME_COUNT;
		const float frameWidth = static_cast<float>(flameTexture.width) / static_cast<float>(FLAME_FRAME_COUNT);
		const Rectangle flameSource = { frameWidth * static_cast<float>(frame), 0.0f, frameWidth, static_cast<float>(flameTexture.height) };

		const Vector2 flameOrigin = { TAIL_LENGTH + FLAME_DRAW_WIDTH, FLAME_DRAW_HEIGHT / 2.0f };
		DrawRotatedTexture(flameTexture, flameSource, ship.position, FLAME_DRAW_WIDTH, FLAME_DRAW_HEIGHT, flameOrigin, ship.rotation, WHITE);
	}

	// Nave: pivote en el centro del sprite (como el ejemplo de DrawRectanglePro)
	const Vector2 shipOrigin = { SHIP_DRAW_SIZE / 2.0f, SHIP_DRAW_SIZE / 2.0f };
	DrawRotatedTexture(shipTexture, GetFullSource(shipTexture), ship.position, SHIP_DRAW_SIZE, SHIP_DRAW_SIZE, shipOrigin, ship.rotation, WHITE);
}

void DrawDestroyedShip(const Ship& ship)
{
	// Sin fuego ni parpadeo, solo la nave teñida de rojo
	const Vector2 shipOrigin = { SHIP_DRAW_SIZE / 2.0f, SHIP_DRAW_SIZE / 2.0f };
	DrawRotatedTexture(shipTexture, GetFullSource(shipTexture), ship.position, SHIP_DRAW_SIZE, SHIP_DRAW_SIZE, shipOrigin, ship.rotation, DESTROYED_COLOR);
}

Vector2 GetShipNosePosition(const Ship& ship)
{
	return ToScreen(ship, NOSE_LENGTH, 0.0f);
}

void RespawnShip(Ship& ship)
{
	ship = CreateShip();
	ship.invulnerableTimeLeft = RESPAWN_INVULNERABLE_TIME;
}

bool IsShipInvulnerable(const Ship& ship)
{
	return ship.invulnerableTimeLeft > 0.0f;
}

void UnloadShipTextures()
{
	UnloadTexture(shipTexture);
	UnloadTexture(flameTexture);
}
