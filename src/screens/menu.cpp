#include "screens/menu.h"

#include "raylib.h"

#include "utils/config.h"
#include "utils/ui.h"

namespace ReyesCuadra::menu
{
	namespace
	{
		constexpr int TITLE_FONT_SIZE = 80;
		constexpr int TITLE_POSITION_Y = 150;

		constexpr float BUTTON_WIDTH = 260.0f;
		constexpr float BUTTON_HEIGHT = 60.0f;
		constexpr float FIRST_BUTTON_POSITION_Y = 340.0f;
		constexpr float BUTTON_SPACING = 90.0f;

		constexpr int VERSION_FONT_SIZE = 20;
		constexpr int VERSION_MARGIN = 10;

		ui::Button playButton = {};
		ui::Button creditsButton = {};
		ui::Button exitButton = {};
	}

	void Init()
	{
		playButton = ui::CreateCenteredButton(FIRST_BUTTON_POSITION_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "JUGAR");
		creditsButton = ui::CreateCenteredButton(FIRST_BUTTON_POSITION_Y + BUTTON_SPACING, BUTTON_WIDTH, BUTTON_HEIGHT, "CREDITOS");
		exitButton = ui::CreateCenteredButton(FIRST_BUTTON_POSITION_Y + BUTTON_SPACING * 2.0f, BUTTON_WIDTH, BUTTON_HEIGHT, "SALIR");
	}

	game::Screen Update()
	{
		if (ui::IsButtonClicked(playButton))
		{
			return game::Screen::Gameplay;
		}

		if (ui::IsButtonClicked(creditsButton))
		{
			return game::Screen::Credits;
		}

		if (ui::IsButtonClicked(exitButton) || IsKeyPressed(KEY_ESCAPE))
		{
			return game::Screen::Exit;
		}

		return game::Screen::Menu;
	}

	void Draw()
	{
		ui::DrawCenteredText("ASTEROIDS", TITLE_POSITION_Y, TITLE_FONT_SIZE, RAYWHITE);

		ui::DrawButton(playButton);
		ui::DrawButton(creditsButton);
		ui::DrawButton(exitButton);

		// Version abajo a la derecha
		const int versionWidth = MeasureText(config::GAME_VERSION, VERSION_FONT_SIZE);
		DrawText(config::GAME_VERSION,
			config::SCREEN_WIDTH - versionWidth - VERSION_MARGIN,
			config::SCREEN_HEIGHT - VERSION_FONT_SIZE - VERSION_MARGIN,
			VERSION_FONT_SIZE, GRAY);
	}
}
