#include "game/game.h"

#include "raylib.h"

#include "game/screen.h"
#include "screens/credits.h"
#include "screens/gameplay.h"
#include "screens/menu.h"
#include "utils/config.h"

namespace ReyesCuadra::game
{
	// Privado de este archivo
	namespace
	{
		Screen currentScreen = Screen::Menu;

		// Cambia de pantalla y la reinicia
		void ChangeScreen(Screen newScreen)
		{
			currentScreen = newScreen;

			switch (currentScreen)
			{
			case Screen::Menu:
				menu::Init();
				break;
			case Screen::Gameplay:
				gameplay::Init();
				break;
			case Screen::Credits:
				credits::Init();
				break;
			case Screen::Exit:
				break;
			}
		}

		void Init()
		{
			InitWindow(config::SCREEN_WIDTH, config::SCREEN_HEIGHT, config::GAME_TITLE);
			SetTargetFPS(config::TARGET_FPS);

			// ESC no cierra la ventana: lo maneja cada pantalla
			SetExitKey(KEY_NULL);

			ChangeScreen(Screen::Menu);
		}

		// Actualiza la pantalla activa
		void Update()
		{
			Screen nextScreen = currentScreen;

			switch (currentScreen)
			{
			case Screen::Menu:
				nextScreen = menu::Update();
				break;
			case Screen::Gameplay:
				nextScreen = gameplay::Update();
				break;
			case Screen::Credits:
				nextScreen = credits::Update();
				break;
			case Screen::Exit:
				break;
			}

			if (nextScreen != currentScreen)
			{
				ChangeScreen(nextScreen);
			}
		}

		// Dibuja la pantalla activa
		void Draw()
		{
			BeginDrawing();
			ClearBackground(BLACK);

			switch (currentScreen)
			{
			case Screen::Menu:
				menu::Draw();
				break;
			case Screen::Gameplay:
				gameplay::Draw();
				break;
			case Screen::Credits:
				credits::Draw();
				break;
			case Screen::Exit:
				break;
			}

			EndDrawing();
		}

		void Close()
		{
			CloseWindow();
		}
	}

	void Run()
	{
		Init();

		// Loop principal
		while (!WindowShouldClose() && currentScreen != Screen::Exit)
		{
			Update();
			Draw();
		}

		Close();
	}
}
