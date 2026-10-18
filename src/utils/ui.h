#pragma once

#include "raylib.h"

// Botones y texto centrado
namespace ReyesCuadra::ui
{
	struct Button
	{
		Rectangle bounds;
		const char* text;
	};

	// Boton en una posicion cualquiera
	Button CreateButton(float positionX, float positionY, float width, float height, const char* text);

	// Boton centrado en X
	Button CreateCenteredButton(float positionY, float width, float height, const char* text);

	bool IsButtonHovered(const Button& button);
	bool IsButtonClicked(const Button& button);
	void DrawButton(const Button& button);

	// Texto centrado en X
	void DrawCenteredText(const char* text, int positionY, int fontSize, Color color);

	// Mira que reemplaza al cursor del sistema
	void LoadCursorTexture();
	void DrawCursor();
	void UnloadCursorTexture();
}
