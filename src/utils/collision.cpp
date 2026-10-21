#include "utils/collision.h"

bool AreCirclesColliding(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB)
{
	// Distancia entre centros en cada eje
	const float distanceX = centerA.x - centerB.x;
	const float distanceY = centerA.y - centerB.y;

	// Pitagoras: distancia^2 = distanciaX^2 + distanciaY^2
	const float squaredDistance = distanceX * distanceX + distanceY * distanceY;

	// Se tocan si la distancia es menor o igual a la suma de los radios
	// (las dos partes al cuadrado, asi no hace falta la raiz)
	const float radiusSum = radiusA + radiusB;

	return squaredDistance <= radiusSum * radiusSum;
}
