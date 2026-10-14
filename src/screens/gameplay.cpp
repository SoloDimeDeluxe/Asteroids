#include "screens/gameplay.h"

#include "raylib.h"

#include "utils/config.h"
#include "utils/ui.h"

namespace ReyesCuadra::gameplay
{
	namespace
	{
		constexpr int PLACEHOLDER_FONT_SIZE = 30;
		constexpr int HINT_FONT_SIZE = 20;
		constexpr int PAUSE_FONT_SIZE = 60;

		bool isPaused = false;

		// Aca va la logica de la partida (nave, asteroides, balas)
		void UpdateMatch()
		{
		}

		// Aca se dibuja la partida
		void DrawMatch()
		{
			ui::DrawCenteredText("Aca van a ir la nave y los asteroides",
				config::SCREEN_HEIGHT / 2 - PLACEHOLDER_FONT_SIZE, PLACEHOLDER_FONT_SIZE, DARKGRAY);
		}

		void DrawPauseOverlay()
		{
			// Capa oscura sobre la partida
			DrawRectangle(0, 0, config::SCREEN_WIDTH, config::SCREEN_HEIGHT, Fade(BLACK, 0.7f));

			ui::DrawCenteredText("PAUSA", config::SCREEN_HEIGHT / 2 - PAUSE_FONT_SIZE, PAUSE_FONT_SIZE, RAYWHITE);
			ui::DrawCenteredText("P para continuar - ESC para volver al menu",
				config::SCREEN_HEIGHT / 2 + HINT_FONT_SIZE, HINT_FONT_SIZE, LIGHTGRAY);
		}
	}

	void Init()
	{
		isPaused = false;
	}

	game::Screen Update()
	{
		if (IsKeyPressed(KEY_ESCAPE))
		{
			return game::Screen::Menu;
		}

		if (IsKeyPressed(KEY_P))
		{
			isPaused = !isPaused;
		}

		if (!isPaused)
		{
			UpdateMatch();
		}

		return game::Screen::Gameplay;
	}

	void Draw()
	{
		DrawMatch();

		if (isPaused)
		{
			DrawPauseOverlay();
		}
		else
		{
			DrawText("P: pausa  -  ESC: menu", 10, 10, HINT_FONT_SIZE, GRAY);
		}
	}
}
