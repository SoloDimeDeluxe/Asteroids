#include "utils/collision.h"

namespace ReyesCuadra
{
	bool AreCirclesColliding(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB)
	{
		// Distancia entre centros en cada eje
		const float distanceX = centerA.x - centerB.x;
		const float distanceY = centerA.y - centerB.y;

		// Pitagoras: distancia^2 = distanciaX^2 + distanciaY^2
		const float squaredDistance = distanceX * distanceX + distanceY * distanceY;

		// Dos circulos se tocan si la distancia entre centros <= suma de los radios.
		// La distancia real seria sqrt(squaredDistance), pero la raiz es costosa y esto
		// se chequea muchas veces por frame (cada bala contra cada asteroide).
		// Truco: si a y b son positivos, a <= b es lo mismo que a*a <= b*b.
		// Entonces se comparan los dos valores al cuadrado y se evita la raiz.
		// Ejemplo: centros a 30 px, radios 20 y 15 -> 30*30 = 900 <= 35*35 = 1225 -> se tocan
		const float radiusSum = radiusA + radiusB;

		return squaredDistance <= radiusSum * radiusSum;
	}
}
