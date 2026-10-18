#include "game/audio.h"

#include "raylib.h"

namespace ReyesCuadra::audio
{
	namespace
	{
		constexpr float SOUND_VOLUME = 0.6f;
		constexpr float THRUST_VOLUME = 0.35f;
		constexpr float MUSIC_VOLUME = 0.45f;

		Sound shootSound = {};
		Sound asteroidExplosionSound = {};
		Sound shipExplosionSound = {};
		Sound buttonClickSound = {};
		Sound gameOverSound = {};
		Sound thrustSound = {};

		Music menuMusic = {};
		Music gameplayMusic = {};

		// Musica que esta sonando ahora (nullptr = ninguna)
		Music* currentMusic = nullptr;

		Sound LoadSoundWithVolume(const char* path, float volume)
		{
			Sound sound = LoadSound(path);
			SetSoundVolume(sound, volume);

			return sound;
		}

		Music LoadLoopingMusic(const char* path)
		{
			Music music = LoadMusicStream(path);
			music.looping = true; // Al terminar vuelve a empezar
			SetMusicVolume(music, MUSIC_VOLUME);

			return music;
		}

		Music* GetMusic(MusicTrack track)
		{
			switch (track)
			{
			case MusicTrack::Menu:
				return &menuMusic;
			case MusicTrack::Gameplay:
				return &gameplayMusic;
			}

			return &menuMusic;
		}
	}

	void Init()
	{
		InitAudioDevice();

		shootSound = LoadSoundWithVolume("res/sounds/shoot.wav", SOUND_VOLUME);
		asteroidExplosionSound = LoadSoundWithVolume("res/sounds/asteroid_explosion.wav", SOUND_VOLUME);
		shipExplosionSound = LoadSoundWithVolume("res/sounds/ship_explosion.wav", SOUND_VOLUME);
		buttonClickSound = LoadSoundWithVolume("res/sounds/button_click.wav", SOUND_VOLUME);
		gameOverSound = LoadSoundWithVolume("res/sounds/game_over.wav", SOUND_VOLUME);
		thrustSound = LoadSoundWithVolume("res/sounds/thrust.wav", THRUST_VOLUME);

		menuMusic = LoadLoopingMusic("res/music/menu_music.ogg");
		gameplayMusic = LoadLoopingMusic("res/music/gameplay_music.ogg");
	}

	void Close()
	{
		UnloadSound(shootSound);
		UnloadSound(asteroidExplosionSound);
		UnloadSound(shipExplosionSound);
		UnloadSound(buttonClickSound);
		UnloadSound(gameOverSound);
		UnloadSound(thrustSound);

		UnloadMusicStream(menuMusic);
		UnloadMusicStream(gameplayMusic);

		currentMusic = nullptr;

		CloseAudioDevice();
	}

	void PlaySoundEffect(SoundEffect soundEffect)
	{
		switch (soundEffect)
		{
		case SoundEffect::Shoot:
			PlaySound(shootSound);
			break;
		case SoundEffect::AsteroidExplosion:
			PlaySound(asteroidExplosionSound);
			break;
		case SoundEffect::ShipExplosion:
			PlaySound(shipExplosionSound);
			break;
		case SoundEffect::ButtonClick:
			PlaySound(buttonClickSound);
			break;
		case SoundEffect::GameOver:
			PlaySound(gameOverSound);
			break;
		}
	}

	void PlayMusic(MusicTrack track)
	{
		Music* newMusic = GetMusic(track);

		// Si ya esta sonando esa musica, no hace nada
		if (newMusic == currentMusic)
		{
			return;
		}

		if (currentMusic != nullptr)
		{
			StopMusicStream(*currentMusic);
		}

		currentMusic = newMusic;
		PlayMusicStream(*currentMusic);
	}

	void UpdateMusic()
	{
		if (currentMusic != nullptr)
		{
			UpdateMusicStream(*currentMusic);
		}
	}

	void UpdateThrust(bool isAccelerating)
	{
		if (isAccelerating)
		{
			// Si termino de sonar lo vuelve a arrancar (asi queda en loop)
			if (!IsSoundPlaying(thrustSound))
			{
				PlaySound(thrustSound);
			}
		}
		else if (IsSoundPlaying(thrustSound))
		{
			StopSound(thrustSound);
		}
	}
}
