#include "screens/instructions.h"

#include <iterator>

#include "raylib.h"

#include "utils/ui.h"

namespace ReyesCuadra
{
	// Cada renglon de la pantalla: titulo de seccion o texto normal
	struct InstructionLine
	{
		const char* text;
		bool isSectionTitle;
	};

	static const int TITLE_FONT_SIZE = 50;
	static const int TITLE_POSITION_Y = 40;

	static const int SECTION_FONT_SIZE = 28;
	static const int TEXT_FONT_SIZE = 22;
	static const int FIRST_LINE_POSITION_Y = 120;
	static const int LINE_SPACING = 29;
	static const int SECTION_EXTRA_SPACING = 16; // Espacio de mas antes de cada seccion

	static const float BACK_BUTTON_WIDTH = 220.0f;
	static const float BACK_BUTTON_HEIGHT = 60.0f;
	static const float BACK_BUTTON_POSITION_Y = 685.0f;

	static const Color SECTION_COLOR = SKYBLUE;
	static const Color TEXT_COLOR = LIGHTGRAY;

	static const InstructionLine LINES[] =
	{
		{ "OBJETIVO", true },
		{ "Sumar puntos: cada asteroide da 1 punto y cada OVNI da 5.", false },
		{ "Los grandes se parten en medianos y los medianos en chicos.", false },
		{ "Si algo toca la nave (asteroide, OVNI o bala), se pierde una vida.", false },
		{ "Con las 3 vidas perdidas, termina la partida.", false },

		{ "CONTROLES", true },
		{ "Mover el mouse: apuntar la nave", false },
		{ "Mantener clic derecho: acelerar", false },
		{ "Clic izquierdo o barra espaciadora: disparar", false },
		{ "La nave no frena sola: para frenar, acelerar hacia el otro lado.", false },

		{ "BOMBA Y OVNI", true },
		{ "Bomba: si la nave la toca, destruye todo lo que hay en pantalla.", false },
		{ "OVNI: cruza la pantalla disparando a la nave. Hay que esquivarlo.", false },

		{ "PAUSA", true },
		{ "Boton PAUSA (arriba a la derecha), P o ESC: pausar el juego.", false },
		{ "Desde la pausa se puede continuar o volver al MENU.", false },
	};

	static const int LINE_COUNT = static_cast<int>(std::size(LINES));

	static Button backButton = {};

	void InitInstructions()
	{
		backButton = CreateCenteredButton(BACK_BUTTON_POSITION_Y, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT, "VOLVER");
	}

	Screen UpdateInstructions()
	{
		if (IsButtonClicked(backButton) || IsKeyPressed(KEY_ESCAPE))
		{
			return Screen::Menu;
		}

		return Screen::Instructions;
	}

	void DrawInstructions()
	{
		DrawCenteredText("COMO JUGAR", TITLE_POSITION_Y, TITLE_FONT_SIZE, RAYWHITE);

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

				DrawCenteredText(LINES[i].text, positionY, SECTION_FONT_SIZE, SECTION_COLOR);
			}
			else
			{
				DrawCenteredText(LINES[i].text, positionY, TEXT_FONT_SIZE, TEXT_COLOR);
			}

			positionY += LINE_SPACING;
		}

		DrawButton(backButton);
	}
}
