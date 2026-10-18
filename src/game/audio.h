#pragma once

namespace ReyesCuadra::audio
{
	enum class SoundEffect
	{
		Shoot,
		AsteroidExplosion,
		ShipExplosion,
		ButtonClick,
		GameOver,
		Bomb,
		UfoShoot
	};

	enum class MusicTrack
	{
		Menu,
		Gameplay
	};

	// Inicia el dispositivo de audio y carga sonidos y musicas (despues de InitWindow)
	void Init();

	void PlaySoundEffect(SoundEffect soundEffect);

	// Cambia la musica de fondo (si ya esta sonando esa, no la reinicia)
	void PlayMusic(MusicTrack track);

	// Hay que llamarla cada frame para que la musica siga sonando
	void UpdateMusic();

	// Sonido del motor: suena en loop mientras isAccelerating sea true
	void UpdateThrust(bool isAccelerating);

	// Zumbido del OVNI: suena en loop mientras isUfoOnScreen sea true
	void UpdateUfoHum(bool isUfoOnScreen);

	// Libera todo y cierra el dispositivo de audio (antes de CloseWindow)
	void Close();
}
