#include "utils/random_utils.h"

#include "raylib.h"

namespace ReyesCuadra
{
	// GetRandomValue de raylib solo da enteros: se pide un entero grande y se lo pasa a porcentaje
	static const int PRECISION = 10000;

	float RandomFloat(float min, float max)
	{
		const float randomPercent = static_cast<float>(GetRandomValue(0, PRECISION)) / static_cast<float>(PRECISION);

		// randomPercent va de 0.0 a 1.0: lo lleva al rango [min, max]
		// Ejemplo: min 50, max 90, randomPercent 0.5 -> 50 + 40 * 0.5 = 70
		return min + (max - min) * randomPercent;
	}
}
