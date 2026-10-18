# Asteroids

Remake del clásico **Asteroids** (Atari, 1979) hecho en C++ con [raylib](https://www.raylib.com/), para la materia Desarrollo de Videojuegos I de la Tecnicatura Superior en Desarrollo de Videojuegos (Image Campus, 2026).

Pilotá una nave en gravedad cero y destruí todos los asteroides que puedas. Los grandes se parten en medianos, los medianos en chicos, y los chicos desaparecen. Tenés 3 vidas.

## Controles

Todo el juego se maneja con el mouse.

| Acción | Control |
|---|---|
| Apuntar | Mover el mouse (la nave mira hacia la mira) |
| Acelerar | Clic derecho (mantener) |
| Disparar | Clic izquierdo (o barra espaciadora) |
| Pausa | Botón PAUSA (o P / ESC) |
| Menús | Clic en los botones |

## Características

- Nave con inercia: para frenar hay que acelerar en sentido contrario.
- La nave, las balas y los asteroides aparecen del lado contrario al salir de la pantalla.
- Asteroides de 3 tamaños que se dividen al ser destruidos.
- Colisiones círculo-círculo propias, sin raíz cuadrada.
- Power-up de bomba que aparece cada tanto y destruye todo lo que hay en pantalla.
- OVNI enemigo que cruza la pantalla en zigzag y le dispara a la nave.
- 3 vidas, con invulnerabilidad temporal al reaparecer.
- Puntaje: 1 punto por asteroide y 5 por OVNI destruido.
- Movimiento independiente del frame rate.
- Sprites, música y efectos de sonido originales.

## Descargar

El juego compilado de cada versión está en la sección [Releases](https://github.com/SoloDimeDeluxe/Asteroids/releases): bajá el `.zip`, descomprimilo y ejecutá `Asteroids.exe`.

## Compilar

Requisitos: Visual Studio 2026 (toolset v145), plataforma x64. raylib 5.5 ya está incluida en `lib/raylib` (linkeo estático).

1. Abrir `Asteroids.slnx`.
2. Elegir la configuración `Debug` o `Release` (x64).
3. Compilar. Al terminar se copia la carpeta `res/` junto al ejecutable, en `bin/x64/<configuración>/`, así el `.exe` funciona también fuera de Visual Studio.

## Estructura

```
Asteroids/
├── src/        código fuente (main.cpp, game, screens, entities, utils)
├── lib/        raylib (include y lib)
├── res/        assets que carga el juego (texturas, sonidos, música)
├── res_raw/    fuentes editables de los assets (scripts que los generan)
├── bin/        ejecutables (se genera al compilar)
└── obj/        archivos intermedios (se genera al compilar)
```

## Autor

**Valentin Reyes Cuadra**

- GitHub: [SoloDimeDeluxe](https://github.com/SoloDimeDeluxe)
- itch.io: [SoloDim3Deluxe](https://solodim3deluxe.itch.io)

## Herramientas

- C++ con [raylib](https://www.raylib.com/) 5.5
- Visual Studio 2026
- Git y GitHub
- Claude, de Anthropic ([claude.ai](https://claude.ai)): generación y revisión de código, y creación de los sprites, la música y los efectos de sonido (generados por código, ver `res_raw/`).
