#pragma once

namespace ReyesCuadra
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
	void InitAudio();

	void PlaySoundEffect(SoundEffect soundEffect);

	// Cambia la musica de fondo (si ya esta sonando esa, no la reinicia)
	void PlayMusicTrack(MusicTrack track);

	// Hay que llamarla cada frame para que la musica siga sonando
	void UpdateMusicTrack();

	// Sonido del motor: suena en loop mientras isAccelerating sea true
	void UpdateThrustSound(bool isAccelerating);

	// Zumbido del OVNI: suena en loop mientras isUfoOnScreen sea true
	void UpdateUfoHumSound(bool isUfoOnScreen);

	// Libera todo y cierra el dispositivo de audio (antes de CloseWindow)
	void CloseAudio();
}
