#include "screens/credits.h"

#include "raylib.h"

#include "utils/config.h"
#include "utils/ui.h"

namespace ReyesCuadra::credits
{
	static constexpr int TITLE_FONT_SIZE = 60;
	static constexpr int TITLE_POSITION_Y = 120;

	static constexpr int TEXT_FONT_SIZE = 25;
	static constexpr int FIRST_LINE_POSITION_Y = 260;
	static constexpr int LINE_SPACING = 50;

	static constexpr float BACK_BUTTON_WIDTH = 220.0f;
	static constexpr float BACK_BUTTON_HEIGHT = 60.0f;
	static constexpr float BACK_BUTTON_POSITION_Y = 600.0f;

	static ui::Button backButton = {};

	void Init()
	{
		backButton = ui::CreateCenteredButton(BACK_BUTTON_POSITION_Y, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT, "VOLVER");
	}

	game::Screen Update()
	{
		if (ui::IsButtonClicked(backButton) || IsKeyPressed(KEY_ESCAPE))
		{
			return game::Screen::Menu;
		}

		return game::Screen::Credits;
	}

	void Draw()
	{
		ui::DrawCenteredText("CREDITOS", TITLE_POSITION_Y, TITLE_FONT_SIZE, RAYWHITE);

		ui::DrawCenteredText(TextFormat("Programacion: %s", config::AUTHOR_NAME),
			FIRST_LINE_POSITION_Y, TEXT_FONT_SIZE, LIGHTGRAY);
		ui::DrawCenteredText("Sprites, Musica, Assets y Texturas: generados con IA (Claude, de Anthropic)",
			FIRST_LINE_POSITION_Y + LINE_SPACING, TEXT_FONT_SIZE, LIGHTGRAY);
		ui::DrawCenteredText("Hecho con raylib",
			FIRST_LINE_POSITION_Y + LINE_SPACING * 2, TEXT_FONT_SIZE, LIGHTGRAY);
		ui::DrawCenteredText("Basado en Asteroids (Atari, 1979)",
			FIRST_LINE_POSITION_Y + LINE_SPACING * 3, TEXT_FONT_SIZE, LIGHTGRAY);
		ui::DrawCenteredText("Image Campus",
			FIRST_LINE_POSITION_Y + LINE_SPACING * 4, TEXT_FONT_SIZE, LIGHTGRAY);

		ui::DrawButton(backButton);
	}
}
