#include "utils/ui.h"

#include "utils/config.h"

namespace ReyesCuadra::ui
{
	namespace
	{
		constexpr int BUTTON_FONT_SIZE = 30;
		constexpr float BUTTON_BORDER_THICKNESS = 2.0f;

		const Color BUTTON_COLOR = RAYWHITE;
		const Color BUTTON_HOVER_COLOR = SKYBLUE;
	}

	Button CreateCenteredButton(float positionY, float width, float height, const char* text)
	{
		Button button = {};

		button.bounds.x = (static_cast<float>(config::SCREEN_WIDTH) - width) / 2.0f;
		button.bounds.y = positionY;
		button.bounds.width = width;
		button.bounds.height = height;
		button.text = text;

		return button;
	}

	bool IsButtonHovered(const Button& button)
	{
		return CheckCollisionPointRec(GetMousePosition(), button.bounds);
	}

	bool IsButtonClicked(const Button& button)
	{
		return IsButtonHovered(button) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
	}

	void DrawButton(const Button& button)
	{
		// Solo el borde (estilo vectorial), resaltado con el mouse encima
		const Color color = IsButtonHovered(button) ? BUTTON_HOVER_COLOR : BUTTON_COLOR;

		DrawRectangleLinesEx(button.bounds, BUTTON_BORDER_THICKNESS, color);

		// Texto centrado en el boton
		const int textWidth = MeasureText(button.text, BUTTON_FONT_SIZE);
		const int textX = static_cast<int>(button.bounds.x + (button.bounds.width - static_cast<float>(textWidth)) / 2.0f);
		const int textY = static_cast<int>(button.bounds.y + (button.bounds.height - static_cast<float>(BUTTON_FONT_SIZE)) / 2.0f);

		DrawText(button.text, textX, textY, BUTTON_FONT_SIZE, color);
	}

	void DrawCenteredText(const char* text, int positionY, int fontSize, Color color)
	{
		const int textWidth = MeasureText(text, fontSize);
		const int textX = (config::SCREEN_WIDTH - textWidth) / 2;

		DrawText(text, textX, positionY, fontSize, color);
	}
}
