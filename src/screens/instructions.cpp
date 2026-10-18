#include "screens/instructions.h"

#include <iterator>

#include "raylib.h"

#include "utils/ui.h"

namespace ReyesCuadra::instructions
{
	// Cada renglon de la pantalla: titulo de seccion o texto normal
	struct InstructionLine
	{
		const char* text;
		bool isSectionTitle;
	};

	static constexpr int TITLE_FONT_SIZE = 50;
	static constexpr int TITLE_POSITION_Y = 50;

	static constexpr int SECTION_FONT_SIZE = 28;
	static constexpr int TEXT_FONT_SIZE = 22;
	static constexpr int FIRST_LINE_POSITION_Y = 140;
	static constexpr int LINE_SPACING = 34;
	static constexpr int SECTION_EXTRA_SPACING = 16; // Espacio de mas antes de cada seccion

	static constexpr float BACK_BUTTON_WIDTH = 220.0f;
	static constexpr float BACK_BUTTON_HEIGHT = 60.0f;
	static constexpr float BACK_BUTTON_POSITION_Y = 670.0f;

	static const Color SECTION_COLOR = SKYBLUE;
	static const Color TEXT_COLOR = LIGHTGRAY;

	static const InstructionLine LINES[] =
	{
		{ "OBJETIVO", true },
		{ "Destruir la mayor cantidad de asteroides posible.", false },
		{ "Los grandes se parten en medianos y los medianos en chicos.", false },
		{ "Si un asteroide toca la nave, se pierde una vida.", false },
		{ "Con las 3 vidas perdidas, termina la partida.", false },

		{ "CONTROLES", true },
		{ "Mover el mouse: apuntar la nave", false },
		{ "Mantener clic derecho: acelerar", false },
		{ "Clic izquierdo o barra espaciadora: disparar", false },
		{ "La nave no frena sola: para frenar, acelerar hacia el otro lado.", false },

		{ "PAUSA", true },
		{ "Boton PAUSA (arriba a la derecha), P o ESC: pausar el juego.", false },
		{ "Desde la pausa se puede continuar o volver al MENU.", false },
	};

	static constexpr int LINE_COUNT = static_cast<int>(std::size(LINES));

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

		return game::Screen::Instructions;
	}

	void Draw()
	{
		ui::DrawCenteredText("COMO JUGAR", TITLE_POSITION_Y, TITLE_FONT_SIZE, RAYWHITE);

		int positionY = FIRST_LINE_POSITION_Y;

		for (int i = 0; i < LINE_COUNT; i++)
		{
			if (LINES[i].isSectionTitle)
			{
				// Las secciones (menos la primera) llevan un poco de aire antes
				if (i > 0)
				{
					positionY += SECTION_EXTRA_SPACING;
				}

				ui::DrawCenteredText(LINES[i].text, positionY, SECTION_FONT_SIZE, SECTION_COLOR);
			}
			else
			{
				ui::DrawCenteredText(LINES[i].text, positionY, TEXT_FONT_SIZE, TEXT_COLOR);
			}

			positionY += LINE_SPACING;
		}

		ui::DrawButton(backButton);
	}
}
