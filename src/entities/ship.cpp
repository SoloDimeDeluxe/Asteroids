#include "entities/ship.h"

#include <cmath>

#include "utils/config.h"
#include "utils/screen_wrap.h"

namespace ReyesCuadra::ship
{
	namespace
	{
		// Sprites
		constexpr const char* SHIP_TEXTURE_PATH = "res/textures/ship.png";
		constexpr const char* FLAME_TEXTURE_PATH = "res/textures/ship_flame.png";

		// Tamaños en pantalla (la imagen se escala a esto)
		constexpr float SHIP_DRAW_SIZE = 48.0f;    // Ancho y alto del sprite
		constexpr float FLAME_DRAW_WIDTH = 24.0f;
		constexpr float FLAME_DRAW_HEIGHT = 12.0f;

		// Medidas desde el centro de la nave (segun el dibujo del sprite)
		constexpr float NOSE_LENGTH = 22.0f;       // Del centro a la punta (de ahi salen las balas)
		constexpr float TAIL_LENGTH = 17.0f;       // Del centro al motor (ahi empieza el fuego)
		constexpr float WRAP_MARGIN = TAIL_LENGTH + FLAME_DRAW_WIDTH; // Lo mas lejos que se dibuja algo

		// Movimiento
		constexpr float ACCELERATION = 300.0f; // Pixeles por segundo, cada segundo
		constexpr float MAX_SPEED = 450.0f;    // Pixeles por segundo

		// Invulnerabilidad al reaparecer
		constexpr float RESPAWN_INVULNERABLE_TIME = 3.0f; // Segundos
		constexpr float BLINK_INTERVAL = 0.15f;           // Segundos entre parpadeos

		Texture2D shipTexture = {};
		Texture2D flameTexture = {};

		// Dibuja una textura entera, rotada alrededor de "origin" y con ese punto ubicado en "position"
		void DrawRotatedTexture(const Texture2D& texture, Vector2 position, float width, float height,
			Vector2 origin, float rotationRadians)
		{
			const Rectangle source = { 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) };
			const Rectangle destination = { position.x, position.y, width, height };

			// DrawTexturePro usa grados, nuestra rotacion esta en radianes
			DrawTexturePro(texture, source, destination, origin, rotationRadians * RAD2DEG, WHITE);
		}

		// Rota un punto relativo a la nave y lo pasa a coordenadas de pantalla
		Vector2 ToScreen(const Ship& ship, float localX, float localY)
		{
			const float cosine = std::cos(ship.rotation);
			const float sine = std::sin(ship.rotation);

			Vector2 point = {};
			point.x = ship.position.x + localX * cosine - localY * sine;
			point.y = ship.position.y + localX * sine + localY * cosine;

			return point;
		}

		void RotateTowardMouse(Ship& ship)
		{
			// Vector de la nave al mouse
			const Vector2 mousePosition = GetMousePosition();
			const float distanceX = mousePosition.x - ship.position.x;
			const float distanceY = mousePosition.y - ship.position.y;

			// atan2 da el angulo de ese vector
			ship.rotation = std::atan2(distanceY, distanceX);
		}

		void Accelerate(Ship& ship, float deltaTime)
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
	}

	void LoadTextures()
	{
		shipTexture = LoadTexture(SHIP_TEXTURE_PATH);
		flameTexture = LoadTexture(FLAME_TEXTURE_PATH);

		// Filtro suave para que no se vea pixelado al escalar y rotar
		SetTextureFilter(shipTexture, TEXTURE_FILTER_BILINEAR);
		SetTextureFilter(flameTexture, TEXTURE_FILTER_BILINEAR);
	}

	void UnloadTextures()
	{
		UnloadTexture(shipTexture);
		UnloadTexture(flameTexture);
	}

	Ship Create()
	{
		Ship ship = {};

		ship.position.x = static_cast<float>(config::SCREEN_WIDTH) / 2.0f;
		ship.position.y = static_cast<float>(config::SCREEN_HEIGHT) / 2.0f;
		ship.velocity = { 0.0f, 0.0f };
		ship.rotation = 0.0f;
		ship.isAccelerating = false;
		ship.invulnerableTimeLeft = 0.0f;

		return ship;
	}

	void Update(Ship& ship)
	{
		// Tiempo del ultimo frame: el movimiento no depende de los FPS
		const float deltaTime = GetFrameTime();

		RotateTowardMouse(ship);

		// Clic derecho mantenido: acelera hacia donde mira
		ship.isAccelerating = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

		if (ship.isAccelerating)
		{
			Accelerate(ship, deltaTime);
		}

		// Gravedad cero: nunca frena sola, sigue con la misma velocidad
		ship.position.x += ship.velocity.x * deltaTime;
		ship.position.y += ship.velocity.y * deltaTime;

		// Si sale por un borde, aparece por el contrario
		screen_wrap::WrapPosition(ship.position, WRAP_MARGIN);

		// Descuenta el tiempo de invulnerabilidad
		if (ship.invulnerableTimeLeft > 0.0f)
		{
			ship.invulnerableTimeLeft -= deltaTime;
		}
	}

	void Respawn(Ship& ship)
	{
		ship = Create();
		ship.invulnerableTimeLeft = RESPAWN_INVULNERABLE_TIME;
	}

	bool IsInvulnerable(const Ship& ship)
	{
		return ship.invulnerableTimeLeft > 0.0f;
	}

	Vector2 GetNosePosition(const Ship& ship)
	{
		return ToScreen(ship, NOSE_LENGTH, 0.0f);
	}

	void Draw(const Ship& ship)
	{
		// Mientras es invulnerable parpadea: se dibuja un intervalo si y otro no
		if (IsInvulnerable(ship))
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
			const Vector2 flameOrigin = { TAIL_LENGTH + FLAME_DRAW_WIDTH, FLAME_DRAW_HEIGHT / 2.0f };
			DrawRotatedTexture(flameTexture, ship.position, FLAME_DRAW_WIDTH, FLAME_DRAW_HEIGHT, flameOrigin, ship.rotation);
		}

		// Nave: pivote en el centro del sprite (como el ejemplo de DrawRectanglePro)
		const Vector2 shipOrigin = { SHIP_DRAW_SIZE / 2.0f, SHIP_DRAW_SIZE / 2.0f };
		DrawRotatedTexture(shipTexture, ship.position, SHIP_DRAW_SIZE, SHIP_DRAW_SIZE, shipOrigin, ship.rotation);
	}
}
