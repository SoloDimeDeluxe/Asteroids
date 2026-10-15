#include "screens/gameplay.h"

#include "raylib.h"

#include "entities/bullet.h"
#include "entities/ship.h"
#include "utils/config.h"
#include "utils/ui.h"

namespace ReyesCuadra::gameplay
{
	namespace
	{
		constexpr int PAUSE_FONT_SIZE = 60;
		constexpr int PAUSE_TITLE_POSITION_Y = 220;

		// Boton de pausa (arriba a la derecha)
		constexpr float PAUSE_BUTTON_WIDTH = 140.0f;
		constexpr float PAUSE_BUTTON_HEIGHT = 45.0f;
		constexpr float PAUSE_BUTTON_MARGIN = 15.0f;

		// Botones del menu de pausa
		constexpr float MENU_BUTTON_WIDTH = 260.0f;
		constexpr float MENU_BUTTON_HEIGHT = 60.0f;
		constexpr float RESUME_BUTTON_POSITION_Y = 340.0f;
		constexpr float BUTTON_SPACING = 90.0f;

		bool isPaused = false;
		ship::Ship playerShip = {};
		bullet::Bullet bullets[bullet::MAX_BULLETS] = {};

		ui::Button pauseButton = {};
		ui::Button resumeButton = {};
		ui::Button menuButton = {};

		// Clic izquierdo (o espacio) dispara.
		// Si el mouse esta sobre el boton de pausa no dispara,
		// asi el clic para pausar no tira una bala.
		bool IsFirePressed()
		{
			const bool clickedToFire = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ui::IsButtonHovered(pauseButton);

			return clickedToFire || IsKeyPressed(KEY_SPACE);
		}

		// Aca va la logica de la partida (nave, asteroides, balas)
		void UpdateMatch()
		{
			ship::Update(playerShip);

			if (IsFirePressed())
			{
				bullet::Fire(bullets, ship::GetNosePosition(playerShip), playerShip.rotation);
			}

			bullet::UpdateAll(bullets);
		}

		// Aca se dibuja la partida
		void DrawMatch()
		{
			bullet::DrawAll(bullets);
			ship::Draw(playerShip);
		}

		void DrawPauseOverlay()
		{
			// Capa oscura sobre la partida
			DrawRectangle(0, 0, config::SCREEN_WIDTH, config::SCREEN_HEIGHT, Fade(BLACK, 0.7f));

			ui::DrawCenteredText("PAUSA", PAUSE_TITLE_POSITION_Y, PAUSE_FONT_SIZE, RAYWHITE);

			ui::DrawButton(resumeButton);
			ui::DrawButton(menuButton);
		}
	}

	void Init()
	{
		isPaused = false;
		playerShip = ship::Create();
		bullet::InitAll(bullets);

		const float pauseButtonX = static_cast<float>(config::SCREEN_WIDTH) - PAUSE_BUTTON_WIDTH - PAUSE_BUTTON_MARGIN;
		pauseButton = ui::CreateButton(pauseButtonX, PAUSE_BUTTON_MARGIN, PAUSE_BUTTON_WIDTH, PAUSE_BUTTON_HEIGHT, "PAUSA");

		resumeButton = ui::CreateCenteredButton(RESUME_BUTTON_POSITION_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "CONTINUAR");
		menuButton = ui::CreateCenteredButton(RESUME_BUTTON_POSITION_Y + BUTTON_SPACING, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT, "MENU");
	}

	game::Screen Update()
	{
		// P o ESC tambien pausan/reanudan (opcional, por teclado)
		const bool pauseKeyPressed = IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE);

		if (isPaused)
		{
			if (ui::IsButtonClicked(menuButton))
			{
				return game::Screen::Menu;
			}

			if (ui::IsButtonClicked(resumeButton) || pauseKeyPressed)
			{
				isPaused = false;
			}
		}
		else
		{
			if (ui::IsButtonClicked(pauseButton) || pauseKeyPressed)
			{
				isPaused = true;
			}
			else
			{
				UpdateMatch();
			}
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
			ui::DrawButton(pauseButton);
		}
	}
}
