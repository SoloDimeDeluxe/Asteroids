#include "screens/gameplay.h"

#include "raylib.h"

#include "entities/ship.h"
#include "utils/config.h"
#include "utils/ui.h"

namespace ReyesCuadra::gameplay
{
	namespace
	{
		constexpr int HINT_FONT_SIZE = 20;
		constexpr int PAUSE_FONT_SIZE = 60;

		bool isPaused = false;
		ship::Ship playerShip = {};

		// Aca va la logica de la partida (nave, asteroides, balas)
		void UpdateMatch()
		{
			ship::Update(playerShip);
		}

		// Aca se dibuja la partida
		void DrawMatch()
		{
			ship::Draw(playerShip);
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
		playerShip = ship::Create();
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
