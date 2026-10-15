#include "entities/ship.h"

#include <cmath>

#include "utils/config.h"
#include "utils/screen_wrap.h"

namespace ReyesCuadra::ship
{
	namespace
	{
		// Forma
		constexpr float NOSE_LENGTH = 20.0f;   // Del centro a la punta
		constexpr float TAIL_LENGTH = 14.0f;   // Del centro a la cola
		constexpr float HALF_WIDTH = 12.0f;    // Mitad del ancho de la cola
		constexpr float FLAME_LENGTH = 28.0f;  // Del centro a la punta del fuego
		constexpr float FLAME_HALF_WIDTH = 6.0f;
		constexpr float WRAP_MARGIN = FLAME_LENGTH; // Lo mas lejos que se dibuja algo desde el centro

		// Movimiento
		constexpr float ACCELERATION = 300.0f; // Pixeles por segundo, cada segundo
		constexpr float MAX_SPEED = 450.0f;    // Pixeles por segundo

		// Invulnerabilidad al reaparecer
		constexpr float RESPAWN_INVULNERABLE_TIME = 3.0f; // Segundos
		constexpr float BLINK_INTERVAL = 0.15f;           // Segundos entre parpadeos

		const Color SHIP_COLOR = RAYWHITE;
		const Color FLAME_COLOR = ORANGE;

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

		// Triangulo: punta adelante, dos esquinas atras
		const Vector2 nose = GetNosePosition(ship);
		const Vector2 leftCorner = ToScreen(ship, -TAIL_LENGTH, -HALF_WIDTH);
		const Vector2 rightCorner = ToScreen(ship, -TAIL_LENGTH, HALF_WIDTH);

		DrawTriangleLines(nose, leftCorner, rightCorner, SHIP_COLOR);

		// Fuego del motor, solo mientras acelera
		if (ship.isAccelerating)
		{
			const Vector2 flameTip = ToScreen(ship, -FLAME_LENGTH, 0.0f);
			const Vector2 flameLeft = ToScreen(ship, -TAIL_LENGTH, -FLAME_HALF_WIDTH);
			const Vector2 flameRight = ToScreen(ship, -TAIL_LENGTH, FLAME_HALF_WIDTH);

			DrawTriangleLines(flameTip, flameLeft, flameRight, FLAME_COLOR);
		}
	}
}
