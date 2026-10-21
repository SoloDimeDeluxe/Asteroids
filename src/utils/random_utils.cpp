#include "utils/random_utils.h"

#include "raylib.h"

namespace ReyesCuadra
{
	// GetRandomValue de raylib solo da enteros: se pide un entero grande y se lo pasa a porcentaje
	static const int PRECISION = 10000;

	float RandomFloat(float min, float max)
	{
		const float randomPercent = static_cast<float>(GetRandomValue(0, PRECISION)) / static_cast<float>(PRECISION);

		return min + (max - min) * randomPercent;
	}
}
