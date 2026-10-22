#include "entities/ship.h"

#include <cmath>

#include "utils/config.h"
#include "utils/screen_wrap.h"

namespace ReyesCuadra
{
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

	// Dibuja una textura rotada con DrawTexturePro:
	// - source: que parte de la imagen se dibuja (la imagen entera o un cuadro de la animacion)
	// - position + width/height: donde y de que tamaño se dibuja en pantalla (asi se escala)
	// - origin: pivote de rotacion, medido desde la esquina de la imagen dibujada.
	//   Ese punto es el que queda ubicado en "position" y alrededor de el gira
	// - tint: tiñe la imagen (WHITE = colores originales, RED = nave destruida)
	static void DrawRotatedTexture(const Texture2D& texture, Rectangle source, Vector2 position, float width, float height,
		Vector2 origin, float rotationRadians, Color tint)
	{
		const Rectangle destination = { position.x, position.y, width, height };

		// DrawTexturePro usa grados, nuestra rotacion esta en radianes
		DrawTexturePro(texture, source, destination, origin, rotationRadians * RAD2DEG, tint);
	}

	// Rota un punto relativo a la nave y lo pasa a coordenadas de pantalla.
	// "localX/localY" es el punto como si la nave mirara a la derecha (ej: la punta es (22, 0)).
	// Es la formula de rotacion de un punto (matriz de rotacion de algebra):
	//   x' = x * cos - y * sin
	//   y' = x * sin + y * cos
	// y despues se le suma la posicion de la nave.
	// Con localY = 0 queda: avanzar "localX" pixeles desde el centro en la direccion en la que mira
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
		// Vector de la nave al mouse (restar posiciones: destino - origen)
		const Vector2 mousePosition = GetMousePosition();
		const float distanceX = mousePosition.x - ship.position.x;
		const float distanceY = mousePosition.y - ship.position.y;

		// atan2(y, x) da el angulo de ese vector en radianes.
		// Se usa atan2 y no atan(y / x) porque distingue los 4 cuadrantes
		// (sabe si el mouse esta a la izquierda o a la derecha) y no falla si x es 0
		ship.rotation = std::atan2(distanceY, distanceX);
	}

	static void Accelerate(Ship& ship, float deltaTime)
	{
		// Direccion de la nave: cos y sin pasan el angulo a un vector de largo 1
		// (0 rad = (1, 0) a la derecha; en pantalla Y crece hacia abajo)
		const float directionX = std::cos(ship.rotation);
		const float directionY = std::sin(ship.rotation);

		// La aceleracion se suma a la velocidad (por deltaTime: es "por segundo")
		ship.velocity.x += directionX * ACCELERATION * deltaTime;
		ship.velocity.y += directionY * ACCELERATION * deltaTime;

		// Si supera la velocidad maxima, se achica sin cambiar la direccion:
		// 1) speed = largo del vector velocidad (Pitagoras)
		// 2) velocidad / speed = vector de largo 1 en la misma direccion (normalizar)
		// 3) * MAX_SPEED = queda justo en la velocidad maxima
		// Aca si se usa sqrt porque hace falta el largo real (y es un solo calculo por frame)
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

		// Inercia (gravedad cero): no hay friccion, la velocidad solo cambia al acelerar.
		// Para frenar hay que acelerar en sentido contrario
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
		// Mientras es invulnerable parpadea: se dibuja un intervalo si y otro no.
		// Tiempo restante / BLINK_INTERVAL = numero de intervalo (ej: 2.40 / 0.15 = 16).
		// % 2 da 0 en los pares y 1 en los impares: en los impares no se dibuja
		if (IsShipInvulnerable(ship))
		{
			const int blinkStep = static_cast<int>(ship.invulnerableTimeLeft / BLINK_INTERVAL);

			if (blinkStep % 2 == 1)
			{
				return;
			}
		}

		// Fuego del motor (atras de la nave), solo mientras acelera.
		// Truco del pivote: el fuego se dibuja en la posicion de la nave (su centro), pero con el
		// origin corrido a la derecha de la imagen del fuego, a TAIL_LENGTH + su ancho.
		// Asi la imagen queda atras de la nave, pegada al motor, y como gira alrededor
		// del centro de la nave siempre queda atras aunque la nave rote (sin calcular con cos/sin).
		if (ship.isAccelerating)
		{
			// Animacion con hoja de sprites: la imagen tiene FLAME_FRAME_COUNT cuadros uno al lado del otro.
			// tiempo / tiempo por cuadro = cuantos cuadros pasaron; % FLAME_FRAME_COUNT hace que de la vuelta (0,1,2,3,0...).
			// El rectangulo source recorta solo ese cuadro (empieza en ancho de cuadro * numero de cuadro)
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
		const Vector2 lastPosition = ship.position;

		ship = CreateShip();

		ship.position = lastPosition;
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
}
