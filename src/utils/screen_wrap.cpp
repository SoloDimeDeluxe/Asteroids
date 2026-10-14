#include "utils/screen_wrap.h"

#include "utils/config.h"

namespace ReyesCuadra::screen_wrap
{
	void WrapPosition(Vector2& position, float margin)
	{
		const float screenWidth = static_cast<float>(config::SCREEN_WIDTH);
		const float screenHeight = static_cast<float>(config::SCREEN_HEIGHT);

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
