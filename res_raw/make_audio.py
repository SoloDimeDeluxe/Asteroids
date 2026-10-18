"""Generates the original sound effects and chiptune music for the Asteroids TP."""
import subprocess
import sys
from pathlib import Path

import numpy as np
from scipy.io import wavfile
from scipy.signal import butter, lfilter

OUT = Path(sys.argv[1])
SFX_DIR = OUT / "sounds"
MUSIC_DIR = OUT / "music"
SFX_DIR.mkdir(parents=True, exist_ok=True)
MUSIC_DIR.mkdir(parents=True, exist_ok=True)

RATE = 44100
rng = np.random.default_rng(7)


# ------------------------------------------------------------------ helpers
def t_axis(seconds):
    return np.arange(int(RATE * seconds)) / RATE


def envelope(n, attack=0.005, release=0.05, sustain_level=1.0):
    env = np.ones(n) * sustain_level
    a = max(1, int(attack * RATE))
    r = max(1, int(release * RATE))
    env[:a] = np.linspace(0, sustain_level, a)
    env[-r:] = np.linspace(sustain_level, 0, r) if r < n else env[-r:]
    return env


def square(freq, t, duty=0.5):
    phase = np.cumsum(freq / RATE) if np.ndim(freq) else freq * t
    return np.where((phase % 1.0) < duty, 1.0, -1.0)


def triangle(freq, t):
    phase = np.cumsum(freq / RATE) if np.ndim(freq) else freq * t
    return 2.0 * np.abs(2.0 * (phase % 1.0) - 1.0) - 1.0


def lowpass(signal, cutoff):
    b, a = butter(2, cutoff / (RATE / 2), btype="low")
    return lfilter(b, a, signal)


def bandpass(signal, low, high):
    b, a = butter(2, [low / (RATE / 2), high / (RATE / 2)], btype="band")
    return lfilter(b, a, signal)


def normalize(signal, peak=0.85):
    m = np.max(np.abs(signal))
    return signal / m * peak if m > 0 else signal


def save_wav(path, signal):
    wavfile.write(path, RATE, (normalize(signal) * 32767).astype(np.int16))
    print("saved", path.name, f"{len(signal) / RATE:.2f}s")


def note_freq(name):
    names = {"C": -9, "C#": -8, "D": -7, "D#": -6, "E": -5, "F": -4, "F#": -3,
             "G": -2, "G#": -1, "A": 0, "A#": 1, "B": 2}
    pitch, octave = name[:-1], int(name[-1])
    return 440.0 * 2 ** ((names[pitch] + (octave - 4) * 12) / 12)


# ------------------------------------------------------------------ sound effects
def sfx_shoot():
    t = t_axis(0.16)
    freq = np.linspace(1400, 300, len(t)) ** 1.0
    s = square(freq, t, duty=0.25) * 0.6 + triangle(freq * 0.5, t) * 0.4
    s *= np.exp(-t * 22)
    save_wav(SFX_DIR / "shoot.wav", lowpass(s, 6000))


def explosion(seconds, low_cut, decay, rumble_freq):
    t = t_axis(seconds)
    noise = rng.uniform(-1, 1, len(t))
    # cutoff that falls over time: bright crack then dark rumble
    s = np.zeros_like(noise)
    chunks = 24
    size = len(t) // chunks + 1
    for i in range(chunks):
        part = noise[i * size:(i + 1) * size]
        cutoff = max(300, low_cut * (1 - i / chunks) ** 2 + 250)
        s[i * size:(i + 1) * size] = lowpass(part, cutoff)
    rumble = np.sin(2 * np.pi * np.linspace(rumble_freq, rumble_freq * 0.5, len(t)) * t)
    s = s * 0.8 + rumble * 0.35
    s *= np.exp(-t * decay) * envelope(len(t), attack=0.002, release=0.05)
    return s


def sfx_asteroid_explosion():
    save_wav(SFX_DIR / "asteroid_explosion.wav", explosion(0.45, 5000, 9, 90))


def sfx_ship_explosion():
    t = t_axis(1.2)
    boom = explosion(1.2, 7000, 3.2, 60)
    sweep = square(np.linspace(600, 60, len(t)), t, duty=0.5) * np.exp(-t * 3) * 0.25
    save_wav(SFX_DIR / "ship_explosion.wav", boom + lowpass(sweep, 3000))


def sfx_button_click():
    t = t_axis(0.07)
    s = square(np.full(len(t), 880.0), t, duty=0.5) * 0.5 + square(np.full(len(t), 1320.0), t, 0.5) * 0.3
    s *= np.exp(-t * 60)
    save_wav(SFX_DIR / "button_click.wav", lowpass(s, 5000))


def sfx_game_over():
    notes = ["E4", "D4", "C4", "A3"]
    parts = []
    for i, n in enumerate(notes):
        dur = 0.22 if i < 3 else 0.7
        t = t_axis(dur)
        f = np.full(len(t), note_freq(n))
        if i == 3:
            f = f * (1 + 0.015 * np.sin(2 * np.pi * 6 * t))  # vibrato on the last note
        s = square(f, t, duty=0.5) * 0.5 + triangle(f * 0.5, t) * 0.5
        s *= envelope(len(t), attack=0.005, release=0.08 if i < 3 else 0.4)
        parts.append(s)
    save_wav(SFX_DIR / "game_over.wav", lowpass(np.concatenate(parts), 4000))


def sfx_thrust():
    # 1 second of filtered noise that loops smoothly (fades at both ends)
    t = t_axis(1.0)
    noise = rng.uniform(-1, 1, len(t))
    s = bandpass(noise, 80, 900) * (0.85 + 0.15 * np.sin(2 * np.pi * 8 * t))
    fade = int(0.03 * RATE)
    s[:fade] *= np.linspace(0, 1, fade)
    s[-fade:] *= np.linspace(1, 0, fade)
    save_wav(SFX_DIR / "thrust.wav", s)


