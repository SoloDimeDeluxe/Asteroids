#include "screens/credits.h"

#include "raylib.h"

#include "utils/config.h"
#include "utils/ui.h"

static const int TITLE_FONT_SIZE = 60;
static const int TITLE_POSITION_Y = 120;

static const int TEXT_FONT_SIZE = 25;
static const int FIRST_LINE_POSITION_Y = 260;
static const int LINE_SPACING = 50;

static const float BACK_BUTTON_WIDTH = 220.0f;
static const float BACK_BUTTON_HEIGHT = 60.0f;
static const float BACK_BUTTON_POSITION_Y = 600.0f;

static Button backButton = {};

void InitCredits()
{
	backButton = CreateCenteredButton(BACK_BUTTON_POSITION_Y, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT, "VOLVER");
}

Screen UpdateCredits()
{
	if (IsButtonClicked(backButton) || IsKeyPressed(KEY_ESCAPE))
	{
		return Screen::Menu;
	}

	return Screen::Credits;
}

void DrawCredits()
{
	DrawCenteredText("CREDITOS", TITLE_POSITION_Y, TITLE_FONT_SIZE, RAYWHITE);

	DrawCenteredText(TextFormat("Programacion: %s", AUTHOR_NAME),
		FIRST_LINE_POSITION_Y, TEXT_FONT_SIZE, LIGHTGRAY);
	DrawCenteredText("Sprites, Musica, Assets y Texturas: generados con IA (Claude, de Anthropic)",
		FIRST_LINE_POSITION_Y + LINE_SPACING, TEXT_FONT_SIZE, LIGHTGRAY);
	DrawCenteredText("Hecho con raylib",
		FIRST_LINE_POSITION_Y + LINE_SPACING * 2, TEXT_FONT_SIZE, LIGHTGRAY);
	DrawCenteredText("Basado en Asteroids (Atari, 1979)",
		FIRST_LINE_POSITION_Y + LINE_SPACING * 3, TEXT_FONT_SIZE, LIGHTGRAY);
	DrawCenteredText("Image Campus",
		FIRST_LINE_POSITION_Y + LINE_SPACING * 4, TEXT_FONT_SIZE, LIGHTGRAY);

	DrawButton(backButton);
}
