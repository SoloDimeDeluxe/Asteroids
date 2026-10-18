# res_raw

Fuentes editables de los assets del juego. Los archivos finales que carga el juego están en `res/`.

Todos los sprites, efectos de sonido y músicas son originales: se generan por código con estos scripts de Python, creados con Claude (Anthropic).

| Script | Genera |
|---|---|
| `make_sprites.py` | `res/textures/` (nave, fuego del motor, asteroides de 3 tamaños, bala y mira) |
| `make_audio.py` | `res/sounds/` (6 efectos `.wav`) y `res/music/` (2 músicas `.ogg`) |
| `make_cover.py` | `res_raw/itch_cover.png` (portada de 630x500 para itch.io, armada con los sprites del juego) |

## Cómo regenerar los assets

Hace falta Python 3 con `pillow`, `numpy` y `scipy`, y `ffmpeg` instalado (para pasar las músicas a `.ogg`).

Desde la carpeta `res_raw`:

```
pip install pillow numpy scipy
python make_sprites.py ../res/textures
python make_audio.py ../res
python make_cover.py ../res/textures itch_cover.png
```

Para cambiar un asset, editá los valores del script (colores, formas, notas, duración) y volvé a correrlo.
