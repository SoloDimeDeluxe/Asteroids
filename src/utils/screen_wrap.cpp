#include "utils/screen_wrap.h"

#include "utils/config.h"

void WrapPosition(Vector2& position, float margin)
{
	const float screenWidth = static_cast<float>(SCREEN_WIDTH);
	const float screenHeight = static_cast<float>(SCREEN_HEIGHT);

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