def sfx_bomb():
    # rising "pickup" chime followed by a big deep explosion
    t = t_axis(0.18)
    chime = square(np.linspace(500, 1600, len(t)), t, duty=0.5) * envelope(len(t), 0.003, 0.05) * 0.4
    boom = explosion(1.6, 9000, 2.2, 45)
    save_wav(SFX_DIR / "bomb.wav", np.concatenate([lowpass(chime, 5000), boom]))


# ------------------------------------------------------------------ music
def render_song(bpm, bars, chords, lead_pattern, bass_style, drums, lead_duty, swing=0.0):
    beat = 60.0 / bpm
    total = int(RATE * beat * 4 * bars)
    mix = np.zeros(total)
    step = beat / 4  # 16th notes

    def place(signal, start_time):
        i = int(start_time * RATE)
        end = min(total, i + len(signal))
        mix[i:end] += signal[:end - i]

    for bar in range(bars):
        root, tones = chords[bar % len(chords)]
        bar_time = bar * beat * 4

        # bass
        for s16 in range(16):
            if bass_style == "pulse" and s16 % 2 == 0:
                n = root if s16 % 8 != 6 else tones[1]
                dur = step * 1.8
            elif bass_style == "slow" and s16 % 8 == 0:
                n = root
                dur = beat * 1.9
            else:
                continue
            t = t_axis(dur)
            f = np.full(len(t), note_freq(n))
            sig = triangle(f, t) * envelope(len(t), 0.004, 0.05) * 0.55
            place(sig, bar_time + s16 * step)

        # lead / arpeggio
        for s16, idx in lead_pattern:
            if idx is None:
                continue
            n = tones[idx % len(tones)]
            dur = step * 0.9
            t = t_axis(dur)
            f = np.full(len(t), note_freq(n))
            sig = square(f, t, duty=lead_duty) * envelope(len(t), 0.003, 0.04) * 0.18
            offset = swing * step if s16 % 2 == 1 else 0.0
            place(sig, bar_time + s16 * step + offset)

        # drums
        for s16 in range(16):
            kind = drums[s16]
            if kind == "k":
                t = t_axis(0.18)
                f = 150 * np.exp(-t * 25) + 45
                sig = np.sin(2 * np.pi * np.cumsum(f) / RATE) * np.exp(-t * 18) * 0.7
            elif kind == "s":
                t = t_axis(0.16)
                sig = bandpass(rng.uniform(-1, 1, len(t)), 1200, 6000) * np.exp(-t * 22) * 0.35
            elif kind == "h":
                t = t_axis(0.04)
                sig = bandpass(rng.uniform(-1, 1, len(t)), 6000, 14000) * np.exp(-t * 90) * 0.15
            else:
                continue
            place(sig, bar_time + s16 * step)

    mix = lowpass(mix, 9000)
    # tiny fade at loop point to avoid clicks
    fade = int(0.01 * RATE)
    mix[:fade] *= np.linspace(0, 1, fade)
    mix[-fade:] *= np.linspace(1, 0, fade)
    return normalize(mix, 0.8)


def save_ogg(name, signal):
    wav_path = MUSIC_DIR / (name + ".wav")
    ogg_path = MUSIC_DIR / (name + ".ogg")
    wavfile.write(wav_path, RATE, (signal * 32767).astype(np.int16))
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-i", str(wav_path),
                    "-c:a", "libvorbis", "-q:a", "4", str(ogg_path)], check=True)
    wav_path.unlink()
    print("saved", ogg_path.name, f"{len(signal) / RATE:.1f}s", ogg_path.stat().st_size // 1024, "KB")


def music_menu():
    # calm, spacey: Am - F - C - G, slow arpeggios
    chords = [
        ("A2", ["A4", "C5", "E5", "A5"]),
        ("F2", ["F4", "A4", "C5", "F5"]),
        ("C3", ["E4", "G4", "C5", "E5"]),
        ("G2", ["D4", "G4", "B4", "D5"]),
    ]
    lead = [(0, 0), (2, 1), (4, 2), (6, 3), (8, 2), (10, 1), (12, 2), (14, 3)]
    drums = ["k", None, None, None, "h", None, None, None, "s", None, None, None, "h", None, None, "h"]
    song = render_song(bpm=96, bars=16, chords=chords, lead_pattern=lead,
                       bass_style="slow", drums=drums, lead_duty=0.5)
    save_ogg("menu_music", song)


def music_gameplay():
    # driving, tense: Em - C - D - B, fast pulse bass
    chords = [
        ("E2", ["E4", "G4", "B4", "E5"]),
        ("C2", ["E4", "G4", "C5", "E5"]),
        ("D2", ["D4", "F#4", "A4", "D5"]),
        ("B1", ["D#4", "F#4", "B4", "D#5"]),
    ]
    lead = [(0, 0), (1, 1), (2, 2), (3, 3), (4, 2), (5, 1), (6, 0), (7, 1),
            (8, 3), (9, 2), (10, 1), (11, 2), (12, 3), (13, 2), (14, 1), (15, 2)]
    drums = ["k", "h", "h", "h", "s", "h", "k", "h", "k", "h", "k", "h", "s", "h", "h", "h"]
    song = render_song(bpm=140, bars=16, chords=chords, lead_pattern=lead,
                       bass_style="pulse", drums=drums, lead_duty=0.25)
    save_ogg("gameplay_music", song)


sfx_shoot()
sfx_asteroid_explosion()
sfx_ship_explosion()
sfx_button_click()
sfx_game_over()
sfx_thrust()
music_menu()
music_gameplay()
sfx_bomb()
