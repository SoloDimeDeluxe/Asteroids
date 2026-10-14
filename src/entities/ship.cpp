#include "entities/ship.h"

#include <cmath>

#include "utils/config.h"

namespace ReyesCuadra::ship
{
	namespace
	{
		constexpr float NOSE_LENGTH = 20.0f;   // Del centro a la punta
		constexpr float TAIL_LENGTH = 14.0f;   // Del centro a la cola
		constexpr float HALF_WIDTH = 12.0f;    // Mitad del ancho de la cola

		const Color SHIP_COLOR = RAYWHITE;

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
	}

	Ship Create()
	{
		Ship ship = {};

		ship.position.x = static_cast<float>(config::SCREEN_WIDTH) / 2.0f;
		ship.position.y = static_cast<float>(config::SCREEN_HEIGHT) / 2.0f;
		ship.rotation = 0.0f;

		return ship;
	}

	void Update(Ship& ship)
	{
		// Vector de la nave al mouse
		const Vector2 mousePosition = GetMousePosition();
		const float distanceX = mousePosition.x - ship.position.x;
		const float distanceY = mousePosition.y - ship.position.y;

		// atan2 da el angulo de ese vector
		ship.rotation = std::atan2(distanceY, distanceX);
	}

	void Draw(const Ship& ship)
	{
		// Triangulo: punta adelante, dos esquinas atras
		const Vector2 nose = ToScreen(ship, NOSE_LENGTH, 0.0f);
		const Vector2 leftCorner = ToScreen(ship, -TAIL_LENGTH, -HALF_WIDTH);
		const Vector2 rightCorner = ToScreen(ship, -TAIL_LENGTH, HALF_WIDTH);

		DrawTriangleLines(nose, leftCorner, rightCorner, SHIP_COLOR);
	}
}
