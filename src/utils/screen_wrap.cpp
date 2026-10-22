#include "utils/screen_wrap.h"

#include "utils/config.h"

namespace ReyesCuadra
{
	void WrapPosition(Vector2& position, float margin)
	{
		const float screenWidth = static_cast<float>(SCREEN_WIDTH);
		const float screenHeight = static_cast<float>(SCREEN_HEIGHT);

		// El margen (el tamaño del objeto) hace que primero salga del todo
		// y recien ahi aparezca del otro lado (si no, desapareceria de golpe a medio salir)

		// Horizontal
		if (position.x < -margin)
		{
			position.x = screenWidth + margin;
		}
		else if (position.x > screenWidth + margin)
		{
			position.x = -margin;
		}

		// Vertical
		if (position.y < -margin)
		{
			position.y = screenHeight + margin;
		}
		else if (position.y > screenHeight + margin)
		{
			position.y = -margin;
		}
	}
}
