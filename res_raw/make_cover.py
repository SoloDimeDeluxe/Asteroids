"""Builds the itch.io cover image (630x500) from the game's own sprites.
Usage (from res_raw): python make_cover.py ../res/textures itch_cover.png"""
import math
import random
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

TEXTURES = Path(sys.argv[1])
OUTPUT = Path(sys.argv[2])
WIDTH, HEIGHT = 630, 500
# First font found is used (Linux paths first, then Windows)
TITLE_FONTS = ["/usr/share/fonts/opentype/noto/NotoSansCJK-Black.ttc", "C:/Windows/Fonts/ariblk.ttf"]
SUBTITLE_FONTS = ["/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "C:/Windows/Fonts/arial.ttf"]

rng = random.Random(42)


def get_font(candidates, size):
    for path in candidates:
        if Path(path).exists():
            return ImageFont.truetype(path, size)
    return ImageFont.load_default(size)


def load(name):
    return Image.open(TEXTURES / name).convert("RGBA")


def paste_rotated(canvas, sprite, center, size, angle_degrees):
    img = sprite.resize((int(size[0]), int(size[1])), Image.LANCZOS)
    img = img.rotate(-angle_degrees, resample=Image.BICUBIC, expand=True)
    canvas.alpha_composite(img, (int(center[0] - img.width / 2), int(center[1] - img.height / 2)))


# ---------------------------------------------------------------- background
canvas = Image.new("RGBA", (WIDTH, HEIGHT), (6, 8, 20, 255))

# soft nebula glow
nebula = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
nd = ImageDraw.Draw(nebula)
for cx, cy, r, color in [(470, 120, 220, (60, 40, 140, 70)), (120, 400, 200, (20, 80, 140, 60))]:
    nd.ellipse([cx - r, cy - r, cx + r, cy + r], fill=color)
canvas.alpha_composite(nebula.filter(ImageFilter.GaussianBlur(70)))

# stars
stars = ImageDraw.Draw(canvas)
for _ in range(170):
    x, y = rng.uniform(0, WIDTH), rng.uniform(0, HEIGHT)
    brightness = rng.randint(90, 255)
    radius = rng.choice([0.6, 0.8, 1.0, 1.4])
    stars.ellipse([x - radius, y - radius, x + radius, y + radius], fill=(brightness, brightness, 255, 255))

# ---------------------------------------------------------------- game elements
large = load("asteroid_large.png")
medium = load("asteroid_medium.png")
small = load("asteroid_small.png")
ship = load("ship.png")
flame = load("ship_flame.png")
bullet = load("bullet.png")

for sprite, center, size, angle in [
    (large, (520, 330), 190, 20),
    (large, (95, 120), 150, 200),
    (medium, (430, 420), 80, 60),
    (medium, (560, 90), 70, 140),
    (small, (60, 300), 40, 30),
    (small, (330, 465), 34, 300),
    (small, (600, 450), 30, 90),
]:
    paste_rotated(canvas, sprite, center, (size, size), angle)

# ship flying up-right, with engine flame behind it
ship_center = (230, 330)
ship_angle = -35  # degrees (screen: negative = towards the top)
ship_size = 120
rad = math.radians(ship_angle)
flame_offset = ship_size * 0.35 + 30
flame_center = (ship_center[0] - math.cos(rad) * flame_offset, ship_center[1] - math.sin(rad) * flame_offset)
paste_rotated(canvas, flame, flame_center, (60, 30), ship_angle)
paste_rotated(canvas, ship, ship_center, (ship_size, ship_size), ship_angle)

# bullets heading to the big asteroid
for distance in (95, 150, 205):
    bx = ship_center[0] + math.cos(rad) * distance
    by = ship_center[1] + math.sin(rad) * distance
    paste_rotated(canvas, bullet, (bx, by), (22, 22), 0)

# ---------------------------------------------------------------- title
title_layer = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
td = ImageDraw.Draw(title_layer)
font = get_font(TITLE_FONTS, 92)
text = "ASTEROIDS"
box = td.textbbox((0, 0), text, font=font)
tx = (WIDTH - (box[2] - box[0])) / 2 - box[0]
ty = 30 - box[1]

# glow behind the letters
glow = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
ImageDraw.Draw(glow).text((tx, ty), text, font=font, fill=(90, 200, 255, 255))
canvas.alpha_composite(glow.filter(ImageFilter.GaussianBlur(12)))

td.text((tx, ty), text, font=font, fill=(245, 248, 255, 255), stroke_width=3, stroke_fill=(20, 60, 120, 255))
canvas.alpha_composite(title_layer)

subtitle_font = get_font(SUBTITLE_FONTS, 18)
subtitle = "Valentin Reyes Cuadra"
sd = ImageDraw.Draw(canvas)
sbox = sd.textbbox((0, 0), subtitle, font=subtitle_font)
sd.text(((WIDTH - (sbox[2] - sbox[0])) / 2, 140), subtitle, font=subtitle_font, fill=(170, 190, 220, 255))

canvas.convert("RGB").save(OUTPUT)
print("saved", OUTPUT, canvas.size)
