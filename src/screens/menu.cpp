#include "screens/menu.h"

#include "raylib.h"

#include "utils/config.h"
#include "utils/ui.h"

namespace ReyesCuadra
{
	static const int TITLE_FONT_SIZE = 80;
	static const int TITLE_POSITION_Y = 150;

	static const float BUTTON_WIDTH = 260.0f;
	static const float BUTTON_HEIGHT = 60.0f;
	static const float FIRST_BUTTON_POSITION_Y = 300.0f;
	static const float BUTTON_SPACING = 85.0f;

	static const int VERSION_FONT_SIZE = 20;
	static const int VERSION_MARGIN = 10;

	static Button playButton = {};
	static Button instructionsButton = {};
	static Button creditsButton = {};
	static Button exitButton = {};

	void InitMenu()
	{
		playButton = CreateCenteredButton(FIRST_BUTTON_POSITION_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "JUGAR");
		instructionsButton = CreateCenteredButton(FIRST_BUTTON_POSITION_Y + BUTTON_SPACING, BUTTON_WIDTH, BUTTON_HEIGHT, "COMO JUGAR");
		creditsButton = CreateCenteredButton(FIRST_BUTTON_POSITION_Y + BUTTON_SPACING * 2.0f, BUTTON_WIDTH, BUTTON_HEIGHT, "CREDITOS");
		exitButton = CreateCenteredButton(FIRST_BUTTON_POSITION_Y + BUTTON_SPACING * 3.0f, BUTTON_WIDTH, BUTTON_HEIGHT, "SALIR");
	}

	Screen UpdateMenu()
	{
		if (IsButtonClicked(playButton))
		{
			return Screen::Gameplay;
		}

		if (IsButtonClicked(instructionsButton))
		{
			return Screen::Instructions;
		}

		if (IsButtonClicked(creditsButton))
		{
			return Screen::Credits;
		}

		if (IsButtonClicked(exitButton) || IsKeyPressed(KEY_ESCAPE))
		{
			return Screen::Exit;
		}

		return Screen::Menu;
	}

	void DrawMenu()
	{
		DrawCenteredText("ASTEROIDS", TITLE_POSITION_Y, TITLE_FONT_SIZE, RAYWHITE);

		DrawButton(playButton);
		DrawButton(instructionsButton);
		DrawButton(creditsButton);
		DrawButton(exitButton);

		// Version abajo a la derecha
		const int versionWidth = MeasureText(GAME_VERSION, VERSION_FONT_SIZE);
		DrawText(GAME_VERSION,
			SCREEN_WIDTH - versionWidth - VERSION_MARGIN,
			SCREEN_HEIGHT - VERSION_FONT_SIZE - VERSION_MARGIN,
			VERSION_FONT_SIZE, GRAY);
	}
}
