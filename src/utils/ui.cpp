#include "utils/ui.h"

#include "game/audio.h"
#include "utils/config.h"

namespace ReyesCuadra
{
	static const int BUTTON_FONT_SIZE = 30;
	static const float BUTTON_BORDER_THICKNESS = 2.0f;

	static const char* const CURSOR_TEXTURE_PATH = "res/textures/cursor.png";
	static const float CURSOR_DRAW_SIZE = 32.0f;

	static Texture2D cursorTexture = {};

	static const Color BUTTON_COLOR = RAYWHITE;
	static const Color BUTTON_HOVER_COLOR = SKYBLUE;

	Button CreateButton(float positionX, float positionY, float width, float height, const char* text)
	{
		Button button = {};

		button.bounds.x = positionX;
		button.bounds.y = positionY;
		button.bounds.width = width;
		button.bounds.height = height;
		button.text = text;

		return button;
	}

	Button CreateCenteredButton(float positionY, float width, float height, const char* text)
	{
		const float positionX = (static_cast<float>(SCREEN_WIDTH) - width) / 2.0f;

		return CreateButton(positionX, positionY, width, height, text);
	}

	bool IsButtonHovered(const Button& button)
	{
		return CheckCollisionPointRec(GetMousePosition(), button.bounds);
	}

	bool IsButtonClicked(const Button& button)
	{
		const bool isClicked = IsButtonHovered(button) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

		// Todos los botones hacen el mismo sonido al apretarlos
		if (isClicked)
		{
			PlaySoundEffect(SoundEffect::ButtonClick);
		}

		return isClicked;
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
		const int textX = (SCREEN_WIDTH - textWidth) / 2;

		DrawText(text, textX, positionY, fontSize, color);
	}

	void LoadCursorTexture()
	{
		cursorTexture = LoadTexture(CURSOR_TEXTURE_PATH);
	}

	void DrawCursor()
	{
		// Centro de la mira justo en la punta del mouse
		const Vector2 mousePosition = GetMousePosition();
		const Rectangle source = { 0.0f, 0.0f, static_cast<float>(cursorTexture.width), static_cast<float>(cursorTexture.height) };
		const Rectangle destination = { mousePosition.x, mousePosition.y, CURSOR_DRAW_SIZE, CURSOR_DRAW_SIZE };
		const Vector2 origin = { CURSOR_DRAW_SIZE / 2.0f, CURSOR_DRAW_SIZE / 2.0f };

		DrawTexturePro(cursorTexture, source, destination, origin, 0.0f, WHITE);
	}

	void UnloadCursorTexture()
	{
		UnloadTexture(cursorTexture);
	}
}
