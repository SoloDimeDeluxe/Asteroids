#include "game/game.h"

#include "raylib.h"

#include "entities/asteroid.h"
#include "entities/bullet.h"
#include "entities/explosion.h"
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

namespace ReyesCuadra
{
	static Screen currentScreen = Screen::Menu;

	// Cambia de pantalla y la reinicia
	static void ChangeScreen(Screen newScreen)
	{
		currentScreen = newScreen;

		// Al salir del gameplay el motor y el OVNI dejan de sonar
		UpdateThrustSound(false);
		UpdateUfoHumSound(false);

		switch (currentScreen)
		{
		case Screen::Menu:
			InitMenu();
			PlayMusicTrack(MusicTrack::Menu);
			break;
		case Screen::Gameplay:
			InitGameplay();
			PlayMusicTrack(MusicTrack::Gameplay);
			break;
		case Screen::Instructions:
			InitInstructions();
			PlayMusicTrack(MusicTrack::Menu);
			break;
		case Screen::Credits:
			InitCredits();
			PlayMusicTrack(MusicTrack::Menu);
			break;
		case Screen::Exit:
			break;
		}
	}

	static void InitGame()
	{
		InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
		SetTargetFPS(TARGET_FPS);

		// ESC no cierra la ventana: lo maneja cada pantalla
		SetExitKey(KEY_NULL);

		// Las rutas "res/..." se buscan desde la carpeta del .exe,
		// asi funciona igual desde Visual Studio o abriendo el .exe suelto
		ChangeDirectory(GetApplicationDirectory());

		InitAudio();

		LoadShipTextures();
		LoadAsteroidTextures();
		LoadBulletTextures();
		LoadUfoTexture();
		LoadExplosionTexture();
		LoadPowerUpTexture();
		LoadCursorTexture();

		// Se usa la mira propia en vez del cursor del sistema
		HideCursor();

		ChangeScreen(Screen::Menu);
	}

	// Actualiza la pantalla activa
	static void UpdateGame()
	{
		UpdateMusicTrack();

		Screen nextScreen = currentScreen;

		switch (currentScreen)
		{
		case Screen::Menu:
			nextScreen = UpdateMenu();
			break;
		case Screen::Gameplay:
			nextScreen = UpdateGameplay();
			break;
		case Screen::Instructions:
			nextScreen = UpdateInstructions();
			break;
		case Screen::Credits:
			nextScreen = UpdateCredits();
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
	static void DrawGame()
	{
		BeginDrawing();
		ClearBackground(BLACK);

		switch (currentScreen)
		{
		case Screen::Menu:
			DrawMenu();
			break;
		case Screen::Gameplay:
			DrawGameplay();
			break;
		case Screen::Instructions:
			DrawInstructions();
			break;
		case Screen::Credits:
			DrawCredits();
			break;
		case Screen::Exit:
			break;
		}

		// La mira va ultima para quedar encima de todo
		DrawCursor();

		EndDrawing();
	}

	static void CloseGame()
	{
		UnloadShipTextures();
		UnloadAsteroidTextures();
		UnloadBulletTextures();
		UnloadUfoTexture();
		UnloadExplosionTexture();
		UnloadPowerUpTexture();
		UnloadCursorTexture();

		CloseAudio();

		CloseWindow();
	}

	void RunGame()
	{
		InitGame();

		// Loop principal
		while (!WindowShouldClose() && currentScreen != Screen::Exit)
		{
			UpdateGame();
			DrawGame();
		}

		CloseGame();
	}
}
