#include "game/game.h"

#include "raylib.h"

#include "entities/asteroid.h"
#include "entities/bullet.h"
#include "entities/powerup.h"
#include "entities/ship.h"
#include "entities/ufo.h"
#include "game/audio.h"
#include "game/screen.h"
#include "screens/credits.h"
#include "screens/gameplay.h"
#include "screens/instructions.h"
#include "screens/menu.h"
#include "utils/config.h"
#include "utils/ui.h"

namespace ReyesCuadra::game
{
	static Screen currentScreen = Screen::Menu;

	// Cambia de pantalla y la reinicia
	static void ChangeScreen(Screen newScreen)
	{
		currentScreen = newScreen;

		// Al salir del gameplay el motor y el OVNI dejan de sonar
		audio::UpdateThrust(false);
		audio::UpdateUfoHum(false);

		switch (currentScreen)
		{
		case Screen::Menu:
			menu::Init();
			audio::PlayMusic(audio::MusicTrack::Menu);
			break;
		case Screen::Gameplay:
			gameplay::Init();
			audio::PlayMusic(audio::MusicTrack::Gameplay);
			break;
		case Screen::Instructions:
			instructions::Init();
			audio::PlayMusic(audio::MusicTrack::Menu);
			break;
		case Screen::Credits:
			credits::Init();
			audio::PlayMusic(audio::MusicTrack::Menu);
			break;
		case Screen::Exit:
			break;
		}
	}

	static void Init()
	{
		InitWindow(config::SCREEN_WIDTH, config::SCREEN_HEIGHT, config::GAME_TITLE);
		SetTargetFPS(config::TARGET_FPS);

		// ESC no cierra la ventana: lo maneja cada pantalla
		SetExitKey(KEY_NULL);

		// Las rutas "res/..." se buscan desde la carpeta del .exe,
		// asi funciona igual desde Visual Studio o abriendo el .exe suelto
		ChangeDirectory(GetApplicationDirectory());

		audio::Init();

		ship::LoadTextures();
		asteroid::LoadTextures();
		bullet::LoadTextures();
		ufo::LoadTexture();
		powerup::LoadTexture();
		ui::LoadCursorTexture();

		// Se usa la mira propia en vez del cursor del sistema
		HideCursor();

		ChangeScreen(Screen::Menu);
	}

	// Actualiza la pantalla activa
	static void Update()
	{
		audio::UpdateMusic();

		Screen nextScreen = currentScreen;

		switch (currentScreen)
		{
		case Screen::Menu:
			nextScreen = menu::Update();
			break;
		case Screen::Gameplay:
			nextScreen = gameplay::Update();
			break;
		case Screen::Instructions:
			nextScreen = instructions::Update();
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
	static void Draw()
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
		case Screen::Instructions:
			instructions::Draw();
			break;
		case Screen::Credits:
			credits::Draw();
			break;
		case Screen::Exit:
			break;
		}

		// La mira va ultima para quedar encima de todo
		ui::DrawCursor();

		EndDrawing();
	}

	static void Close()
	{
		ship::UnloadTextures();
		asteroid::UnloadTextures();
		bullet::UnloadTextures();
		ufo::UnloadTexture();
		powerup::UnloadTexture();
		ui::UnloadCursorTexture();

		audio::Close();

		CloseWindow();
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
